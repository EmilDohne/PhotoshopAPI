#pragma once

#include "Macros.h"
#include "Layer.h"
#include "LayeredFile/concepts.h"
#include "ImageDataMixins.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include "LayeredFile/LinkedData/LinkedLayerData.h"
#include "LayeredFile/LayeredFile.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/Warp/SmartObjectWarp.h"
#include "Core/Render/Render.h"
#include "Util/Enum.h"

#include <fstream>
#include <string>
#include <cassert>

#include "fmt/core.h"
#include <Eigen/Dense>
#include <OpenImageIO/imageio.h>


PSAPI_NAMESPACE_BEGIN



/// Smart objects are Photoshops' way of non-destructive image data edits while keeping a live link to the original file.
///
/// We expose not only ways to replace this linked image data but also have functionality to recreate and store the warps 
/// applied to these objects (with more features coming in the future). 
/// We currently support recreating all the warps found in the Edit->Transform tab. We do not yet support the `Edit->Puppet Warp`
/// and `Edit->Perspective Warp` which are stored as Smart Filters.
/// 
/// Smart objects store their original image data on the `LayeredFile<T>` while storing a decoded preview the size of the layer on
/// the layer itself. We provide multiple methods to get both the scaled and warped image data as well as the full size image 
/// data.
/// 
/// <b>Image Data:</b>
/// 
/// Due to how SmartObjects work, image data is read-only and all write methods will fail a static_assert if you try to access them.
/// In order to modify the underlying image data you should use the `replace()` method which will actually replace the underlying 
/// file the smart object is linked to.
/// 
/// Getting the image data can be done via the `get_image_data()`, `get_channel()` and `original_image_data()` functions. These will retrieve
/// the transformed and warped image data. If you modify these you can requery these functions and get up to date image data.
/// 
/// <b>Transformations:</b>
/// 
/// Unlike normal layers, SmartObjects have slightly different transformation rules. As they link back to a file in memory or on disk
/// the transformations are stored 'live' and can be modified without negatively impacting the quality of the image. We expose a variety
/// of transformation options to allow you to express this freedom. 
/// 
/// Since we have both the original image data, and the rescaled image data to worry about there is two different widths and heights available:
/// 
/// - `original_width()` / `original_height()`
///		These represent the resolution of the original file image data, irrespective of what transforms are applied to it.
///		If you are e.g. loading a 4000x2000 jpeg these will return 4000 and 2000 respectively. These values may not be written to
/// 
/// - `width()` / `height()`
///		These represent the final dimensions of the SmartObject with the warp and any transformations applied to it. 
/// 
/// For actually transforming the layer we expose the following methods:
/// 
/// - `move()`
/// - `rotate()`
/// - `scale()`
/// - `transform()`
/// 
/// These are all individually documented and abstract away the underlying implementation of these operations. You likely will not have to dive deeper than these.
/// 
/// <b>Warp:</b>
/// 
/// Smart objects can also store warps which we implement using the `SmartObject::Warp` structure. These warps are stored as bezier surfaces with transformations applied on top of them.
/// The transformations should be disregarded by the user as we provide easier functions on the SmartObjectLayer directly (see above). The warp itself is stored as a bezier
/// surface. You may transfer these warps from one layer to another, modify them (although this requires knowledge of how bezier surfaces work), or clear them entirely.
/// 
/// For the latter we provide the reset_transform()` and `reset_warp()` functions.
/// 
template <typename T>
	requires concepts::bit_depth<T>
