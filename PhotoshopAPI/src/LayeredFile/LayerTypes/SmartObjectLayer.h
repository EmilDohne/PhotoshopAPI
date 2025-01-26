#pragma once

#include "Macros.h"
#include "Layer.h"
#include "_ImageDataLayerType.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include "LayeredFile/LinkedData/LinkedLayerData.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/Warp/SmartObjectWarp.h"
#include "Core/Render/Render.h"
#include "Util/Enum.h"

#include <fstream>
#include <string>

#include "fmt/core.h"
#include <Eigen/Dense>
#include <OpenImageIO/imageio.h>


PSAPI_NAMESPACE_BEGIN

// Forward declare LayeredFile here
template <typename T>
struct LayeredFile;


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
struct SmartObjectLayer : public _ImageDataLayerType<T>
{
	using _ImageDataLayerType<T>::data_type;
	using _ImageDataLayerType<T>::storage_type;

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
	/// 
	SmartObjectLayer(LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, LinkedLayerType linkage = LinkedLayerType::data)
	{
		this->construct(file, parameters, filepath, linkage);
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
	/// 
	SmartObjectLayer(LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, const SmartObject::Warp& warp, LinkedLayerType linkage = LinkedLayerType::data)
	{
		this->construct(file, parameters, filepath, linkage, warp);
	}

	/// Generate a SmartObjectLayer from a Photoshop File object. This is for internal uses and not intended to be used by users directly. Please use the other
	/// constructors instead.
	SmartObjectLayer(LayeredFile<T>& file, const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo)
		: _ImageDataLayerType<T>(layerRecord, channelImageData, header)
	{
		// Local and global additional layer info in this case refer to the one stored on the individual layer and the one 
		// stored on the LayerAndMaskInfo section respectively
		m_FilePtr = &file;
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
	SmartObject::Warp warp() const noexcept { return m_SmartObjectWarp; }

	/// Set the warp object held by this layer, this function may be used to replace the warp with e.g. the
	/// warp from another layer
	void warp(SmartObject::Warp _warp) 
	{ 
		m_SmartObjectWarp = std::move(_warp);
		_m_CachedSmartObjectWarp = {};
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
	void replace(std::filesystem::path path, bool link_externally = false)
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to replace the smart objects' image without access to the original LayeredFile." \
				" If you believe this to be an error you can try recreating the layer making sure to pass a valid LayeredFile object.");
		}
		auto previous_bbox = m_SmartObjectWarp._warp_bounds();

		// Insert the new path, if it already exists insert() will return a reference to the previous layer.
		LinkedLayerType type = link_externally ? LinkedLayerType::external : LinkedLayerType::data;
		LinkedLayers<T>& linked_layers = m_FilePtr->linked_layers();
		auto linked_layer = linked_layers.insert(path, type);

		m_Filename = linked_layer->path().filename().string();
		m_Hash = linked_layer->hash();
		_m_LayerHash = generate_uuid();
		// Clear the cache before re-evaluation
		_m_CachedSmartObjectWarp = {};
		_m_CachedSmartObjectWarpMesh = {};

		// Update the warp original bounds so it knows the input image data scaled.
		Geometry::BoundingBox<double> bbox(
			Geometry::Point2D<double>(0.0f, 0.0f),
			Geometry::Point2D<double>(linked_layer->width(), linked_layer->height()));

		// Finally we also need to rescale the warp points, this is because they are in the original images' coordinate space.
		auto pts = m_SmartObjectWarp.points();
		auto scalar = Geometry::Point2D<double>(bbox.width() / previous_bbox.width(), bbox.height() / previous_bbox.height());
		auto pivot = previous_bbox.minimum;

		Geometry::Operations::scale(pts, scalar, pivot);

		m_SmartObjectWarp.points(pts);
	}

	/// Check whether the original image file stored by this smart object is linked externally
	bool linked_externally() const
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to get original file linkage without the smart object knowing about the LayeredFile");
		}
		auto linkedlayer_ptr = m_FilePtr->linked_layers().at(m_Hash);
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
	void set_linkage(LinkedLayerType linkage)
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to set original file linkage without the smart object knowing about the LayeredFile");
		}
		auto linkedlayer_ptr = m_FilePtr->linked_layers().at(m_Hash);
		linkedlayer_ptr->type(linkage);
	}

	/// Retrieve the hashed value associated with the layer, this is what is used to identify the
	/// linked layer associated with this smart object (where the original image data is stored)
	const std::string& hash() const noexcept { return m_Hash; }

	/// Retrieve the filename associated with this smart object.
	const std::string& filename() const noexcept { return m_Filename; }

	/// Retrieve the filepath associated with this smart object. Depending on how the 
	/// Smart object is linked (`external` or `data`) this may not be written to disk.
	std::filesystem::path filepath() const 
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query filepath without the smart object knowing about the LayeredFile");
		}
		const auto& linkedlayers = m_FilePtr->linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);

		return layer->path();
	}

	/// Extract all the channels of the original image data.
	/// 
	/// Unlike the accessors `get_image_data()` and `get_channel()` this function gets the full resolution
	/// image data that is stored on the smart object, i.e. the original image data. This may be smaller
	/// or larger than the layers `width` or `height`. To get the actual resolution you can query: `original_width()` and `original_height()`
	_ImageDataLayerType<T>::data_type original_image_data()
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query original file image data without the smart object knowing about the LayeredFile");
		}
		const auto& linkedlayers = m_FilePtr->linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);

		return layer->get_image_data();
	}


	std::vector<T> get_channel(const Enum::ChannelID channelID, bool copy = true) override
	{
		evaluate_image_data();
		return _ImageDataLayerType<T>::get_channel(channelID, copy);
	}

	std::vector<T> get_channel(int16_t channelID, bool copy = true) override
	{
		evaluate_image_data();
		return _ImageDataLayerType<T>::get_channel(channelID, copy);
	}

	_ImageDataLayerType<T>::data_type get_image_data(bool copy = true) override
	{
		evaluate_image_data();
		return _ImageDataLayerType<T>::get_image_data(copy);
	}


	/// Disabled for the smart object as it links back to a file, please use `replace()` instead.
	void set_channel(const Enum::ChannelID channelID, const std::span<const T> data, const Enum::Compression compression = Enum::Compression::ZipPrediction) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers image data is not possible on a smart object as it links back to a file." \
			"Please refer to the documentation for more information. If you instead wish to replace the image" \
			" use the replace() function");
	}
	/// Disabled for the smart object as it links back to a file, please use `replace()` instead.
	void set_channel(const int16_t index, const std::span<const T> data, const Enum::Compression compression = Enum::Compression::ZipPrediction) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers image data is not possible on a smart object as it links back to a file." \
			"Please refer to the documentation for more information. If you instead wish to replace the image" \
			" use the replace() function");
	}
	/// Disabled for the smart object as it links back to a file, please use `replace()` instead.
	void set_image_data(std::unordered_map<int16_t, std::vector<T>>&& data, const Enum::Compression compression = Enum::Compression::ZipPrediction) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers image data is not possible on a smart object as it links back to a file." \
			"Please refer to the documentation for more information. If you instead wish to replace the image" \
			" use the replace() function");
	}
	/// Disabled for the smart object as it links back to a file, please use `replace()` instead.
	void set_image_data(std::unordered_map<Enum::ChannelID, std::vector<T>>&& data, const Enum::Compression compression = Enum::Compression::ZipPrediction) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers image data is not possible on a smart object as it links back to a file." \
			"Please refer to the documentation for more information. If you instead wish to replace the image" \
			" use the replace() function");
	}
	/// Disabled for the smart object as it links back to a file, please use `replace()` instead.
	void set_image_data(_ImageDataLayerType<T>::data_type data, const Enum::Compression compression = Enum::Compression::ZipPrediction) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers image data is not possible on a smart object as it links back to a file." \
			"Please refer to the documentation for more information. If you instead wish to replace the image" \
			" use the replace() function");
	}

	/// Return the stored image data in a zero-copy way. As the SmartObject is lazily evaluated this may give out of date image data.
	const _ImageDataLayerType<T>::storage_type& image_data() const override
	{
		return _ImageDataLayerType<T>::image_data();
	}

	/// Retrieve the original image datas' width.
	///
	/// This does not have the same limitation as Photoshop layers of being limited
	/// to 30,000 or 300,000 pixels depending on the file type
	/// 
	/// \throws std::runtime_error if the hash defined by `hash()` is not valid for the document
	/// 
	/// \returns The width of the original image data
	size_t original_width() const
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query original file width without the smart object knowing about the LayeredFile");
		}
		const auto& linkedlayers = m_FilePtr->linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);
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
	size_t original_height() const
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to query original file height without the smart object knowing about the LayeredFile");
		}
		const auto& linkedlayers = m_FilePtr->linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);
		return layer->height();
	}


	/// Move the SmartObjectLayer (including any warps) by the given offset
	///
	/// \param offset the offset to move the layer by
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
	void scale(double factor, Geometry::Point2D<double> center)
	{
		scale(Geometry::Point2D<double>(factor, factor), center);
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions around the layers center.
	/// 
	/// \param factor The scalar factor
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
	void scale(double factor)
	{
		scale(Geometry::Point2D<double>(factor, factor));
	}

	/// Apply a transformation (affine or non affine) to the smart object. This can be used in order
	/// to e.g. skew or perspective transform the image. Automatically splits the matrix into it's 
	/// affine and non-affine transformations and applies these separately.
	/// 
	/// \param matrix The transformation matrix, will internally be split into its affine and non-affine components.
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

	/// The layers' width from 0 - 300,000
	uint32_t width() const noexcept override 
	{
		return Layer<T>::m_Width;
	}
	/// Set the layers' width, analogous to calling `scale()` while only scaling around the x axis.
	void width(uint32_t layer_width) override
	{
		// Explicitly dont use the width() function on the SmartObjectLayer to avoid
		// floating to int conversion issues
		auto current_width = _m_CachedSmartObjectWarpMesh.bbox().width();

		double scalar_x = static_cast<double>(layer_width) / current_width;
		auto center = Geometry::Point2D<double>(Layer<T>::center_x(), Layer<T>::center_y());

		// This will call evaluate_transforms.
		scale(Geometry::Point2D<double>(scalar_x, 1.0f), center);
	}

	/// The layers' height from 0 - 300,000
	uint32_t height() const noexcept override
	{
		return Layer<T>::m_Height;
	}
	/// Set the layers' height, analogous to calling `scale()` while only scaling around the y axis.
	void height(uint32_t layer_height) override
	{
		// Explicitly dont use the height() function on the SmartObjectLayer to avoid
		// floating to int conversion issues
		auto current_height = _m_CachedSmartObjectWarpMesh.bbox().height();

		double scalar_y = static_cast<double>(layer_height) / current_height;
		auto center = Geometry::Point2D<double>(Layer<T>::center_x(), Layer<T>::center_y());

		// This will call evaluate_transforms.
		scale(Geometry::Point2D<double>(1.0f, scalar_y), center);
	}
	
	/// Return the x center coordinate, evaluates coordinates before doing so.
	float center_x() const noexcept override
	{
		return Layer<T>::m_CenterX;
	}
	/// Set the x center coordinate, analogous to calling `move()` while only moving on the x axis
	void center_x(float x_coord) noexcept override 
	{
		auto offset = x_coord - Layer<T>::m_CenterX;
		// this will call evaluate_transforms
		move(Geometry::Point2D<double>(offset, 0.0f));

	}

	/// Return the y center coordinate, evaluates coordinates before doing so.
	float center_y() const noexcept override
	{
		return Layer<T>::m_CenterY;
	}
	/// Set the y center coordinate, analogous to calling `move()` while only moving on the y axis
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
	void reset_transform()
	{
		auto current_transform = m_SmartObjectWarp.non_affine_transform();
		auto goal_transform = Geometry::create_quad<double>(original_width(), original_height());

		auto homography = Geometry::Operations::create_homography_matrix<double>(current_transform, goal_transform);
		transform(homography);
	}

	/// Reset the warp (not the transformations) applied to the Smart Object.
	/// 
	/// If you instead wish to clear the transformations you can use the `reset_transform()` function.
	/// 
	/// These two may be used in combination and sequence, so it is perfectly valid to call `reset_transform`
	/// and `reset_warp` in any order
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
	std::tuple<LayerRecord, ChannelImageData> to_photoshop(const Enum::ColorMode colorMode, const FileHeader& header) override
	{
		// Evaluate transforms and image data to be sure these are up to date
		evaluate_transforms();
		evaluate_image_data();

		PascalString lrName = Layer<T>::generate_name();
		ChannelExtents extents = generate_extents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY), header);
		uint16_t channelCount = _ImageDataLayerType<T>::m_ImageData.size() + static_cast<uint16_t>(Layer<T>::m_LayerMask.has_value());

		uint8_t clipping = 0u;	// No clipping mask for now
		LayerRecords::BitFlags bitFlags(Layer<T>::m_IsLocked, !Layer<T>::m_IsVisible, false);
		std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generate_mask(header);
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
		auto channelData = _ImageDataLayerType<T>::generate_channel_image_data();
		auto& channelInfoVec = std::get<0>(channelData);
		ChannelImageData channelImgData = std::move(std::get<1>(channelData));

		LayerRecord lrRecord = LayerRecord(
			lrName,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			channelCount,
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
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to generate tagged blocks for the layer as it does not hold a valid reference to a LayeredFile.." \
				" If you believe this to be an error you can try recreating the layer making sure to pass a valid LayeredFile object.");
		}

		auto& linked_layers = m_FilePtr->linked_layers();
		auto linked_layer = linked_layers.at(m_Hash);

		// Before generating the descriptor we need to update the original width and height
		// as we don't keep track of these usually.
		m_OriginalSize[0] = static_cast<float>(linked_layer->width());
		m_OriginalSize[1] = static_cast<float>(linked_layer->height());

		auto descriptor = generate_placed_layer_data();
		auto block_ptr = std::make_shared<PlacedLayerDataTaggedBlock>(std::move(descriptor));
		blocks.push_back(block_ptr);

		return blocks;
	}


