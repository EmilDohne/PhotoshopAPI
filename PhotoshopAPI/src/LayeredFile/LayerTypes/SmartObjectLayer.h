#pragma once

#include "Macros.h"
#include "Layer.h"
#include "_ImageDataLayerType.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

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


/// This struct holds no data, we just use it to identify its type.
/// We could hold references here 
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
	SmartObjectLayer(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath)
	{
		auto warp = SmartObject::Warp{};
		this->construct(file, parameters, filepath, warp);
	}

	SmartObjectLayer(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, const SmartObject::Warp& warp)
	{
		this->construct(file, parameters, filepath, warp);
	}

	/// Generate a SmartObjectLayer from a Photoshop File object. This is for internal uses and not intended to be used by users directly. Please use the other
	/// constructors
	SmartObjectLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo) 
		: _ImageDataLayerType<T>(layerRecord, channelImageData, header)
	{
		// Local and global additional layer info in this case refer to the one stored on the individual layer and the one 
		// stored on the LayerAndMaskInfo section respectively
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
	SmartObject::Warp& warp() noexcept { return m_SmartObjectWarp; }
	const SmartObject::Warp& warp() const noexcept{ return m_SmartObjectWarp; }

	/// Retrieve the hashed value associated with the layer, this is what is used to identify the
	/// linked layer associated with this smart object (where the original image data is stored)
	const std::string& hash() const noexcept { return m_Hash; }

	/// Retrieve the filename associated with this smart object. Note that it is a filename,
	/// not a filepath as Photoshop doesn't store it that way. If the layer is linked externally
	/// it is usually a good idea to have the file relative to the output psd/psb file as otherwise
	/// photoshop won't be able to link it.
	const std::string& filename() const noexcept { return m_Filename; }

	/// \defgroup original_data The image data the smart object links to
	/// 
	/// Photoshop stores both a resampled image data with any warps etc. applied to them 
	/// 
	/// @{

	/// Extract all the channels of the original image data.
	/// 
	/// Unlike the accessors `image_data()` and `channel()` this function gets the full resolution
	/// image data that is stored on the smart object, i.e. the original image data. This may be smaller
	/// or larger than the layers `width` or `height`. To get the actual resolution you can query: `original_width()` and `original_height()`
	/// 
	/// \tparam ExecutionPolicy the execution policy to get the image data with
	/// 
	/// \param document The document associated with this layer, this is required to query the original image data
	/// \param policy The execution policy for the image data decompression, defaults to parallel
	template <typename ExecutionPolicy = std::execution::parallel_policy, std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, int> = 0>
	_ImageDataLayerType<T>::data_type original_image_data(const LayeredFile<T>& document, const ExecutionPolicy policy = std::execution::par)
	{
		const auto& linkedlayers = document.linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);

		return layer.image_data();
	}


	/// Retrieve the original image datas' width.
	///
	/// This does not have the same limitation as Photoshop layers of being limited
	/// to 30,000 or 300,000 pixels depending on the file type
	/// 
	/// \throws std::runtime_error if the hash defined by `hash()` is not valid for the document
	/// 
	/// \returns The width of the original image data
	size_t original_width(const LayeredFile<T>& document) const
	{
		const auto& linkedlayers = document.linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);
		return layer.width();
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
	size_t original_height(const LayeredFile<T>& document) const
	{
		const auto& linkedlayers = document.linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);
		return layer.width();
	}

	/// @}


