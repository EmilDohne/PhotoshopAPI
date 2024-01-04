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
// The scanlineSizes parameter is only relevant for the RLE compression codec and can safely be left
// at its default in all other cases
template <typename T>
inline std::vector<T> CompressData(std::vector<T>& uncompressedIn, const Enum::Compression& compression, const uint32_t width, const uint32_t height, std::shared_ptr<std::vector<uint32_t>> scanlineSizes = nullptr)
{
	// Perform the endian decoding in-place first as this step needs to happen before compressing either way
	endianDecodeBEArray<T>(uncompressedIn);
	switch (compression)
	{
	case Enum::Compression::Raw:
		return std::move(uncompressedIn);
	case Enum::Compression::Rle:
		if (!scanlineSizes)
			PSAPI_LOG_ERROR("Compression", "RLE Compression requires the scanlineSizes parameter to be passed")
		if (scanlineSizes->size() != 0)
			PSAPI_LOG_ERROR("Compression", "scanlineSizes parameter must be of size zero and will be filled on execution")
		return CompressRLE(uncompressedIn, width, height, scanlineSizes);
	case Enum::Compression::Zip:
		return CompressZIP(uncompressedIn, width, height);
	case Enum::Compression::ZipPrediction:
		return CompressZIPPrediction(uncompressedIn, width, height);
	}
}



PSAPI_NAMESPACE_END