struct SmartObjectLayer final: public Layer<T>, public ImageDataMixin<T>
{
	using typename Layer<T>::value_type;
	using typename ImageDataMixin<T>::data_type;
	using typename ImageDataMixin<T>::channel_type;
	using typename ImageDataMixin<T>::image_type;

public:


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::vector<int> channel_indices(bool include_mask) const override
	{
		std::vector<int> indices{};
		for (const auto& [key, _] : ImageDataMixin<T>::m_ImageData)
		{
			indices.push_back(key.index);
		}
		if (Layer<T>::has_mask() && include_mask)
		{
			indices.push_back(Layer<T>::s_mask_index.index);
		}
		return indices;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	size_t num_channels(bool include_mask) const override
	{
		if (Layer<T>::has_mask() && include_mask)
		{
			return ImageDataMixin<T>::m_ImageData.size() + 1;
		}
		return ImageDataMixin<T>::m_ImageData.size();
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_write_compression(Enum::Compression _compcode) override
	{
		for (const auto& [_, channel_ptr] : ImageDataMixin<T>::m_ImageData)
		{
			channel_ptr->m_Compression = _compcode;
		}
		Layer<T>::set_mask_compression(_compcode);
	}


	SmartObjectLayer() = default;

	/// Initialize a SmartObject layer from a filepath.
	///
	/// This will internally load the given file (assuming it exists) into memory, decoding the full resolution
	/// image data as well as generating a resampled image data based on the resolution provided in the layers'
	/// parameters (this may be zero in which case we will ignore the width and height and keep the original size). 
	/// Requires the `LayeredFile` to be passed so we can keep track of this global state of linked layer data.
	/// 
	/// \param file				The LayeredFile this SmartObject is to be associated with
	/// 
	/// \param parameters		The Layers' parameters
	/// 
	/// \param filepath			The path of the file to load, this must be a file format Photoshop knows about and can decode.
	///							If `link_externally` is set to true it is highly recommended to keep this file local to the output directory.
	///							I.e. if the file gets written to `C:/PhotoshopFiles/file.psb` The file should be in `C:/PhotoshopFiles/`
	///							(same applies to linux). To learn more about how photoshop resolves these linkes head to this page:
	///							https://helpx.adobe.com/photoshop/using/create-smart-objects.html#linking_logic
	/// 
	/// \param linkage			Whether to link the file externally (without saving it in the document). While this does reduce 
	///							file size, due to linking limitations it is usually recommended to leave this at its default `false`.
	///							If the given file already exists on the `LayeredFile<T>` e.g. when you link 2 layers with the same filepath
	///							the settings for the first layer are used instead of overriding the behaviour
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	SmartObjectLayer(LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, LinkedLayerType linkage = LinkedLayerType::data)
	{
		m_LinkedLayers = file.linked_layers();
		this->construct(parameters, filepath, linkage);
	}

	/// Initialize a SmartObject layer from a filepath.
	///
	/// This will internally load the given file (assuming it exists) into memory, decoding the full resolution
	/// image data as well as generating a resampled image data based on the resolution provided in the layers'
	/// parameters (this may be zero in which case we will ignore the width and height and keep the original size). 
	/// Requires the `LayeredFile` to be passed so we can keep track of this global state of linked layer data.
	/// 
	/// \param file				The LayeredFile this SmartObject is to be associated with
	/// 
	/// \param parameters		The Layers' parameters
	/// 
	/// \param filepath			The path of the file to load, this must be a file format Photoshop knows about and can decode.
	///							If `link_externally` is set to true it is highly recommended to keep this file local to the output directory.
	///							I.e. if the file gets written to `C:/PhotoshopFiles/file.psb` The file should be in `C:/PhotoshopFiles/`
	///							(same applies to linux). To learn more about how photoshop resolves these linkes head to this page:
	///							https://helpx.adobe.com/photoshop/using/create-smart-objects.html#linking_logic
	/// 
	/// \param warp				The warp to apply to the image data, this may be default generated warp which can be modified later on by
	///							retrieving it using `warp()`. After then modifying it, the updated warp will be lazily evaluated on write or 
	///							access. So you may modify it as many times as you want but only retrieving it will call the evaluation.
	///							If you wish to skip this you can pass `SmartObject::Warp::generate_default(width, height)` or use the alternative ctor
	///							without `warp` argument
	/// 
	/// \param linkage			Whether to link the file externally (without saving it in the document). While this does reduce 
	///							file size, due to linking limitations it is usually recommended to leave this at its default `false`.
	///							If the given file already exists on the `LayeredFile<T>` e.g. when you link 2 layers with the same filepath
	///							the settings for the first layer are used instead of overriding the behaviour
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	SmartObjectLayer(LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, const SmartObject::Warp& warp, LinkedLayerType linkage = LinkedLayerType::data)
	{
		m_LinkedLayers = file.linked_layers();
		this->construct(parameters, filepath, linkage, warp);
	}

	/// Generate a SmartObjectLayer from a Photoshop File object. This is for internal uses and not intended to be used by users directly. Please use the other
	/// constructors instead.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	SmartObjectLayer(LayeredFile<T>& file, const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo)
		: Layer<T>(layerRecord, channelImageData, header)
	{
		// Local and global additional layer info in this case refer to the one stored on the individual layer and the one 
		// stored on the LayerAndMaskInfo section respectively
		m_LinkedLayers = file.linked_layers();
		if (layerRecord.m_AdditionalLayerInfo)
		{
			const auto& localAdditionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			decode(localAdditionalLayerInfo, globalAdditionalLayerInfo, Layer<T>::m_LayerName);
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Expected smart object layer to contain an AdditionalLayerInfo section");
		}
	}


	/// Retrieve the warp object that is stored on this layer. 
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	SmartObject::Warp warp() const noexcept { return m_SmartObjectWarp; }

	/// Set the warp object held by this layer, this function may be used to replace the warp with e.g. the
	/// warp from another layer
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void warp(SmartObject::Warp _warp) 
	{ 
		m_SmartObjectWarp = std::move(_warp);
		evaluate_transforms();
	}


	/// Replace the smart object with the given path keeping transformations as well 
	/// as warp in place.
	/// 
	/// \param path				The path to replace the image data with
	/// \param link_externally	Whether to link the file externally or store the raw file bytes on the 
	///							photoshop document itself. Keeping this at its default `false` is recommended
	///							for sharing these files.
	/// 
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void replace(std::filesystem::path path, bool link_externally = false)
	{
		if (!m_LinkedLayers)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to replace the smart objects' image without access to the original LinkedLayers." \
				" If you believe this to be an error you can try recreating the layer making sure to pass a valid LayeredFile object.");
		}
		auto previous_bbox = m_SmartObjectWarp._warp_bounds();

		// Insert the new path, if it already exists insert() will return a reference to the previous layer.
		LinkedLayerType type = link_externally ? LinkedLayerType::external : LinkedLayerType::data;
		auto linked_layer = m_LinkedLayers->insert(path, type);

		m_Filename = linked_layer->path().filename().string();
		m_Hash = linked_layer->hash();
		_m_LayerHash = generate_uuid();

		// Update the warp original bounds so it knows the input image data scaled.
		Geometry::BoundingBox<double> bbox(
			Geometry::Point2D<double>(0.0f, 0.0f),
			Geometry::Point2D<double>(
				static_cast<double>(linked_layer->width()), 
				static_cast<double>(linked_layer->height())));

		// Finally we also need to rescale the warp points, this is because they are in the original images' coordinate space.
		auto pts = m_SmartObjectWarp.points();
		auto scalar = Geometry::Point2D<double>(bbox.width() / previous_bbox.width(), bbox.height() / previous_bbox.height());
		auto pivot = previous_bbox.minimum;

		Geometry::Operations::scale(pts, scalar, pivot);

		m_SmartObjectWarp.points(pts);
		invalidate_cache();
		invalidate_mesh_cache();
	}


	/// Check whether the original image file stored by this smart object is linked externally
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool linked_externally() const
	{
		if (!m_LinkedLayers)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to get original file linkage without the smart object knowing about the LinkedLayers");
		}
		auto linkedlayer_ptr = m_LinkedLayers->at(m_Hash);
		return linkedlayer_ptr->type() == LinkedLayerType::external;
	}

