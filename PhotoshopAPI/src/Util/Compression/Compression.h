#pragma once

#include "RLE.h"
#include "ZIP.h"
#include "Macros.h"
#include "FileIO/Read.h"
#include "Enum.h"
#include "Struct/ByteStream.h"
#include "Profiling/Perf/Instrumentor.h"


#include <vector>


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
inline std::vector<T> DecompressData(ByteStream& stream, uint64_t offset, const Enum::Compression& compression, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PROFILE_FUNCTION();
	switch (compression)
	{
	case Enum::Compression::Raw:
		return ReadBinaryArray<T>(stream, offset, compressedSize);
	case Enum::Compression::Rle:
		return DecompressRLE<T>(stream, offset, header, width, height, compressedSize);
	case Enum::Compression::Zip:
		return DecompressZIP<T>(stream, offset, width, height, compressedSize);
	case Enum::Compression::ZipPrediction:
		return DecompressZIPPrediction<T>(stream, offset, width, height, compressedSize);
	default:
		return ReadBinaryArray<T>(stream, offset, compressedSize);
	}
}


// Compress an input datastream using the appropriate compression algorithm while encoding to BE order
// RLE compression will encode the scanline sizes at the start of the data as well. This would equals to 
// 2/4 * height bytes of additional data (2 bytes for PSD and 4 for PSB)
template <typename T>
inline std::vector<uint8_t> CompressData(std::vector<T>& uncompressedIn, const Enum::Compression& compression, const FileHeader& header, const uint32_t width, const uint32_t height)
{
	// Perform the endian decoding in-place first as this step needs to happen before compressing either way
	endianDecodeBEArray<T>(uncompressedIn);
	if (compression == Enum::Compression::Raw)
	{
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
		return CompressZIP(uncompressedIn, width, height);
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