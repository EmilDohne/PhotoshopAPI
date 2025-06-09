#pragma once

#include "Macros.h"

#include "Util/Logger.h"
#include "Util/Enum.h"
#include "Core/FileIO/BytesIO.h"
#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/ColorModeData.h"
#include "PhotoshopFile/ImageResources.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <span>
#include <unordered_map>
#include <array>
#include <memory>
#include <ranges>

PSAPI_NAMESPACE_BEGIN


namespace detail
{

	// This is a mini-reader only intended to extract the preview image from a psd/psb. It doesn't parse any of the sections
	// and simply 
	template <typename T = uint8_t>
	struct psd_psb_reader
	{
		using storage_type = std::unordered_map<Enum::ChannelIDInfo, std::unique_ptr<ImageChannel>, Enum::ChannelIDInfoHasher>;
		using data_type = std::unordered_map<Enum::ChannelIDInfo, std::vector<T>, Enum::ChannelIDInfoHasher>;


		psd_psb_reader(const std::vector<uint8_t>& file_data)
		{
			auto header = FileHeader::from_bytes(file_data);
			m_Header = header;
			auto offset = header.get_size();

			// Skip all of these sections
			offset += ColorModeData::get_size(std::span<const uint8_t>(file_data.begin() + offset, file_data.end()));
			offset += ImageResources::get_size(std::span<const uint8_t>(file_data.begin() + offset, file_data.end()));
			offset += LayerAndMaskInformation::get_size(
				std::span<const uint8_t>(file_data.begin() + offset, file_data.end()),
				header
			);

			// Now get the data, and compute the channels
			auto image_data_span = std::span<const uint8_t>(file_data.begin() + offset, file_data.end());
			auto compression = bytes_io::read_as_and_swap<uint16_t>(image_data_span, 0);
			// Exclude the compression marker from the span
			image_data_span = std::span<const uint8_t>(image_data_span.begin() + 2u, image_data_span.end());
			if (compression != 0 && compression != 1)
			{
				PSAPI_LOG_ERROR(
					"psd_psb_reader", "Currently our mini-reader only supports reading uncompressed or RLE compressed image data."
					" If this is a feature you would like to see, please raise an issue on the github page of the PhotoshopAPI."
					" Received compression codec : %i", static_cast<int>(compression)
				);
			}

			// If this is RLE compressed data we decompress this on the fly, swapping out image_data_span as if it was
			// uncompressed. Therefore we also need to have decompressed_rle_data outside of the if as otherwise it would
			// be destructed and the span would be invalid.
			std::vector<T> decompressed_rle_data;
			if (compression == 1)
			{
				auto stream = ByteStream(std::vector<uint8_t>(image_data_span.begin(), image_data_span.end()));
				decompressed_rle_data = std::vector<T>(static_cast<size_t>(header.m_NumChannels) * header.m_Width * header.m_Height);
				DecompressRLE(
					stream,
					std::span<T>(decompressed_rle_data.begin(), decompressed_rle_data.end()),
					0u,
					header,
					header.m_Width,
					header.m_Height * header.m_NumChannels,
					image_data_span.size()
				);

				image_data_span = std::span<uint8_t>(
					reinterpret_cast<uint8_t*>(decompressed_rle_data.data()),
					decompressed_rle_data.size() * sizeof(T)
				);
			}
			
			size_t size_per_channel_t = static_cast<size_t>(header.m_Width) * header.m_Height;
			size_t size_per_channel = size_per_channel_t * sizeof(T);
			size_t num_channels = image_data_span.size() / size_per_channel;
			if (image_data_span.size() % sizeof(T) != 0)
			{
				PSAPI_LOG_WARNING(
					"psd_psb_reader", "Possibly invalid data received for the global image data, size does not evenly"
					" divide across the computed number of channels, will truncate the rest of the data. Expected exactly"
					" %zu bytes but instead got %zu bytes", num_channels * size_per_channel, image_data_span.size()
				);
			}
			if (num_channels != 3 && num_channels != 4)
			{
				PSAPI_LOG_ERROR(
					"psd_psb_reader", "Invalid number of channels received for the global image data, only 3 or 4 channels"
					" are supported (RGB or RGBA), instead got %zu", num_channels
				);
			}

			std::array<Enum::ChannelIDInfo, 4> candidate_ids = {
				Enum::ChannelIDInfo{Enum::ChannelID::Red,   static_cast<int16_t>(0)},
				Enum::ChannelIDInfo{Enum::ChannelID::Green, static_cast<int16_t>(1)},
				Enum::ChannelIDInfo{Enum::ChannelID::Blue,  static_cast<int16_t>(2)},
				Enum::ChannelIDInfo{Enum::ChannelID::Alpha, static_cast<int16_t>(-1)},
			};
			
			for (auto idx : std::views::iota(static_cast<size_t>(0), num_channels))
			{
				auto id = candidate_ids[idx];
				auto base_offset = idx * size_per_channel;
				auto subspan = std::span<const uint8_t>(image_data_span.begin() + base_offset, size_per_channel);
				auto subspan_t = std::span<const T>(reinterpret_cast<const T*>(subspan.data()), size_per_channel_t);

				// Copy the data here as endianDecodeBEBinaryArray modifies in-place and requires non-const objects
				std::vector<T> data_vec(subspan_t.begin(), subspan_t.end());
				// RLE decompression will have already taken care of byteswapping the data.
				if (compression == 0)
				{
					endianDecodeBEArray<T>(std::span<T>(data_vec.begin(), data_vec.end()));
				}

				// Now we can compress and insert
				m_Storage[id] = std::make_unique<ImageChannel>(
					Enum::Compression::Raw,
					data_vec,
					id,
					static_cast<int32_t>(header.m_Width),
					static_cast<int32_t>(header.m_Height),
					static_cast<float>(header.m_Width) / 2,
					static_cast<float>(header.m_Height) / 2
				);
			}
		}

		static Enum::BitDepth bit_depth(const std::vector<uint8_t>& file_data)
		{
			auto header = FileHeader::from_bytes(file_data);
			return header.m_Depth;
		}

		FileHeader header()
		{
			return m_Header;
		}

		static storage_type extract_storage_type(psd_psb_reader&& reader)
		{
			return std::move(reader.m_Storage);
		}

	private:
		FileHeader m_Header;
		storage_type m_Storage;
	};

} // detail


PSAPI_NAMESPACE_END