	/// Set the type of linkage for the original image data.
	///
	/// This can be data or external where data is storing the raw file bytes on the 
	/// file itself while external will reference a file from disk. As this property
	/// only has an effect on-write this can be modified as many times as wanted.
	/// 
	/// As this affects not the layer directly but the shared linked layer if any other
	/// layers refer to this same file we modify that too.
	/// 
	/// \param linkage The type of linkage we want to set
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_linkage(LinkedLayerType linkage)
	{
		if (!m_LinkedLayers)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to set original file linkage without the smart object knowing about the LinkedLayers");
		}
		auto linkedlayer_ptr = m_LinkedLayers->at(m_Hash);
		linkedlayer_ptr->type(linkage);
	}

	/// Retrieve the hashed value associated with the layer, this is what is used to identify the
	/// linked layer associated with this smart object (where the original image data is stored)
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	const std::string& hash() const noexcept { return m_Hash; }

	/// Retrieve the filename associated with this smart object.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	const std::string& filename() const noexcept { return m_Filename; }

	/// Retrieve the filepath associated with this smart object. Depending on how the 
	/// Smart object is linked (`external` or `data`) this may not be written to disk.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::filesystem::path filepath() const 
	{
		if (!m_LinkedLayers)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query filepath without the smart object knowing about the linked layers");
		}
		const auto& layer = m_LinkedLayers->at(m_Hash);
		return layer->path();
	}

	/// Extract all the channels of the original image data.
	/// 
	/// Unlike the accessors `get_image_data()` and `get_channel()` this function gets the full resolution
	/// image data that is stored on the smart object, i.e. the original image data. This may be smaller
	/// or larger than the layers `width` or `height`. To get the actual resolution you can query: `original_width()` and `original_height()`
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	data_type get_original_image_data()
	{
		if (!m_LinkedLayers)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query original file image data without the smart object knowing about the linked layers");
		}
		const auto& layer = m_LinkedLayers->at(m_Hash);
		auto original = layer->get_image_data();
		data_type out{};
		for (const auto [key, channel] : original)
		{
			out[key.index] = std::move(channel);
		}
		return std::move(out);
	}

	/// Retrieve the original image datas' width.
	///
	/// This does not have the same limitation as Photoshop layers of being limited
	/// to 30,000 or 300,000 pixels depending on the file type
	/// 
	/// \throws std::runtime_error if the hash defined by `hash()` is not valid for the document
	/// 
	/// \returns The width of the original image data
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	size_t original_width() const
	{
		if (!m_LinkedLayers)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query original file width without the smart object knowing about the linked layers");
		}
		const auto& layer = m_LinkedLayers->at(m_Hash);
		return layer->width();
	}

	/// Retrieve the original image datas' height.
	///
	/// This does not have the same limitation as Photoshop layers of being limited
	/// to 30,000 or 300,000 pixels depending on the file type
	/// 
	/// \throws std::runtime_error if the hash defined by `hash()` is not valid for the document
	/// 
	/// \return The height of the original image data
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	size_t original_height() const
	{
		if (!m_LinkedLayers)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query original file height without the smart object knowing about the linked layers");
		}
		const auto& layer = m_LinkedLayers->at(m_Hash);
		return layer->height();
	}


	/// Move the SmartObjectLayer (including any warps) by the given offset
	///
	/// \param offset the offset to move the layer by
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void move(Geometry::Point2D<double> offset)
	{
		{
			auto affine_transform = m_SmartObjectWarp.affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(affine_transform.begin(), affine_transform.end());

			Geometry::Operations::move(pts, offset);

			m_SmartObjectWarp.affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}
		{
			auto non_affine_transform = m_SmartObjectWarp.non_affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(non_affine_transform.begin(), non_affine_transform.end());

			Geometry::Operations::move(pts, offset);

			m_SmartObjectWarp.non_affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}
		evaluate_transforms();
	}

	/// Rotate the SmartObjectLayer (including any warps) by the given offset in degrees
	/// around the provided center point, this point does not have to lie on the pixels of the image
	/// 
	/// \param offset The rotation value in degrees
	/// \param center The center point to rotate around
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void rotate(double offset, Geometry::Point2D<double> center)
	{
		{
			auto affine_transform = m_SmartObjectWarp.affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(affine_transform.begin(), affine_transform.end());

			Geometry::Operations::rotate(pts, offset, center);

			m_SmartObjectWarp.affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}
		{
			auto non_affine_transform = m_SmartObjectWarp.non_affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(non_affine_transform.begin(), non_affine_transform.end());

			Geometry::Operations::rotate(pts, offset, center);

			m_SmartObjectWarp.non_affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}
		evaluate_transforms();
	}

	/// Rotate the SmartObjectLayer (including any warps) by the given offset in degrees
	/// around the center of the layer
	/// 
	/// \param offset The rotation value in degrees
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void rotate(double offset)
	{
		auto affine_transform = m_SmartObjectWarp.affine_transform();
		auto pts = std::vector<Geometry::Point2D<double>>(affine_transform.begin(), affine_transform.end());

		rotate(offset, Geometry::BoundingBox<double>::compute(pts).center());
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions.
	/// 
	/// \param factor The scalar factor
	/// \param center The point to scale about
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void scale(Geometry::Point2D<double> factor, Geometry::Point2D<double> center)
	{
		{
			auto affine_transform = m_SmartObjectWarp.affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(affine_transform.begin(), affine_transform.end());
			Geometry::Operations::scale(pts, factor, center);

			m_SmartObjectWarp.affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}
		{
			auto non_affine_transform = m_SmartObjectWarp.non_affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(non_affine_transform.begin(), non_affine_transform.end());
			Geometry::Operations::scale(pts, factor, center);

			m_SmartObjectWarp.non_affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}
		evaluate_transforms();
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions.
	/// 
	/// \param factor The scalar factor
	/// \param center The point to scale about
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void scale(double factor, Geometry::Point2D<double> center)
	{
		scale(Geometry::Point2D<double>(factor, factor), center);
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions around the layers center.
	/// 
	/// \param factor The scalar factor
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void scale(Geometry::Point2D<double> factor)
	{
		auto affine_transform = m_SmartObjectWarp.affine_transform();
		auto pts = std::vector<Geometry::Point2D<double>>(affine_transform.begin(), affine_transform.end());
		scale(factor, Geometry::BoundingBox<double>::compute(pts).center());
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions around the layers center.
	/// 
	/// \param factor The scalar factor
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void scale(double factor)
	{
		scale(Geometry::Point2D<double>(factor, factor));
	}

	/// Apply a transformation (affine or non affine) to the smart object. This can be used in order
	/// to e.g. skew or perspective transform the image. Automatically splits the matrix into it's 
	/// affine and non-affine transformations and applies these separately.
	/// 
	/// \param matrix The transformation matrix, will internally be split into its affine and non-affine components.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void transform(const Eigen::Matrix3d& matrix)
	{
		Eigen::Matrix3d affine = matrix;
		Eigen::Matrix3d non_affine = matrix;

		// Remove the perspective component of the affine transform matrix.
		affine(2, 0) = 0.0f;
		affine(2, 1) = 0.0f;
		// Normalize affine by m33
		if (affine(2, 2) != 1.0) 
		{
			affine /= affine(2, 2);
		}

		// Now apply the transformation to both the affine and non affine component.
		{
			auto affine_transform = m_SmartObjectWarp.affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(affine_transform.begin(), affine_transform.end());
			Geometry::Operations::transform(pts, affine);

			m_SmartObjectWarp.affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}

		{
			auto non_affine_transform = m_SmartObjectWarp.non_affine_transform();
			auto pts = std::vector<Geometry::Point2D<double>>(non_affine_transform.begin(), non_affine_transform.end());
			Geometry::Operations::transform(pts, non_affine);

			m_SmartObjectWarp.non_affine_transform(pts[0], pts[1], pts[2], pts[3]);
		}

		evaluate_transforms();
	}

	uint32_t width() const noexcept override
	{
		return Layer<T>::m_Width;
	}
	/// Set the layers' width, analogous to calling `scale()` while only scaling around the x axis.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void width(uint32_t layer_width) override
	{
		// Explicitly dont use the width() function on the SmartObjectLayer to avoid
		// floating to int conversion issues
		auto current_width = this->evaluate_mesh_or_get_cached().bbox().width();

		double scalar_x = static_cast<double>(layer_width) / current_width;
		auto center = Geometry::Point2D<double>(Layer<T>::center_x(), Layer<T>::center_y());

		// This will call evaluate_transforms.
		scale(Geometry::Point2D<double>(scalar_x, 1.0f), center);
	}

	uint32_t height() const noexcept override
	{
		return Layer<T>::m_Height;
	}
	/// Set the layers' height, analogous to calling `scale()` while only scaling around the y axis.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void height(uint32_t layer_height) override
	{
		// Explicitly dont use the height() function on the SmartObjectLayer to avoid
		// floating to int conversion issues
		auto current_height = this->evaluate_mesh_or_get_cached().bbox().height();

		double scalar_y = static_cast<double>(layer_height) / current_height;
		auto center = Geometry::Point2D<double>(Layer<T>::center_x(), Layer<T>::center_y());

		// This will call evaluate_transforms.
		scale(Geometry::Point2D<double>(1.0f, scalar_y), center);
	}
	

	float center_x() const noexcept override
	{
		return Layer<T>::m_CenterX;
	}
	/// Set the x center coordinate, analogous to calling `move()` while only moving on the x axis
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void center_x(float x_coord) noexcept override 
	{
		auto offset = x_coord - Layer<T>::m_CenterX;
		// this will call evaluate_transforms
		move(Geometry::Point2D<double>(offset, 0.0f));

	}

	float center_y() const noexcept override
	{
		return Layer<T>::m_CenterY;
	}
	/// Set the y center coordinate, analogous to calling `move()` while only moving on the y axis
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void center_y(float y_coord) noexcept override
	{
		auto offset = y_coord - Layer<T>::m_CenterY;
		// this will call evaluate_transforms
		move(Geometry::Point2D<double>(0.0f, offset));
	}


	/// Reset all the transformations (not the warp) applied to the layer to map it back to the original square 
	/// from [0 - `original_width()`] and [0 - `original_height()`]. This does not reset the warp itself so if you had a warp applied it will stay.
	/// 
	/// If you instead wish to clear the warp you can use `reset_warp()`.
	/// 
	/// These two may be used in combination and sequence, so it is perfectly valid to call `reset_transform`
	/// and `reset_warp` in any order
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void reset_transform()
	{
		auto current_transform = m_SmartObjectWarp.non_affine_transform();
		auto goal_transform = Geometry::create_quad<double>(
			static_cast<double>(original_width()), 
			static_cast<double>(original_height())
		);

		auto homography = Geometry::Operations::create_homography_matrix<double>(current_transform, goal_transform);
		transform(homography);
	}

	/// Reset the warp (not the transformations) applied to the Smart Object.
	/// 
	/// If you instead wish to clear the transformations you can use the `reset_transform()` function.
	/// 
	/// These two may be used in combination and sequence, so it is perfectly valid to call `reset_transform`
	/// and `reset_warp` in any order
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void reset_warp()
	{
		auto affine = m_SmartObjectWarp.affine_transform();
		auto non_affine = m_SmartObjectWarp.non_affine_transform();

		m_SmartObjectWarp = SmartObject::Warp::generate_default(original_width(), original_height());

		m_SmartObjectWarp.affine_transform(affine);
		m_SmartObjectWarp.non_affine_transform(non_affine);

		evaluate_transforms();
	}


	// Override the to_photoshop method to specialize for smart objects.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::tuple<LayerRecord, ChannelImageData> to_photoshop() override
	{
		// Evaluate transforms and image data to be sure these are up to date
		evaluate_transforms();
		evaluate_image_data();

		PascalString lrName = Layer<T>::generate_name();
		ChannelExtents extents = generate_extents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY));
		size_t channelCount = this->num_channels(true);

		uint8_t clipping = 0u;	// No clipping mask for now
		LayerRecords::BitFlags bitFlags(Layer<T>::m_IsLocked, !Layer<T>::m_IsVisible, false);
		std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::internal_generate_mask_data();
		LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generate_blending_ranges();

		// Generate our AdditionalLayerInfoSection. This will contain e.g. the placed layer data.
		auto blockVec = this->generate_tagged_blocks();
		std::optional<AdditionalLayerInfo> taggedBlocks = std::nullopt;
		if (blockVec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { blockVec };
			taggedBlocks.emplace(blockStorage);
		}

		// Initialize the channel information as well as the channel image data, the size held in the channelInfo might change depending on
		// the compression mode chosen on export and must therefore be updated later. This step is done last as generateChannelImageData() invalidates
		// all image data which we might need for operations above
		auto channelData = this->generate_channel_image_data();
		auto& channelInfoVec = std::get<0>(channelData);
		ChannelImageData channelImgData = std::move(std::get<1>(channelData));

		LayerRecord lrRecord = LayerRecord(
			lrName,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			static_cast<int16_t>(channelCount),
			channelInfoVec,
			Layer<T>::m_BlendMode,
			Layer<T>::m_Opacity,
			clipping,
			bitFlags,
			lrMaskData,
			blendingRanges,
			std::move(taggedBlocks)
		);

		return std::make_tuple(std::move(lrRecord), std::move(channelImgData));
	}

protected:

	virtual std::vector<std::shared_ptr<TaggedBlock>> generate_tagged_blocks() override
	{
		auto blocks = Layer<T>::generate_tagged_blocks();
		if (!m_LinkedLayers)
		{
			throw std::runtime_error(fmt::format("SmartObjectLayer '{}': Unexpected failure while generating tagged blocks: m_LinkedLayers is a nullptr", Layer<T>::m_LayerName));
		}
		auto linked_layer = m_LinkedLayers->at(m_Hash);

		// Before generating the descriptor we need to update the original width and height
		// as we don't keep track of these usually.
		m_OriginalSize[0] = static_cast<float>(linked_layer->width());
		m_OriginalSize[1] = static_cast<float>(linked_layer->height());

		auto descriptor = generate_placed_layer_data();
		auto block_ptr = std::make_shared<PlacedLayerDataTaggedBlock>(std::move(descriptor));
		blocks.push_back(block_ptr);

		return blocks;
	}


	/// Extracts the m_ImageData as well as the layer mask into two vectors holding channel information as well as the image data 
	/// itself. This also takes care of generating our layer mask channel if it is present. Invalidates any data held by the ImageLayer
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> generate_channel_image_data()
	{
		std::vector<LayerRecords::ChannelInformation> channel_info;
		std::vector<std::unique_ptr<ImageChannel>> channel_data;

		// First extract our mask data, the order of our channels does not matter as long as the 
		// order of channelInfo and channelData is the same
		auto mask_data = Layer<T>::internal_extract_mask();
		if (mask_data.has_value())
		{
			channel_info.push_back(std::get<0>(mask_data.value()));
			channel_data.push_back(std::move(std::get<1>(mask_data.value())));
		}

		// Extract all the channels next and push them into our data representation
		for (auto& [id, channel] : ImageDataMixin<T>::m_ImageData)
		{
			channel_info.push_back(LayerRecords::ChannelInformation{ id, channel->m_OrigByteSize });
			channel_data.push_back(std::move(channel));
		}

		// Construct the channel image data from our vector of ptrs, moving gets handled by the constructor
		ChannelImageData channel_image_data(std::move(channel_data));
		return std::make_tuple(channel_info, std::move(channel_image_data));
	}

private:

	std::shared_ptr<LinkedLayers<T>> m_LinkedLayers;

	/// The warp on the object, may be a no-op in the case of no warp.
	SmartObject::Warp m_SmartObjectWarp{};

	/// The hash of the file, this is the same as what is stored on the LinkedLayerData
	/// and identical files are automatically de-duplicated
	std::string m_Hash{};

	/// The filename that the Smart Object was constructed with.
	std::string m_Filename{};

	/// The original width and height of the image data
	std::array<double, 2> m_OriginalSize{};

	/// Resolution in DPI
	double m_Resolution = 72.0f;

	/// Internal values for roundtripping:

	/// Hash of the layer itself, doesn't seem to relate back to the LinkedLayers and instead is just a uuid
	std::string _m_LayerHash{};

	int32_t _m_PageNum = 1;
	int32_t _m_NumPages = 1;

	int32_t _m_Crop = 1;	// Appears to always be 1

	int32_t _m_FrameStepNumerator = 0;
	int32_t _m_FrameStepDenominator = 600;
	int32_t _m_DurationStepNumerator = 0;
	int32_t _m_DurationStepDenominator = 600;
	int32_t _m_FrameCount = 1;

	int32_t _m_AntiAliasing = 16; // Maybe the sample radius for rescaling? I.e. 4x4 = 16 = bicubic?

	int32_t _m_Type = 2;	// Appears to always be 2

	int32_t _m_Comp = -1;
	int32_t _m_CompInfoID = -1;
	int32_t _m_CompInfoOriginalID = -1;
	
private:

	/// Cache keeping track of which channel is up-to-date and which channel is out of date. 
	/// Some or all of these may be out of date at any point, we must therefore ensure during evaluation if
	/// image data can be grabbed directly or if it needs to be evaluated first.
	/// Maps back to m_ImageData.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::unordered_map<Enum::ChannelIDInfo, bool, Enum::ChannelIDInfoHasher> m_Cache;

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool is_cache_valid()
	{
		for (const auto& [key, _] : ImageDataMixin<T>::m_ImageData)
		{
			if (!m_Cache.contains(key))
			{
				return false;
			}
		}
		return true;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool is_cache_valid(Enum::ChannelIDInfo channel)
	{
		if (m_Cache.contains(channel))
		{
			return m_Cache[channel];
		}
		return false;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool is_cache_valid(std::vector<Enum::ChannelIDInfo> channels)
	{
		for (auto _id : channels)
		{
			if (!m_Cache.contains(_id) || !m_Cache[_id])
			{
				return false;
			}
		}
		return true;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void invalidate_cache(std::optional<Enum::ChannelIDInfo> channel = std::nullopt)
	{
		if (channel)
		{
			m_Cache[channel.value()] = false;
		}
		else
		{
			m_Cache = {};
		}
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void store_was_cached()
	{
		for (const auto& [key, _] : ImageDataMixin<T>::m_ImageData)
		{
			m_Cache[key] = true;
		}
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void store_was_cached(Enum::ChannelIDInfo channel)
	{
		m_Cache[channel] = true;
	}


	/// Cache storing the latest mesh data so we don't have to recompute it on the fly
	/// for every transformation.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::QuadMesh<double> m_MeshCache{};
	bool m_MeshCacheValid = false;

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool is_mesh_cache_valid()
	{
		return m_MeshCacheValid;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void invalidate_mesh_cache()
	{
		m_MeshCacheValid = false;
		m_MeshCache = {};
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void store_mesh_was_cached()
	{
		m_MeshCacheValid = true;
	}

	/// Evaluate the mesh from the smartobject warp or retrieve it from the cache (if it is valid).
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	const Geometry::QuadMesh<double>& evaluate_mesh_or_get_cached()
	{
		PSAPI_PROFILE_FUNCTION();
		if (!m_LinkedLayers)
		{
			throw std::runtime_error(fmt::format("SmartObjectLayer '{}': Unexpected failure while evaluating the mesh: m_LinkedLayers is a nullptr", Layer<T>::m_LayerName));
		}
		if (!is_mesh_cache_valid())
		{
			auto linked_layer = m_LinkedLayers->at(m_Hash);
			assert(linked_layer != nullptr);

			// Get the warp mesh at a resolution of 20 pixels per subdiv. Ideally we'd lower this as we improve our algorithms
			auto warp_surface = m_SmartObjectWarp.surface();
			m_MeshCache = warp_surface.mesh(
				linked_layer->width() / 20,
				linked_layer->height() / 20,
				true	// move_to_zero, that way we don't have to deal with bbox stuff
			);
			store_mesh_was_cached();
		}
		return m_MeshCache;
	}

protected:

	/// Evaluates the transformation (updates center coordinates and width/height) meaning grabbing the bbox width and height will give the 
	/// latest warp information
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void evaluate_transforms()
	{
		// Invalidate and re-evaluate all the caches.
		this->invalidate_cache();
		this->invalidate_mesh_cache();
		auto& mesh = this->evaluate_mesh_or_get_cached();

		Layer<T>::m_CenterX = static_cast<float>(mesh.bbox().center().x);
		Layer<T>::m_CenterY = static_cast<float>(mesh.bbox().center().y);

		Layer<T>::m_Width = static_cast<uint32_t>(std::round(mesh.bbox().width()));
		Layer<T>::m_Height = static_cast<uint32_t>(std::round(mesh.bbox().height()));
	}

	/// Lazily evaluates (and updates if necessary) the ImageData of the SmartObjectLayer. Checks whether
	/// the cached warp and transform values match what is cached on the object, if that is not the case
	/// we recompute the image data and assign the warp to m_Warp.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	data_type evaluate_image_data() override
	{
		PSAPI_PROFILE_FUNCTION();
		if (!m_LinkedLayers)
		{
			throw std::runtime_error(fmt::format("SmartObjectLayer '{}': Unexpected failure while evaluating the image data: m_LinkedLayers is a nullptr", Layer<T>::m_LayerName));
		}

		constexpr auto s_alpha_idinfo = Enum::ChannelIDInfo{ Enum::ChannelID::Alpha, static_cast<int16_t>(-1) };
		const std::shared_ptr<LinkedLayerData<T>> linked_layer = m_LinkedLayers->at(m_Hash);

		// Construct all the channel indices including alpha and mask
		auto all_channel_indices = linked_layer->channel_indices();
		if (std::find(all_channel_indices.begin(), all_channel_indices.end(), s_alpha_idinfo) == all_channel_indices.end())
		{
			// We always insert an alpha channel
			all_channel_indices.push_back(s_alpha_idinfo);
		}
		if (Layer<T>::has_mask())
		{
			all_channel_indices.push_back(Layer<T>::s_mask_index);
		}

		// evaluate all the channels and return them. This could be a bit 
		// more efficient by preallocating the channels in parallel but since 
		// evaluate_channel calls the apply function which is already parallelized
		// we will keep it like this
		data_type out{};
		for (auto& item : all_channel_indices)
		{
			out[item.index] = this->evaluate_channel(item);
		}

		return out;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::vector<T> evaluate_channel(std::variant<int, Enum::ChannelID, Enum::ChannelIDInfo> _id) override
	{
		PSAPI_PROFILE_FUNCTION();
		auto idinfo = ImageDataMixin<T>::idinfo_from_variant(_id, Layer<T>::m_ColorMode);
		constexpr auto s_alpha_idinfo = Enum::ChannelIDInfo{ Enum::ChannelID::Alpha, static_cast<int16_t>(-1) };

		if (!m_LinkedLayers)
		{
			throw std::runtime_error(fmt::format("SmartObjectLayer '{}': Unexpected failure while evaluating the image data: m_LinkedLayers is a nullptr", Layer<T>::m_LayerName));
		}

		// Short-circuit mask channels
		if (idinfo == Layer<T>::s_mask_index && Layer<T>::has_mask())
		{
			return Layer<T>::get_mask();
		}

		// If we have a cached item we return it.
		if (this->is_cache_valid(idinfo))
		{
			if (!ImageDataMixin<T>::m_ImageData.contains(idinfo))
			{
				throw std::invalid_argument(fmt::format(
					"SmartObjectLayer '{}': Invalid channel '{}' accessed while calling evaluate_channel(). This does not exist on the smart object", 
					Layer<T>::m_LayerName, 
					Enum::channelIDToString(idinfo.id))
				);
			}
			return ImageDataMixin<T>::m_ImageData.at(idinfo)->template getData<T>();
		}
		else
		{
			// Evaluate the warp and cache the result.
			auto linked_layer = m_LinkedLayers->at(m_Hash);

			// The alpha channel may not necessarily exist on the image data, however we always
			// want to create it if that is the case. Other channels we do not generate though.
			std::vector<T> image_data;
			if (linked_layer->has_channel(idinfo))
			{
				image_data = linked_layer->get_channel(idinfo);
			}
			else if (idinfo == s_alpha_idinfo)
			{
				T value = std::numeric_limits<T>::max();
				if constexpr (std::is_same_v<T, float32_t>)
				{
					value = 1.0f;
				}
				image_data = std::vector<T>(linked_layer->width() * linked_layer->height(), value);
			}
			else
			{
				throw std::invalid_argument(fmt::format(
					"SmartObjectLayer '{}': Invalid channel '{}' accessed while calling evaluate_channel(). This does not exist on the smart object",
					Layer<T>::m_LayerName,
					Enum::channelIDToString(idinfo.id))
				);
			}
			Render::ConstChannelBuffer<T> orig_buffer(image_data, linked_layer->width(), linked_layer->height());

			// Generate the warped result
			std::vector<T> channel_warp(this->width() * this->height());
			Render::ChannelBuffer<T> channel_warp_buffer(channel_warp, this->width(), this->height());

			auto& warp_mesh = this->evaluate_mesh_or_get_cached();

			// Restore the saved compression codec of the channel (if previously evaluated).
			auto compression_codec = Enum::Compression::ZipPrediction;
			if (ImageDataMixin<T>::m_ImageData.contains(idinfo))
			{
				compression_codec = ImageDataMixin<T>::m_ImageData[idinfo]->m_Compression;
			}

			// Finally apply the warp and store the cache
			m_SmartObjectWarp.apply(channel_warp_buffer, orig_buffer, warp_mesh);
			ImageDataMixin<T>::m_ImageData[idinfo] = std::make_unique<ImageChannel>(
				compression_codec,
				channel_warp,
				idinfo,
				Layer<T>::width(),
				Layer<T>::height(),
				Layer<T>::m_CenterX,
				Layer<T>::m_CenterY
			);
			this->store_was_cached(idinfo);
			return channel_warp;
		}
	};

private:

	/// Construct the SmartObjectLayer, initializing the structure and populating the warp (if necessary).
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void construct(Layer<T>::Params& parameters, std::filesystem::path filepath, LinkedLayerType linkage, std::optional<SmartObject::Warp> warp = std::nullopt)
	{
		PSAPI_PROFILE_FUNCTION();

		// The path needs to be absolute in order to link back properly
		filepath = std::filesystem::absolute(filepath);

		// Insert (or find) the linked layer and create a rescaled version of the image data.
		const auto& linkedlayer = m_LinkedLayers->insert(filepath, linkage);
		m_Hash = linkedlayer->hash();
		m_Filename = linkedlayer->path().filename().string();
		_m_LayerHash = generate_uuid();

		if (warp)
		{
			m_SmartObjectWarp = warp.value();
		}
		else
		{
			m_SmartObjectWarp = SmartObject::Warp::generate_default(linkedlayer->width(), linkedlayer->height());
		}
		
		Layer<T>::m_ColorMode = parameters.colormode;
		Layer<T>::m_LayerName = parameters.name;
		if (parameters.blendmode == Enum::BlendMode::Passthrough)
		{
			PSAPI_LOG_WARNING("ImageLayer", "The Passthrough blend mode is reserved for groups, defaulting to 'Normal'");
			Layer<T>::m_BlendMode = Enum::BlendMode::Normal;
		}
		else
		{
			Layer<T>::m_BlendMode = parameters.blendmode;
		}
		Layer<T>::m_Opacity = parameters.opacity;
		Layer<T>::m_IsVisible = parameters.visible;
		Layer<T>::m_IsLocked = parameters.locked;
		Layer<T>::m_CenterX = static_cast<float>(parameters.center_x);
		Layer<T>::m_CenterY = static_cast<float>(parameters.center_y);
		Layer<T>::m_Width = parameters.width;
		Layer<T>::m_Height = parameters.height;
		Layer<T>::parse_mask(parameters);

		// Transform the layer by the passed parameters' width and height
		if (parameters.width != 0 && parameters.height != 0)
		{
			auto affine_transform = Geometry::create_quad<double>(parameters.width, parameters.height);
			auto homography = Geometry::Operations::create_homography_matrix<double>(m_SmartObjectWarp.affine_transform(), affine_transform);

			transform(homography);
		}
		else
		{
			PSAPI_LOG_DEBUG("SmartObject", "Zero width or height passed to smart object layer constructor, the layer will instead be constructed using the linked image data's width and height.");
			Layer<T>::m_Width = static_cast<uint32_t>(linkedlayer->width());
			Layer<T>::m_Height = static_cast<uint32_t>(linkedlayer->height());
		}
	}

	/// Decode the structures passed from the PhotoshopFile object to parse the information for necessary
	/// to identify the smart object layer.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void decode(const AdditionalLayerInfo& local, const AdditionalLayerInfo& global, const std::string& name)
	{
		// Get the LinkedLayers from the global additional layer info
		auto g_linked_layers = global.get_tagged_blocks<LinkedLayerTaggedBlock>();

		const auto l_placed_layers = local.get_tagged_blocks<PlacedLayerTaggedBlock>();
		const auto l_placed_layers_data = local.get_tagged_blocks<PlacedLayerDataTaggedBlock>();

		// Prefer decoding via placed layer data as that is more up to date
		if (!g_linked_layers.empty() && !l_placed_layers_data.empty())
		{
			decode_placed_layer_data(l_placed_layers_data);
		}
		else if (!g_linked_layers.empty() && !l_placed_layers.empty())
		{
			// This currently throws a runtime_error. Perhaps we get around to adding
			// this in the future but most files should have PlacedLayerData anyways
			decode_placed_layer(l_placed_layers, name);
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Unable to decode SmartObject layer '%s' as we couldn't find the appropriate tagged blocks", name.c_str());
		}
	}

	/// Decode the smart object from the PlacedLayerData Tagged Block, this contains information such as 
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void decode_placed_layer_data(const std::vector<std::shared_ptr<PlacedLayerDataTaggedBlock>>& locals)
	{
		if (locals.size() > 1)
		{
			PSAPI_LOG_WARNING("SmartObject", "More than one PlacedLayerData tagged block found, this is likely an error in the file, continuing parsing with the first one found");
		}
		const auto& local = locals[0];


		const auto& descriptor = local->m_Descriptor;
		if (descriptor->contains("filterFX"))
		{
			PSAPI_LOG_WARNING("SmartObject", "Filter based warps are not supported at the moment (Edit->Puppet Warp and Edit->Perspective Warp)." \
				" These will not be represented properly in the API");
			return;
		}

		m_Hash = descriptor->at<UnicodeString>("Idnt").string();	// The identifier that maps back to the LinkedLayer

		// These we all ignore for the time being, we store them locally and just rewrite them back out later
		// This isn't necessarily in order
		{
			_m_LayerHash = descriptor->at<UnicodeString>("placed").getString();
			_m_PageNum = descriptor->at<int32_t>("PgNm");
			_m_NumPages = descriptor->at<int32_t>("totalPages");
			_m_Crop = descriptor->at<int32_t>("Crop");

			const auto _frame_step = descriptor->at<Descriptors::Descriptor>("frameStep");
			_m_FrameStepNumerator = _frame_step->at<int32_t>("numerator");
			_m_FrameStepDenominator = _frame_step->at<int32_t>("denominator");

			const auto _duration = descriptor->at<Descriptors::Descriptor>("duration");
			_m_DurationStepNumerator = _duration->at<int32_t>("numerator");
			_m_DurationStepDenominator = _duration->at<int32_t>("denominator");

			_m_FrameCount = descriptor->at<int32_t>("frameCount");
			_m_AntiAliasing = descriptor->at<int32_t>("Annt");

			_m_Type = descriptor->at<int32_t>("Type");

			_m_Comp = descriptor->at<int32_t>("comp");

			const auto _comp_info = descriptor->at<Descriptors::Descriptor>("compInfo");
			_m_CompInfoID = _comp_info->at<int32_t>("compID");
			_m_CompInfoOriginalID = _comp_info->at<int32_t>("originalCompID");
		}

		const auto size = descriptor->at<Descriptors::Descriptor>("Sz  ");		// The spaces are not a mistake
		m_OriginalSize = { size->at<double>("Wdth"), size->at<double>("Hght") };
		const auto resolution = descriptor->at<Descriptors::UnitFloat>("Rslt");	// In DPI
		m_Resolution = resolution->m_Value;

		const auto transform = descriptor->at<Descriptors::List>("Trnf");
		const auto non_affine_transform = descriptor->at<Descriptors::List>("nonAffineTransform");

		// The warp struct is present on all descriptors, if it is however a warp with a non-standard
		// number of subdivisions (i.e. not 4x4) the warp struct will be empty and instead we will be dealing with a quilt warp
		const auto warp = descriptor->at<Descriptors::Descriptor>("warp");
		SmartObject::Warp warpStruct;
		
		if (descriptor->contains("quiltWarp"))
		{
			warpStruct = SmartObject::Warp::_deserialize(descriptor->at<Descriptors::Descriptor>("quiltWarp"), transform, non_affine_transform, SmartObject::Warp::quilt_warp{});
		}
		else
		{
			warpStruct = SmartObject::Warp::_deserialize(warp, transform, non_affine_transform, SmartObject::Warp::normal_warp{});
		}
		m_SmartObjectWarp = warpStruct;
		
	}

	/// Generate a PlacedLayerData descriptor from the SmartObject that can be passed to the tagged blocks of the layer.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::unique_ptr<Descriptors::Descriptor> generate_placed_layer_data() const
	{
		auto placed_layer = std::make_unique<Descriptors::Descriptor>("null");

		placed_layer->insert("Idnt", UnicodeString(m_Hash, 2u));
		placed_layer->insert("placed", UnicodeString(_m_LayerHash, 2u));

		placed_layer->insert("PgNm", _m_PageNum);
		placed_layer->insert("totalPages", _m_NumPages);

		placed_layer->insert("Crop", _m_Crop);
		
		auto frame_step = std::make_unique<Descriptors::Descriptor>("null");
		frame_step->insert("numerator", _m_FrameStepNumerator);
		frame_step->insert("denominator", _m_FrameStepDenominator);
		placed_layer->insert("frameStep", std::move(frame_step));

		auto duration = Descriptors::Descriptor("null");
		duration.insert("numerator", _m_DurationStepNumerator);
		duration.insert("denominator", _m_DurationStepDenominator);
		placed_layer->insert("duration", std::make_unique<Descriptors::Descriptor>(std::move(duration)));

		placed_layer->insert("frameCount", _m_FrameCount);
		placed_layer->insert("Annt", _m_AntiAliasing);
		placed_layer->insert("Type", _m_Type);

		// Store the Transformation and non-affine transformation. 
		{
			auto [affine_transform, non_affine_transform] = m_SmartObjectWarp._generate_transform_descriptors();

			placed_layer->insert("Trnf", std::move(affine_transform));
			placed_layer->insert("nonAffineTransform", std::move(non_affine_transform));
		}

		// Store the warp, in the case of a quilt warp this would hold 2 descriptors
		// with the "warp" descriptor just being default initialized
		{
			if (m_SmartObjectWarp._warp_type() == SmartObject::Warp::WarpType::quilt)
			{
				auto quilt_descriptor = m_SmartObjectWarp._serialize();
				auto warp_descriptor = m_SmartObjectWarp._serialize_default(
					static_cast<size_t>(m_OriginalSize[0]), 
					static_cast<size_t>(m_OriginalSize[1])
				);
			
				placed_layer->insert("quiltWarp", std::move(quilt_descriptor));
				placed_layer->insert("warp", std::move(warp_descriptor));
			}
			else
			{
				auto warp_descriptor = m_SmartObjectWarp._serialize();

				placed_layer->insert("warp", std::move(warp_descriptor));
			}
		}

		{
			auto size_descriptor = std::make_unique<Descriptors::Descriptor>("Pnt ");

			size_descriptor->insert("Wdth", m_OriginalSize[0]);
			size_descriptor->insert("Hght", m_OriginalSize[1]);

			placed_layer->insert("Sz  ", std::move(size_descriptor));
		}
		{
			auto resolution_descriptor = std::make_unique<Descriptors::UnitFloat>(
				"Rslt",
				Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::UnitFloat),
				Descriptors::Impl::UnitFloatType::Density,
				m_Resolution
				);

			placed_layer->insert("Rslt", std::move(resolution_descriptor));
		}

		placed_layer->insert("comp", _m_Comp);
		{
			Descriptors::Descriptor comp_info_descriptor("null");

			comp_info_descriptor.insert("compID", _m_CompInfoID);
			comp_info_descriptor.insert("originalCompID", _m_CompInfoOriginalID);

			placed_layer->insert("compInfo", std::make_unique<Descriptors::Descriptor>(std::move(comp_info_descriptor)));
		}
		return std::move(placed_layer);
		
	}

	/// TBD: Unimplemented and might not ever implement depending on if we find many users who try to open up old files.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void decode_placed_layer(
		[[maybe_unused]] const std::vector<std::shared_ptr<PlacedLayerTaggedBlock>>& locals,
		[[maybe_unused]] const std::string& name)
	{
		PSAPI_LOG_ERROR("SmartObject", "Parsing of the PlacedLayerTaggedBlock is currently unimplemented, this is likely due to trying to open an older file.");
	}

};


extern template struct SmartObjectLayer<bpp8_t>;
extern template struct SmartObjectLayer<bpp16_t>;
extern template struct SmartObjectLayer<bpp32_t>;


PSAPI_NAMESPACE_END