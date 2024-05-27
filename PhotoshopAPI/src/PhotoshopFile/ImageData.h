#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"
#include "Core/FileIO/Write.h"
#include "Core/Compression/Compress_RLE.h"

#include "blosc2.h"


PSAPI_NAMESPACE_BEGIN


namespace ImageDataImpl
{
	template <typename T>
	void writeCompressedData(File& document, const FileHeader& header, const uint16_t numChannels, std::vector<T>&& uncompressedData)
	{
		if (header.m_Version == Enum::Version::Psd)
		{
			std::vector<uint16_t> scanlineSizes;
			std::vector<uint8_t> compressedData = CompressRLEImageDataPsd(uncompressedData, header, header.m_Width, header.m_Height, scanlineSizes);
			// First write all the scanline sizes, then the compressed data
			for (int i = 0; i < numChannels; ++i)
			{
				// we must copy here as we otherwise byteswap multiple times
				auto data = scanlineSizes;
				WriteBinaryArray<uint16_t>(document, std::move(data));
			}
			for (int i = 0; i < numChannels; ++i)
			{
				// we must copy here as we otherwise byteswap multiple times
				auto data = compressedData;
				WriteBinaryArray<uint8_t>(document, std::move(data));
			}
		}
		else
		{
			std::vector<uint32_t> scanlineSizes;
			std::vector<uint8_t> compressedData = CompressRLEImageDataPsb(uncompressedData, header, header.m_Width, header.m_Height, scanlineSizes);
			// First write all the scanline sizes, then the compressed data
			for (int i = 0; i < numChannels; ++i)
			{
				// we must copy here as we otherwise byteswap multiple times
				auto data = scanlineSizes;
				WriteBinaryArray<uint32_t>(document, std::move(data));
			}
			for (int i = 0; i < numChannels; ++i)
			{
				// we must copy here as we otherwise byteswap multiple times
				auto data = compressedData;
				WriteBinaryArray<uint8_t>(document, std::move(data));
			}
		}
	}

	template <typename T>
	void writeRawData(File& document, const FileHeader& header, std::vector<T>&& uncompressedData)
	{
		WriteBinaryArray<T>(document, uncompressedData);
	}
}


/// \brief This section is for interoperability with different software such as lightroom and holds a composite of all the layers
///
/// When writing out data we fill it with empty pixels using Rle compression, this is due to Photoshop unfortunately requiring
/// it to be present. Due to this compression step we can usually save lots of data over what Photoshop writes out
struct ImageData : public FileSection
{

	inline uint64_t calculateSize(std::shared_ptr<FileHeader> header /* = nullptr */) const override { return 0; };

	/// Write out an empty image data section from the number of channels. This section is unfortunately required
	inline void write(File& document, const FileHeader& header)
	{
		// Compression marker, we default to RLE compression to reduce the size significantly. The way in which the scanlines are stored
		// is slightly different though. All the channels store their scanline sizes at the start of the ImageData section rather than
		// at the start of each channel
		WriteBinaryData<uint16_t>(document, 1u);
		// Write out empty data for all of the channels
		if (header.m_Depth == Enum::BitDepth::BD_8)
		{
			std::vector<uint8_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			ImageDataImpl::writeCompressedData(document, header, m_NumChannels, std::move(emptyData));
		}
		else if (header.m_Depth == Enum::BitDepth::BD_16)
		{
			std::vector<uint16_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			ImageDataImpl::writeCompressedData(document, header, m_NumChannels, std::move(emptyData));
		}
		else if (header.m_Depth == Enum::BitDepth::BD_32)
		{
			std::vector<float32_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			ImageDataImpl::writeCompressedData(document, header, m_NumChannels, std::move(emptyData));
		}
	}

	ImageData() = default;

	/// Initialize the ImageData with a given number of channels to write out. We do this rather than deducting
	/// from the header as the header counts alpha channels while this does not!
	ImageData(uint16_t numChannels) : m_NumChannels(numChannels) {};

private:
	uint16_t m_NumChannels = 0u;
};




PSAPI_NAMESPACE_END