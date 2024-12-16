#pragma once

#include "Macros.h"
#include "Layer.h"
#include "_ImageDataLayerType.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include "LayeredFile/LinkedData/LinkedLayerData.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/TaggedBlock.h"
#include "Core/Warp/SmartObjectWarp.h"
#include "Core/Render/Render.h"
#include "Util/Enum.h"

#include <fstream>
#include <string>

#include "fmt/core.h"
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
/// <b>Transformations:</b>
/// 
/// Unlike normal layers SmartObjects have slightly different transformation rules. Due to the fact that a smart object links
/// back to another image with a potentially different resolution we have to take this into account, additionally Photoshop
/// stores a transformation from original -> rescaled. However, we also have warps to deal with which leads to the PhotoshopAPI
/// exposing 3 transformation values for a SmartObjects.
/// 
/// - `original_width()` / `original_height()`
///		These represent the resolution of the original file image data, irrespective of what transforms are applied to it.
///		If you are e.g. loading a 4000x2000 jpeg these will return 4000 and 2000 respectively. These values may not be written to
/// 
/// - `transform_width()` / `transform_heigth()`
///		These represent the scaling factor applied to the smart object, i.e. if you do a transformation on the smart object 
///		this will be those values. If you have scaled down your 4000x2000 image to 2000x1000 image this will be 2000 and 1000 respectively.
///		Unlike original_width/original_height these may be modified to rescale the image. The underlying image data is evaluated lazily,
///		meaning you may modify these as many times as you like but only on access or write will we apply the transformation and warp.
/// 
/// - `width()` / `height()`
///		These represent the final dimensions of the SmartObject with the warp applied to it. These are read-only and describe the actual
///		width and height of the image data you can access through e.g. `get_channel()` or `get_image_data()`.
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
	/// parameters. Requires the `LayeredFile` to be passed so we can keep track of this global state of 
	/// linked layer data.
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
	/// \param link_externally	Whether to link the file externally (without saving it in the document). While this does reduce 
	///							file size, due to linking limitations it is usually recommended to leave this at its default `false`.
	///							If the given file already exists on the `LayeredFile<T>` e.g. when you link 2 layers with the same filepath
	///							the settings for the first layer are used instead of overriding the behaviour
	/// 
	SmartObjectLayer(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, bool link_externally = false)
	{
		this->construct(file, parameters, filepath, link_externally);
	}

	/// Initialize a SmartObject layer from a filepath.
	///
	/// This will internally load the given file (assuming it exists) into memory, decoding the full resolution
	/// image data as well as generating a resampled image data based on the resolution provided in the layers'
	/// parameters. Requires the `LayeredFile` to be passed so we can keep track of this global state of 
	/// linked layer data.
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
	///							retrieving it using `warp()`. After then modifying it the update warp will be lazily evaluated on write or 
	///							access. So you may modify it as many times as you want but only retrieving it will call the evaluation.
	/// 
	/// \param link_externally	Whether to link the file externally (without saving it in the document). While this does reduce 
	///							file size, due to linking limitations it is usually recommended to leave this at its default `false`.
	///							If the given file already exists on the `LayeredFile<T>` e.g. when you link 2 layers with the same filepath
	///							the settings for the first layer are used instead of overriding the behaviour
	/// 
	SmartObjectLayer(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, const SmartObject::Warp& warp, bool link_externally = false)
	{
		this->construct(file, parameters, filepath, link_externally, warp);
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
			{
				auto descriptor = generate_placed_layer_data();
				// Temporarily write the descriptor json to file so we can easily debug it.
				std::ofstream file_json(fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/placed_layer_descriptor_{}_out.json", Layer<T>::m_LayerName));
				file_json << descriptor.to_json().dump(4);
				file_json.flush();
			}
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Expected smart object layer to contain an AdditionalLayerInfo section");
		}
	}

	/// Retrieve the warp object that is stored on this layer. 
	SmartObject::Warp& warp() noexcept { return m_SmartObjectWarp; }
	const SmartObject::Warp& warp() const noexcept { return m_SmartObjectWarp; }


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

		LinkedLayerType type = link_externally ? LinkedLayerType::external : LinkedLayerType::data;

		LinkedLayers<T>& linked_layers = m_FilePtr->linked_layers();
		auto linked_layer = linked_layers.insert(path, type);

		m_Hash = linked_layer->hash();
		// Clear the cache before re-evaluation
		_m_CachedSmartObjectWarp = {};
		_m_CachedSmartObjectWarpMesh = {};

		evaluate_transforms();
	}

	/// Check whether the original image file stored by this smart object is linked externally
	bool linked_externally() const
	{
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to get original file linkage without the smart object knowing about the LayeredFile");
		}
		auto& linkedlayer_ptr = m_FilePtr->linked_layers().at(m_Hash);
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
		auto& linkedlayer_ptr = m_FilePtr->linked_layers().at(m_Hash);
		linkedlayer_ptr->type(linkage);
	}

	/// Retrieve the hashed value associated with the layer, this is what is used to identify the
	/// linked layer associated with this smart object (where the original image data is stored)
	const std::string& hash() const noexcept { return m_Hash; }

	/// Retrieve the filename associated with this smart object. 
	/// 
	/// Note that it is a filename, not a filepath as Photoshop doesn't store it that way. 
	/// If the layer is linked externally it is usually a good idea to have the file relative to the output 
	/// psd/psb file as otherwise photoshop won't be able to link it.
	const std::string& filename() const noexcept { return m_Filename; }

	/// Extract all the channels of the original image data.
	/// 
	/// Unlike the accessors `get_image_data()` and `get_channel()` this function gets the full resolution
	/// image data that is stored on the smart object, i.e. the original image data. This may be smaller
	/// or larger than the layers `width` or `height`. To get the actual resolution you can query: `original_width()` and `original_height()`
	data_type original_image_data()
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

	data_type get_image_data(bool copy = true) override
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
	void set_image_data(data_type data, const Enum::Compression compression = Enum::Compression::ZipPrediction) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers image data is not possible on a smart object as it links back to a file." \
			"Please refer to the documentation for more information. If you instead wish to replace the image" \
			" use the replace() function");
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
	/// \param document The document where the LinkedLayer is stored
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
	/// \param the offset to move the layer by
	void move(Geometry::Point2D<double> offset)
	{
		auto pts = std::vector<Geometry::Point2D<double>>(m_SmartObjectWarp.affine_transform());

		Geometry::QuadMesh<double> affine_mesh(pts, 2, 2);
		affine_mesh.move(offset);

		auto out_pts = affine_mesh.points();

		m_SmartObjectWarp.affine_transform(out_pts[0], out_pts[1], out_pts[2], out_pts[3]);
		evaluate_transforms();
	}


	/// Rotate the SmartObjectLayer (including any warps) by the given offset in degrees
	/// around the provided center point, this point does not have to lie on the pixels of the image
	void rotate(double offset, Geometry::Point2D<double> center)
	{
		auto pts = std::vector<Geometry::Point2D<double>>(m_SmartObjectWarp.affine_transform());

		Geometry::QuadMesh<double> affine_mesh(pts, 2, 2);
		affine_mesh.rotate(offset, center);

		auto out_pts = affine_mesh.points();

		m_SmartObjectWarp.affine_transform(out_pts[0], out_pts[1], out_pts[2], out_pts[3]);
		evaluate_transforms();
	}

	/// Rotate the SmartObjectLayer (including any warps) by the given offset in degrees
	/// around the center of the transformation
	void rotate(double offset)
	{
		auto pts = std::vector<Geometry::Point2D<double>>(m_SmartObjectWarp.affine_transform());
		Geometry::QuadMesh<double> affine_mesh(pts, 2, 2);

		rotate(offset, affine_mesh.bbox().center());
		evaluate_transforms();
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions.
	void scale(Geometry::Point2D<double> factor, Geometry::Point2D<double> center)
	{
		auto pts = std::vector<Geometry::Point2D<double>>(m_SmartObjectWarp.affine_transform());

		Geometry::QuadMesh<double> affine_mesh(pts, 2, 2);
		affine_mesh.scale(factor, center);

		auto out_pts = affine_mesh.points();

		m_SmartObjectWarp.affine_transform(out_pts[0], out_pts[1], out_pts[2], out_pts[3]);
		evaluate_transforms();
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions.
	void scale(double factor, Geometry::Point2D<double> center)
	{
		scale(Geometry::Point2D<double>(factor, factor), center);
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions.
	void scale(Geometry::Point2D<double> factor)
	{
		auto pts = std::vector<Geometry::Point2D<double>>(m_SmartObjectWarp.affine_transform());
		Geometry::QuadMesh<double> affine_mesh(pts, 2, 2);

		scale(factor, affine_mesh.bbox().center());
	}

	/// Scale the SmartObjectLayer (including any warps) by the given factor in both the x and y
	/// dimensions.
	void scale(double factor)
	{
		scale(Geometry::Point2D<double>(factor, factor));
	}


	/// The layers' width from 0 - 300,000
	uint32_t width() const noexcept override 
	{
		auto width = _m_CachedSmartObjectWarpMesh.bbox().width();
		return width;
	}
	/// Disabled for smart objects, please use move(), scale() and rotate() to modify the transformation instead.
	void width(uint32_t layer_width) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers width is not possible on a smart object as its bounds are dynamically computed." \
			"Please refer to the documentation for more information. If you instead wish to set the transformation" \
			" use the move(), scale(), rotate() function");
	}


	/// The layers' height from 0 - 300,000
	uint32_t height() const noexcept override
	{ 
		auto height = _m_CachedSmartObjectWarpMesh.bbox().height();
		return height;
	}
	/// Disabled for smart objects, please use move(), scale() and rotate() to modify the transformation instead.
	void height(uint32_t layer_height) override
	{
		PSAPI_LOG_ERROR("SmartObject",
			"Setting the layers height is not possible on a smart object as its bounds are dynamically computed." \
			"Please refer to the documentation for more information. If you instead wish to set the transformation" \
			" use the move(), scale(), rotate() function");
	}

	
	/// Set the x center coordinate, in the context of a smartobject this both offsets the bounding box as well as the
	/// transform.
	void center_x(float x_coord) noexcept override 
	{
		auto offset = x_coord - Layer<T>::m_CenterX;
		auto _pts_array = m_SmartObjectWarp.affine_transform();
		auto pts = std::vector<Geometry::Point2D<double>>(_pts_array.begin(), _pts_array.end());

		Geometry::QuadMesh<double> affine_mesh(pts, 2, 2);
		affine_mesh.move(Geometry::Point2D<double>( offset, 0));
		Layer<T>::m_CenterX += offset;

		auto out_pts = affine_mesh.points();

		m_SmartObjectWarp.affine_transform(out_pts[0], out_pts[1], out_pts[2], out_pts[3]);
	}

	/// Set the x center coordinate, in the context of a smartobject this both offsets the bounding box as well as the
	/// transform.
	void center_y(float y_coord) noexcept override
	{
		auto offset = y_coord - Layer<T>::m_CenterY;
		auto _pts_array = m_SmartObjectWarp.affine_transform();
		auto pts = std::vector<Geometry::Point2D<double>>(_pts_array.begin(), _pts_array.end());

		Geometry::QuadMesh<double> affine_mesh(pts, 2, 2);
		affine_mesh.move(Geometry::Point2D<double>(0, offset));
		Layer<T>::m_CenterY += offset;

		auto out_pts = affine_mesh.points();

		m_SmartObjectWarp.affine_transform(out_pts[0], out_pts[1], out_pts[2], out_pts[3]);
	}


	std::tuple<LayerRecord, ChannelImageData> to_photoshop(const Enum::ColorMode colorMode, const FileHeader& header) override
	{
		// TODO: this function needs to compute the mask and image data separately as e.g. a mask channel could 
		LayerRecord record{};
		ChannelImageData image_data{};
		return std::make_tuple(std::move(record), std::move(image_data));
	}
;

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
		// TODO: Find a way to not have this recalculate if our mesh is up-to-date but the image data is stale
		if (m_SmartObjectWarp == _m_CachedSmartObjectWarp)
		{
			PSAPI_LOG_DEBUG("SmartObject", "No need to re-evaluate the transform data as it matches the cached values");
			return;
		}
		if (!m_FilePtr)
		{
			PSAPI_LOG_ERROR("SmartObject", "Unable to evaluate the smart objects' transforms without access to the original file." \
				" If you believe this to be an error you can try recreating the layer making sure to pass a valid LayeredFile object.");
		}

		const auto& linked_layers = m_FilePtr->linked_layers();
		const auto& linked_layer = linked_layers.at(m_Hash);

		_m_CachedSmartObjectWarpMesh = m_SmartObjectWarp.surface().mesh(
			linked_layer->width() / 25,
			linked_layer->height() / 25,
			false // move_to_zero
		);
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

		// Get the warp mesh at a resolution of 25 pixels per subdiv. Ideally we'd lower this as we improve our algorithms
		const auto& image_data = linked_layer->get_image_data();
		auto warp_surface = m_SmartObjectWarp.surface();

		auto warp_mesh = warp_surface.mesh(
			linked_layer->width() / 25,
			linked_layer->height() / 25,
			true	// move_to_zero
		);

		_m_CachedSmartObjectWarpMesh = std::move(warp_mesh);

		// Generate a channel buffer that can fit the fully scaled warp
		std::vector<T> channel_warp(width() * height());
		Render::ImageBuffer<T> channel_warp_buffer(channel_warp, width(), height());

		for (const auto& [key, orig_channel] : image_data)
		{
			// We can reuse the same channel_buffer due to the fact that the warp.apply() method
			// will simply overwrite any values that lie on the warps uv space and seeing as the algorithm
			// is deterministic this will just overwrite previous values
			Render::ConstImageBuffer<T> orig_buffer(orig_channel, linked_layer->width(), linked_layer->height());
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

		// Store the cached values for next evaluation
		_m_CachedSmartObjectWarp = m_SmartObjectWarp;
	}

	void construct(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, bool link_externally, std::optional<SmartObject::Warp> warp = std::nullopt)
	{
		PSAPI_PROFILE_FUNCTION();

		m_FilePtr = &file;
		if (warp)
		{
			m_SmartObjectWarp = warp.value();
		}
		else
		{
			m_SmartObjectWarp.generate_default(parameters.width, parameters.height);
		}

		LinkedLayerType type = link_externally ? LinkedLayerType::external : LinkedLayerType::data;
		// Insert (or find) the linked layer and create a rescaled version of the image data.
		const auto& linkedlayer = file.linked_layers().insert(filepath, type);

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
	}

	
	void decode(const AdditionalLayerInfo& local, const AdditionalLayerInfo& global, const std::string& name)
	{
		// Get the LinkedLayers from the global additional layer info
		auto g_linkedLayer = global.getTaggedBlock<LinkedLayerTaggedBlock>(Enum::TaggedBlockKey::lrLinked);
		if (!g_linkedLayer)
		{
			g_linkedLayer = global.getTaggedBlock<LinkedLayerTaggedBlock>(Enum::TaggedBlockKey::lrLinked_8Byte);
		}
		const auto l_placedLayer = local.getTaggedBlock<PlacedLayerTaggedBlock>(Enum::TaggedBlockKey::lrPlaced);
		const auto l_placedLayerData = local.getTaggedBlock<PlacedLayerDataTaggedBlock>(Enum::TaggedBlockKey::lrPlacedData);

		// Prefer decoding via placed layer data as that is more up to date
		if (g_linkedLayer && l_placedLayerData)
		{
			decode_placed_layer_data(l_placedLayerData.value(), g_linkedLayer.value(), name);
		}
		// Fallback to placed layer if placed layer data wasn't present
		else if (g_linkedLayer && l_placedLayer)
		{
			// This currently throws a runtime_error. Perhaps we get around to adding
			// this in the future but most files should have PlacedLayerData anyways
			decode_placed_layer(l_placedLayer.value(), g_linkedLayer.value(), name);
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Unable to decode SmartObject layer '%s' as we couldn't find the appropriate tagged blocks", name.c_str());
		}
	}


	/// Decode the smart object from the PlacedLayerData Tagged Block, this contains information such as 
	void decode_placed_layer_data(const std::shared_ptr<PlacedLayerDataTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		const auto& descriptor = local->m_Descriptor;
		{
			// Temporarily write the descriptor json to file so we can easily debug it.
			std::ofstream file(fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/placed_layer_descriptor_{}.json", name));
			file << descriptor.to_json().dump(4);
			file.flush();
		}

		if (descriptor.contains("filterFX"))
		{
			PSAPI_LOG_WARNING("SmartObject", "Filter based warps are not supported at the moment (Edit->Puppet Warp and Edit->Perspective Warp)." \
				" These will not be represented properly in the API");
			return;
		}

		m_Hash = descriptor.at<UnicodeString>("Idnt").string();	// The identifier that maps back to the LinkedLayer

		// These we all ignore for the time being, we store them locally and just rewrite them back out later
		// This isn't necessarily in order
		{
			_m_LayerHash = descriptor.at<UnicodeString>("placed").getString();
			_m_PageNum = descriptor.at<int32_t>("PgNm");
			_m_NumPages = descriptor.at<int32_t>("totalPages");
			_m_Crop = descriptor.at<int32_t>("Crop");

			const auto& _frame_step = descriptor.at<Descriptors::Descriptor>("frameStep");
			_m_FrameStepNumerator = _frame_step.at<int32_t>("numerator");
			_m_FrameStepDenominator = _frame_step.at<int32_t>("denominator");

			const auto& _duration = descriptor.at<Descriptors::Descriptor>("duration");
			_m_DurationStepNumerator = _duration.at<int32_t>("numerator");
			_m_DurationStepDenominator = _duration.at<int32_t>("denominator");

			_m_FrameCount = descriptor.at<int32_t>("frameCount");
			_m_AntiAliasing = descriptor.at<int32_t>("Annt");

			_m_Type = descriptor.at<int32_t>("Type");

			_m_Comp = descriptor.at<int32_t>("comp");

			const auto& _comp_info = descriptor.at<Descriptors::Descriptor>("compInfo");
			_m_CompInfoID = _comp_info.at<int32_t>("compID");
			_m_CompInfoOriginalID = _comp_info.at<int32_t>("originalCompID");
		}

		const auto& size = descriptor.at<Descriptors::Descriptor>("Sz  ");		// The spaces are not a mistake
		m_OriginalSize = { size.at<double>("Wdth"), size.at<double>("Hght") };
		const auto& resolution = descriptor.at<Descriptors::UnitFloat>("Rslt");	// In DPI
		m_Resolution = resolution.m_Value;

		const auto& transform = descriptor.at<Descriptors::List>("Trnf");
		const auto& non_affine_transform = descriptor.at<Descriptors::List>("nonAffineTransform");

		// The warp struct is present on all descriptors, if it is however a warp with a non-standard
		// number of subdivisions (i.e. not 4x4) the warp struct will be empty and instead we will be dealing with a quilt warp
		const auto& warp = descriptor.at<Descriptors::Descriptor>("warp");
		SmartObject::Warp warpStruct;
		
		if (descriptor.contains("quiltWarp"))
		{
			warpStruct = SmartObject::Warp::_deserialize(descriptor.at<Descriptors::Descriptor>("quiltWarp"), transform, non_affine_transform, SmartObject::Warp::quilt_warp{});
		}
		else
		{
			warpStruct = SmartObject::Warp::_deserialize(warp, transform, non_affine_transform, SmartObject::Warp::normal_warp{});
		}
		m_SmartObjectWarp = warpStruct;
		
	}


	/// Generate a PlacedLayerData descriptor from the SmartObject checking whether the layers' linked data is still valid throwing otherwise.
	Descriptors::Descriptor generate_placed_layer_data() const
	{
		Descriptors::Descriptor placed_layer("null");

		placed_layer.insert("Idnt", UnicodeString(m_Hash, 2u));
		placed_layer.insert("placed", UnicodeString(_m_LayerHash, 2u));

		placed_layer.insert("PgNm", _m_PageNum);
		placed_layer.insert("totalPages", _m_NumPages);

		placed_layer.insert("Crop", _m_Crop);
		
		auto frame_step = Descriptors::Descriptor("null");
		frame_step.insert("numerator", _m_FrameStepNumerator);
		frame_step.insert("denominator", _m_FrameStepDenominator);
		placed_layer.insert("frameStep", frame_step);

		auto duration = Descriptors::Descriptor("null");
		duration.insert("numerator", _m_DurationStepNumerator);
		duration.insert("denominator", _m_DurationStepDenominator);
		placed_layer.insert("duration", duration);

		placed_layer.insert("frameCount", _m_FrameCount);
		placed_layer.insert("Annt", _m_AntiAliasing);
		placed_layer.insert("Type", _m_Type);

		// Store the Transformation and non-affine transformation. 
		{
			auto [affine_transform, non_affine_transform] = m_SmartObjectWarp._generate_transform_descriptors();

			placed_layer.insert("Trnf", affine_transform);
			placed_layer.insert("nonAffineTransform", non_affine_transform);
		}

		// Store the warp, in the case of a quilt warp this would hold 2 descriptors
		// with the "warp" descriptor just being default initialized
		{
			if (m_SmartObjectWarp._warp_type() == SmartObject::Warp::WarpType::quilt)
			{
				auto quilt_descriptor = m_SmartObjectWarp._serialize();
				auto warp_descriptor = m_SmartObjectWarp._serialize_default(m_OriginalSize[0], m_OriginalSize[1]);
			
				placed_layer.insert("quiltWarp", quilt_descriptor);
				placed_layer.insert("warp", warp_descriptor);
			}
			else
			{
				auto warp_descriptor = m_SmartObjectWarp._serialize();

				placed_layer.insert("warp", warp_descriptor);
			}
		}

		{
			Descriptors::Descriptor size_descriptor("Pnt ");

			size_descriptor.insert("Wdth", m_OriginalSize[0]);
			size_descriptor.insert("Hght", m_OriginalSize[1]);

			placed_layer.insert("Sz  ", size_descriptor);
		}
		{
			Descriptors::UnitFloat resolution_descriptor(
				"Rslt",
				Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::UnitFloat),
				Descriptors::Impl::UnitFloatType::Density,
				m_Resolution
				);

			placed_layer.insert("Rslt", resolution_descriptor);
		}

		placed_layer.insert("comp", _m_Comp);

		{
			Descriptors::Descriptor comp_info_descriptor("null");

			comp_info_descriptor.insert("compID", _m_CompInfoID);
			comp_info_descriptor.insert("originalCompID", _m_CompInfoOriginalID);

			placed_layer.insert("compInfo", comp_info_descriptor);
		}
		return placed_layer;
		
	}


	void decode_placed_layer(const std::shared_ptr<PlacedLayerTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		PSAPI_LOG_ERROR("SmartObject", "Parsing of the PlacedLayerTaggedBlock is currently unimplemented, this is likely due to trying to open an older file.");
	}

};


PSAPI_NAMESPACE_END