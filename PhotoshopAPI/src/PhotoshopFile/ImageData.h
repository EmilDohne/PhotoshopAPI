#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "FileIO/Write.h"

#include "blosc2.h"


PSAPI_NAMESPACE_BEGIN


namespace ImageDataImpl
{
	template <typename T>
	void writeCompressedData(File& document, const FileHeader& header, std::vector<T>&& uncompressedData)
	{
		if (header.m_Version == Enum::Version::Psd)
		{
			std::vector<uint16_t> scanlineSizes;
			std::vector<uint8_t> compressedData = CompressRLEImageDataPsd(uncompressedData, header, header.m_Width, header.m_Height, scanlineSizes);
			// First write all the scanline sizes, then the compressed data
			for (int i = 0; i < header.m_NumChannels; ++i)
			{
				WriteBinaryArray<uint16_t>(document, scanlineSizes);
			}
			for (int i = 0; i < header.m_NumChannels; ++i)
			{
				WriteBinaryArray<uint8_t>(document, compressedData);
			}
		}
		else
		{
			std::vector<uint32_t> scanlineSizes;
			std::vector<uint8_t> compressedData = CompressRLEImageDataPsb(uncompressedData, header, header.m_Width, header.m_Height, scanlineSizes);
			// First write all the scanline sizes, then the compressed data
			for (int i = 0; i < header.m_NumChannels; ++i)
			{
				WriteBinaryArray<uint32_t>(document, scanlineSizes);
			}
			for (int i = 0; i < header.m_NumChannels; ++i)
			{
				WriteBinaryArray<uint8_t>(document, compressedData);
			}
		}
	}
}


struct ImageData : public FileSection
{
	
	inline uint64_t calculateSize(std::shared_ptr<FileHeader> header /* = nullptr */) const override { return 0; };

	// Write out an empty image data section from the number of channels. This section is unfortunately required
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
			ImageDataImpl::writeCompressedData(document, header, std::move(emptyData));
		}
		else if (header.m_Depth == Enum::BitDepth::BD_16)
		{
			std::vector<uint16_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			ImageDataImpl::writeCompressedData(document, header, std::move(emptyData));
		}
		else if (header.m_Depth == Enum::BitDepth::BD_32)
		{
			std::vector<float32_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			ImageDataImpl::writeCompressedData(document, header, std::move(emptyData));
		}
	}
};




PSAPI_NAMESPACE_END