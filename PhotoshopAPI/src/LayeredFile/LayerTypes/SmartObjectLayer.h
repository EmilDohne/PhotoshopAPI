#pragma once

#include "Macros.h"
#include "Layer.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/Warp/SmartObjectWarp.h"
#include "Core/Render/Render.h"

#include <fstream>
#include <string>

#include "fmt/core.h"
#include <OpenImageIO/imageio.h>



PSAPI_NAMESPACE_BEGIN


namespace SmartObject
{
	struct Transform
	{

	};

	struct Warp
	{

	};
}

/// This struct holds no data, we just use it to identify its type.
/// We could hold references here 
template <typename T>
struct SmartObjectLayer : Layer<T>
{


	SmartObjectWarp m_Warp;


	SmartObjectLayer() = default;

	SmartObjectLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo) 
		: Layer<T>(layerRecord, channelImageData, header)
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


		//if (m_Warp.m_WarpPoints.size() > 0)
		//{
		//	for (int i = 0; i < layerRecord.m_ChannelCount; ++i)
		//	{
		//		auto height = m_Warp.m_Bounds[2] - m_Warp.m_Bounds[0];
		//		auto width = m_Warp.m_Bounds[3] - m_Warp.m_Bounds[1];

		//		auto& channelInfo = layerRecord.m_ChannelInformation[i];

		//		// We already extract masks ahead of time and skip them here to avoid raising warnings
		//		if (channelInfo.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask) continue;

		//		auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
		//		// Pointers might have already been released previously
		//		if (!channelPtr) continue;

		//		auto pixels = channelPtr->extractData<T>();
		//		auto buffer = Render::ImageBuffer<T, true>(pixels, channelPtr->getWidth(), channelPtr->getHeight());

		//		std::vector<T> out_pixels(height * width, 0);
		//		auto out_buffer = Render::ImageBuffer<T>(out_pixels, width , height);

		//		m_Warp.apply_warp<T>(out_buffer, buffer, 100);


		//		auto filename = fmt::format("C:/Users/emild/Desktop/linkedlayers/warp_{}_{}.jpg", Layer<T>::m_LayerName, i);
		//		std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);
		//		if (!out)
		//			return;  // error
		//		OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::UINT8);
		//		out->open(filename, spec);
		//		out->write_image(OIIO::TypeDesc::UINT8, out_pixels.data());
		//		out->close();
		//	}
		//	PSAPI_LOG_ERROR("A", "A");
		//}
	}


	std::vector<T> getImageData()
	{
		return {};
	}


private:

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

	void decodePlacedLayerData(const std::shared_ptr<PlacedLayerDataTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		const auto& descriptor = local->m_Descriptor;

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
		const auto& transform = descriptor.at("Trnf");
		const auto& non_affine_transform = descriptor.at("nonAffineTransform");

		// The warp struct is present on all descriptors, if it is however a warp with a non-standard
		// number of subdivisions (i.e. not 4x4) the warp struct will be empty and instead we will be dealing with a quilt warp
		const auto& warp = descriptor.at<Descriptors::Descriptor>("warp");
		SmartObjectWarp warpStruct;
		if (descriptor.contains("quiltWarp"))
		{
			warpStruct = SmartObjectWarp::deserialize(descriptor.at<Descriptors::Descriptor>("quiltWarp"), SmartObjectWarp::WarpType::quilt);
		}
		else
		{
			warpStruct = SmartObjectWarp::deserialize(warp, SmartObjectWarp::WarpType::normal);
			//renderMesh(warpStruct, fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/warp_grid{}.jpg", name));
		}
		m_Warp = warpStruct;
		const auto& size = descriptor.at("Sz  ");		// The spaces are not a mistake
		const auto& resolution = descriptor.at("Rslt");	// In DPI

		const auto& _comp = descriptor.at("comp");
		const auto& _comp_info = descriptor.at("compInfo");
	}


	void decodePlacedLayer(const std::shared_ptr<PlacedLayerTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{

	}

	void renderMesh(const SmartObjectWarp& warp, std::string filename)
	{
		auto height = warp.m_Bounds[2] - warp.m_Bounds[0];
		auto width = warp.m_Bounds[3] - warp.m_Bounds[1];
		{
			std::vector<T> pixels(height * width, 0);
			auto buffer = Render::ImageBuffer<T>(pixels, width, height);
			auto surface = warp.surface();
			auto subdivided_mesh = surface.mesh(100, 100);
			Render::render_mesh<T>(buffer, subdivided_mesh);

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
			for (size_t y = 0; y < height; ++y) {
				for (size_t x = 0; x < width; ++x) {
					// Linear interpolation for gradient
					u_coordinate_buffer[y * width + x] = static_cast<T>((255 * x) / (width - 1));
				}
			}
			const auto u_buffer = Render::ImageBuffer<T, true>(u_coordinate_buffer, width, height);
			warp.apply<T>(buffer, u_buffer, 100);

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
			warp.apply<T>(buffer, v_buffer, 100);
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
		PSAPI_LOG_ERROR("A", "A");
		std::cout << 1 << std::endl;

	}

	std::string m_UUID;
	std::string m_Filename;

	std::vector<uint8_t> m_RawBytes;
};


PSAPI_NAMESPACE_END