private:
	/// Pointer back to the original file object this layer was created with.
	/// Under normal circumstances this should always be available as the 
	/// SmartObjectLayer is one of the few layers that can only be constructed
	/// with knowledge of the file
	LayeredFile<T>* m_FilePtr = nullptr;

	/// The warp on the object, may be a no-op in the case of no warp.
	SmartObject::Warp m_SmartObjectWarp;
	/// We cache the smart object warp on evaluation to use for comparison
	SmartObject::Warp _m_CachedSmartObjectWarp;
	/// We also cache the smart object mesh for easy access to the bbox for width() 
	/// and height()
	Geometry::QuadMesh<double> _m_CachedSmartObjectWarpMesh;

	/// The hash of the file, this is the same as what is stored on the LinkedLayerData
	/// and identical files are automatically de-duplicated
	std::string m_Hash;

	/// The filename that the Smart Object was constructed with.
	std::string m_Filename;

	/// The orginal width and height of the image data
	std::array<double, 2> m_OriginalSize;

	/// Resolution in DPI
	double m_Resolution = 72.0f;

	/// Internal values for roundtripping:

	/// Hash of the layer itself, doesn't seem to relate back to the LinkedLayers
	std::string _m_LayerHash;

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
	

	/// Evaluates the transformation (updates _m_CachedSmartObjectWarpMesh) meaning grabbing the bbox width and height will give the 
	/// latest warp information
	void evaluate_transforms()
	{
		PSAPI_PROFILE_FUNCTION();
		if (m_SmartObjectWarp == _m_CachedSmartObjectWarp)
		{
			PSAPI_LOG_DEBUG("SmartObject", "No need to re-evaluate the transform data as it matches the cached values");
			return;
		}

		_m_CachedSmartObjectWarpMesh = m_SmartObjectWarp.surface().mesh(
			75,		// x resolution
			75,		// y resolution
			false	// move_to_zero
		);

		Layer<T>::m_CenterX = _m_CachedSmartObjectWarpMesh.bbox().center().x;
		Layer<T>::m_CenterX -= m_FilePtr->width() / 2;
		Layer<T>::m_CenterY = _m_CachedSmartObjectWarpMesh.bbox().center().y;
		Layer<T>::m_CenterY -= m_FilePtr->height() / 2;

		Layer<T>::m_Width = static_cast<uint32_t>(std::round(_m_CachedSmartObjectWarpMesh.bbox().width()));
		Layer<T>::m_Height = static_cast<uint32_t>(std::round(_m_CachedSmartObjectWarpMesh.bbox().height()));
	}

	/// Lazily evaluates (and updates if necessary) the ImageData of the SmartObjectLayer. Checks whether
	/// the cached warp and transform values match what is cached on the object, if that is not the case
	/// we recompute the image data and assign the warp to m_Warp.
	void evaluate_image_data()
	{
		PSAPI_PROFILE_FUNCTION();
		if (m_SmartObjectWarp == _m_CachedSmartObjectWarp)
		{
			PSAPI_LOG_DEBUG("SmartObject", "No need to re-evaluate the image data as it matches the cached values");
			return;
		}

		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to evaluate the smart objects' image data without access to the original file." \
				" If you believe this to be an error you can try recreating the layer making sure to pass a valid LayeredFile object.");
		}
		const auto& layered_file = *m_FilePtr;

		const LinkedLayers<T>& linkedlayers = layered_file.linked_layers();
		const std::shared_ptr<LinkedLayerData<T>> linked_layer = linkedlayers.at(m_Hash);

		// Get the warp mesh at a resolution of 15 pixels per subdiv. Ideally we'd lower this as we improve our algorithms
		const auto& image_data = linked_layer->get_image_data();
		auto warp_surface = m_SmartObjectWarp.surface();

		auto warp_mesh = warp_surface.mesh(
			linked_layer->width() / 15,
			linked_layer->height() / 15,
			true	// move_to_zero
		);
		_m_CachedSmartObjectWarpMesh = std::move(warp_mesh);

		// Generate a channel buffer that can fit the fully scaled warp
		std::vector<T> channel_warp(width() * height());
		Render::ChannelBuffer<T> channel_warp_buffer(channel_warp, width(), height());

		for (const auto& [key, orig_channel] : image_data)
		{
			// We can reuse the same channel_buffer due to the fact that the warp.apply() method
			// will simply overwrite any values that lie on the warps uv space and seeing as the algorithm
			// is deterministic this will just overwrite previous values
			Render::ConstChannelBuffer<T> orig_buffer(orig_channel, linked_layer->width(), linked_layer->height());

			m_SmartObjectWarp.apply(channel_warp_buffer, orig_buffer, _m_CachedSmartObjectWarpMesh);
			_ImageDataLayerType<T>::m_ImageData[key] = std::make_unique<ImageChannel>(
				Enum::Compression::ZipPrediction,
				channel_warp,
				key,
				width(),
				height(),
				Layer<T>::m_CenterX,
				Layer<T>::m_CenterY
			);
		}

		// If the original data held no alpha we need to account for this by warping in place the alpha and setting it
		// so that we dont get a result on black
		auto alpha_id = Enum::toChannelIDInfo(Enum::ChannelID::Alpha, Layer<T>::m_ColorMode);
		if (!image_data.contains(alpha_id))
		{
			T value = std::numeric_limits<T>::max();
			if constexpr (std::is_same_v<T, float32_t>)
			{
				value = 1.0f;
			}

			std::vector<T> channel(linked_layer->width() * linked_layer->height(), value);
			Render::ConstChannelBuffer<T> orig_buffer(channel, linked_layer->width(), linked_layer->height());

			m_SmartObjectWarp.apply<T>(channel_warp_buffer, orig_buffer, _m_CachedSmartObjectWarpMesh);

			_ImageDataLayerType<T>::m_ImageData[alpha_id] = std::make_unique<ImageChannel>(
				Enum::Compression::ZipPrediction,
				channel_warp,
				alpha_id,
				width(),
				height(),
				Layer<T>::m_CenterX,
				Layer<T>::m_CenterY
			);
		}

		// Store the cached values for next evaluation
		_m_CachedSmartObjectWarp = m_SmartObjectWarp;
	}


	/// Construct the SmartObjectLayer, initializing the structure and populating the warp (if necessary).
	void construct(LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, LinkedLayerType linkage, std::optional<SmartObject::Warp> warp = std::nullopt)
	{
		PSAPI_PROFILE_FUNCTION();

		// The path needs to be absolute in order to link back properly
		filepath = std::filesystem::absolute(filepath);

		m_FilePtr = &file;

		// Insert (or find) the linked layer and create a rescaled version of the image data.
		const auto& linkedlayer = file.linked_layers().insert(filepath, linkage);
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
		auto warp_surface = m_SmartObjectWarp.surface();
		auto warp_mesh = warp_surface.mesh(
			linkedlayer->width() / 25,
			linkedlayer->height() / 25,
			true	// move_to_zero
		);
		_m_CachedSmartObjectWarpMesh = std::move(warp_mesh);
		_m_CachedSmartObjectWarp = m_SmartObjectWarp;
		
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
		Layer<T>::m_CenterX = parameters.center_x;
		Layer<T>::m_CenterY = parameters.center_y;
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
			Layer<T>::m_Width = linkedlayer->width();
			Layer<T>::m_Height = linkedlayer->height();
		}
	}

	/// Decode the structures passed from the PhotoshopFile object to parse the information for necessary
	/// to identify the smart object layer.
	void decode(const AdditionalLayerInfo& local, const AdditionalLayerInfo& global, const std::string& name)
	{
		// Get the LinkedLayers from the global additional layer info
		auto g_linked_layers = global.get_tagged_blocks<LinkedLayerTaggedBlock>();

		const auto l_placed_layers = local.get_tagged_blocks<PlacedLayerTaggedBlock>();
		const auto l_placed_layers_data = local.get_tagged_blocks<PlacedLayerDataTaggedBlock>();

		// Prefer decoding via placed layer data as that is more up to date
		if (!g_linked_layers.empty() && !l_placed_layers_data.empty())
		{
			decode_placed_layer_data(l_placed_layers_data, name);
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
	void decode_placed_layer_data(
		const std::vector<std::shared_ptr<PlacedLayerDataTaggedBlock>>& locals, 
		const std::string& name)
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
				auto warp_descriptor = m_SmartObjectWarp._serialize_default(m_OriginalSize[0], m_OriginalSize[1]);
			
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
	void decode_placed_layer(
		const std::vector<std::shared_ptr<PlacedLayerTaggedBlock>>& locals,
		const std::string& name)
	{
		PSAPI_LOG_ERROR("SmartObject", "Parsing of the PlacedLayerTaggedBlock is currently unimplemented, this is likely due to trying to open an older file.");
	}

};


PSAPI_NAMESPACE_END