private:

	SmartObject::Warp m_SmartObjectWarp;
	std::string m_Hash;
	std::string m_Filename;
	

	void construct(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, const SmartObject::Warp& warp)
	{
		PSAPI_PROFILE_FUNCTION();

		// Insert (or find) the linked layer and create a rescaled version of the image data.
		const auto& linkedlayer = file.linked_layers().insert(filepath);
		const auto& image_data = linkedlayer.image_data(parameters.width, parameters.height);
		std::for_each(std::execution::par, image_data.begin(), image_data.end(), [&](const auto& pair)
			{
				const auto& key = pair.first;
				const auto& channel = pair.second;
				_ImageDataLayerType<T>::m_ImageData[key] = std::make_unique<ImageChannel<T>>(
					parameters.compression,
					channel,
					key,
					parameters.width,
					parameters.height,
					parameters.center_x,
					parameters.center_y);
			});


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

	/// Member data that is used only for roundtripping. We initialize these to sensible defaults on write.
	/// Some of these may get promoted to proper members once we add functionality for modifying these

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
			decodePlacedLayerData(l_placedLayerData.value(), g_linkedLayer.value(), name);
		}
		// Fallback to placed layer if placed layer data wasn't present
		else if (g_linkedLayer && l_placedLayer)
		{
			decodePlacedLayer(l_placedLayer.value(), g_linkedLayer.value(), name); // Call decodePlacedLayer here instead of decodePlacedLayerData
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Unable to decode SmartObject layer '%s' as we couldn't find the appropriate tagged blocks", name.c_str());
		}
	}

	/// Decode the smart object from the PlacedLayerData Tagged Block, this contains information such as 
	void decodePlacedLayerData(const std::shared_ptr<PlacedLayerDataTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		const auto& descriptor = local->m_Descriptor;

		std::ofstream file(fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/placed_layer_descriptor_{}.json", name));
		file << descriptor.to_json().dump(4);
		file.flush();

		const auto& identifier = descriptor.at("Idnt");	// The identifier that maps back to the LinkedLayer

		// These we all ignore for the time being, we store them locally and just rewrite them back out later
		const auto& _placed = descriptor.at("placed");
		const auto& _page_num = descriptor.at("PgNm");
		const auto& _total_pages = descriptor.at("totalPages");
		const auto& _crop = descriptor.at("Crop");
		const auto& _frame_step = descriptor.at("frameStep");
		const auto& _duration = descriptor.at("duration");
		const auto& _frame_count = descriptor.at("frameCount");
		const auto& _anti_alias = descriptor.at("Annt");

		const auto& type = descriptor.at("Type");
		const auto& transform = descriptor.at<Descriptors::List>("Trnf");
		const auto& non_affine_transform = descriptor.at<Descriptors::List>("nonAffineTransform");

		// The warp struct is present on all descriptors, if it is however a warp with a non-standard
		// number of subdivisions (i.e. not 4x4) the warp struct will be empty and instead we will be dealing with a quilt warp
		const auto& warp = descriptor.at<Descriptors::Descriptor>("warp");
		SmartObject::Warp warpStruct;
		if (descriptor.contains("quiltSmartObject::Warp"))
		{
			warpStruct = SmartObject::Warp::deserialize(descriptor.at<Descriptors::Descriptor>("quiltSmartObject::Warp"), transform, non_affine_transform, SmartObject::Warp::quilt_warp{});
			renderMesh(warpStruct, fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/warp_grid{}.jpg", name));
		}
		else
		{
			warpStruct = SmartObject::Warp::deserialize(warp, transform, non_affine_transform, SmartObject::Warp::normal_warp{});
			renderMesh(warpStruct, fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/warp_grid{}.jpg", name));
		}
		m_SmartObjectWarp = warpStruct;
		const auto& size = descriptor.at("Sz  ");		// The spaces are not a mistake
		const auto& resolution = descriptor.at("Rslt");	// In DPI

		const auto& _comp = descriptor.at("comp");
		const auto& _comp_info = descriptor.at("compInfo");
	}

	void decodePlacedLayer(const std::shared_ptr<PlacedLayerTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		PSAPI_LOG_ERROR("SmartObject", "Parsing of the PlacedLayerTaggedBlock is currently unimplemented, this is likely due to trying to open an older file.");
	}

	void renderMesh(const SmartObject::Warp& warp, std::string filename)
	{
		auto bounds = warp.bounds();
		auto height = bounds[2] - bounds[0];
		auto width = bounds[3] - bounds[1];

		auto surface = warp.surface();
		auto subdivided_mesh = surface.mesh(100, 100, warp.non_affine_mesh());
		{
			std::vector<T> pixels(height * width, 0);
			auto buffer = Render::ImageBuffer<T>(pixels, width, height);
			
			/*Render::render_mesh<T>(
				buffer,
				Geometry::Mesh<double>(warp.m_SmartObject::WarpPoints, warp.u_dimensions(), warp.v_dimensions()),
				255,
				"C:/Windows/Fonts/arial.ttf",
				true);*/
			//Render::render_bezier_surface<T>(buffer, surface, 255, 100, 100);
			Render::render_mesh<T>(buffer, subdivided_mesh, 255);

			std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);
			if (!out)
				return;  // error
			OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::UINT8);
			out->open(filename, spec);
			out->write_image(OIIO::TypeDesc::UINT8, pixels.data());
			out->close();
		}

		{
			std::vector<T> pixels(height * width, 0);
			auto buffer = Render::ImageBuffer<T>(pixels, width, height);

			// Gradient buffer for left to right (U coordinate)
			std::vector<T> u_coordinate_buffer(height * width);
			size_t cellSize = 200;
			for (size_t y = 0; y < height; ++y)
			{
				for (size_t x = 0; x < width; ++x)
				{
					// Determine the checkerboard color
					bool isBlack = (x / cellSize + y / cellSize) % 2 == 0;
					u_coordinate_buffer[y * width + x] = static_cast<T>(isBlack ? 255 : 0);
				}
			}
			const auto u_buffer = Render::ImageBuffer<T, true>(u_coordinate_buffer, width, height);
			warp.apply<T>(buffer, u_buffer, subdivided_mesh);

			{
				std::string filename_u = "C:/Users/emild/Desktop/linkedlayers/warp/warp_grid_u.jpg";
				std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename_u);
				if (!out)
					return;  // error
				OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::UINT8);
				out->open(filename_u, spec);
				out->write_image(OIIO::TypeDesc::UINT8, pixels.data());
				out->close();
			}
		}
		{
			std::vector<T> pixels(height * width, 0);
			auto buffer = Render::ImageBuffer<T>(pixels, width, height);

			// Gradient buffer for top to bottom (V coordinate)
			std::vector<T> v_coordinate_buffer(height * width);
			for (size_t y = 0; y < height; ++y) {
				for (size_t x = 0; x < width; ++x) {
					// Linear interpolation for gradient
					v_coordinate_buffer[y * width + x] = static_cast<T>((255 * y) / (height - 1));
				}
			}

			const auto v_buffer = Render::ImageBuffer<T, true>(v_coordinate_buffer, width, height);
			warp.apply<T>(buffer, v_buffer, subdivided_mesh);
			{
				std::string filename_v = "C:/Users/emild/Desktop/linkedlayers/warp/warp_grid_v.jpg";
				std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename_v);
				if (!out)
					return;  // error
				OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::UINT8);
				out->open(filename_v, spec);
				out->write_image(OIIO::TypeDesc::UINT8, pixels.data());
				out->close();
			}
		}

	}

};


PSAPI_NAMESPACE_END