#pragma once

#include "Decompress_RLE.h"
#include "Compress_RLE.h"

#include "Decompress_ZIP.h"
#include "Compress_ZIP.h"

#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Util/Enum.h"
#include "Core/Struct/ByteStream.h"
#include "Util/Profiling/Perf/Instrumentor.h"


#include <type_traits>
#include <vector>

#include <cstring>


PSAPI_NAMESPACE_BEGIN

/// Read and decompress a given number of bytes based on the compression algorithm given, after which
/// the data is endian decoded into native encoding and returned either in scanline order
/// 
/// RRR...
/// GGG...
/// BBB...
/// 
/// or as a singular image channel depending on where the call was made from
/// ---------------------------------------------------------------------------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void DecompressData(ByteStream& stream, std::span<T> buffer, uint64_t offset, const Enum::Compression& compression, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PSAPI_PROFILE_FUNCTION();
	switch (compression)
	{
	case Enum::Compression::Raw:
		ReadBinaryArray<T>(stream, buffer, offset, compressedSize);
		break;
	case Enum::Compression::Rle:
		DecompressRLE<T>(stream, buffer, offset, header, width, height, compressedSize);
		break;
	case Enum::Compression::Zip:
		DecompressZIP<T>(stream, buffer, offset, width, height, compressedSize);
		break;
	case Enum::Compression::ZipPrediction:
		DecompressZIPPrediction<T>(stream, buffer, offset, width, height, compressedSize);
		break;
	default:
		ReadBinaryArray<T>(stream, buffer, offset, compressedSize);
		break;
	}
}


// Compress an input datastream using the appropriate compression algorithm while encoding to BE order
// RLE compression will encode the scanline sizes at the start of the data as well. This would equals to 
// 2/4 * height bytes of additional data (2 bytes for PSD and 4 for PSB)
template <typename T>
inline std::vector<uint8_t> CompressData(std::span<T> uncompressedIn, std::span<uint8_t> buffer, libdeflate_compressor* compressor, const Enum::Compression& compression, const FileHeader& header, const uint32_t width, const uint32_t height)
{
	if (compression == Enum::Compression::Raw)
	{
		if (uncompressedIn.size() == 0)
		{
			return {};
		}
		endianEncodeBEArray(uncompressedIn);
		std::vector<uint8_t> data(uncompressedIn.size() * sizeof(T));
		std::memcpy(reinterpret_cast<void*>(data.data()), reinterpret_cast<void*>(uncompressedIn.data()), data.size());
		return data;
	}
	else if (compression == Enum::Compression::Rle)
	{
		return CompressRLE(uncompressedIn, buffer, header, width, height);
	}
	else if (compression == Enum::Compression::Zip)
	{
		return CompressZIP(uncompressedIn, buffer, compressor);
	}
	else if (compression == Enum::Compression::ZipPrediction)
	{
		return CompressZIPPrediction(uncompressedIn, buffer, compressor, width, height);
	}
	else
	{
		return(std::vector<uint8_t>{});
	}
}


// Compress an input datastream using the appropriate compression algorithm while encoding to BE order
// RLE compression will encode the scanline sizes at the start of the data as well. This would equals to 
// 2/4 * height bytes of additional data (2 bytes for PSD and 4 for PSB)
template <typename T>
inline std::vector<uint8_t> CompressData(std::vector<T>& uncompressedIn, const Enum::Compression& compression, const FileHeader& header, const uint32_t width, const uint32_t height)
{
	if (compression == Enum::Compression::Raw)
	{
		endianEncodeBEArray(std::span<T>(uncompressedIn));
		std::vector<uint8_t> data(uncompressedIn.size() * sizeof(T));
		std::memcpy(reinterpret_cast<void*>(data.data()), reinterpret_cast<void*>(uncompressedIn.data()), data.size());
		return data;
	}
	else if (compression == Enum::Compression::Rle)
	{
		return CompressRLE(uncompressedIn, header, width, height);
	}
	else if (compression == Enum::Compression::Zip)
	{
		return CompressZIP(uncompressedIn);
	}
	else if (compression == Enum::Compression::ZipPrediction)
	{
		return CompressZIPPrediction(uncompressedIn, width, height);
	}
	else
	{
		return(std::vector<uint8_t>{});
	}
}



PSAPI_NAMESPACE_END