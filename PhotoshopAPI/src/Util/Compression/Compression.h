#pragma once

#include "RLE.h"
#include "ZIP.h"
#include "../../Macros.h"
#include "../Read.h"
#include "../../Util/Enum.h"


#include <vector>


PSAPI_NAMESPACE_BEGIN


// Read and decompress an input datastream using the appropriate compression algorithm
// Call this as well if your input is using RAW compression as it will handle this case
// Endian-conversion is handled by this function as well
template <typename T>
inline std::vector<T> DecompressData(File& document, const Enum::Compression& compression, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	switch (compression)
	{
	case Enum::Compression::Raw:
		return ReadBinaryArray<T>(document, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));
	case Enum::Compression::Rle:
		return DecompressRLE(document, header, width, height);
	case Enum::Compression::Zip:
		return DecompressZIP(document, header, width, height, compressedSize)
	case Enum::Compression::ZipPrediction:
		return DecompressZIPPrediction(document, header, width, height, compressedSize)
	default:
		return ReadBinaryArray<T>(document, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));
	}
}


// Compress an input datastream using the appropriate compression algorithm
// Call this as well if your input is using RAW compression as it will handle this case
/*template <typename T>
inline std::vector<T> CompressData(std::vector<T>& uncompressedIn, const Enum::Compression& compression)
{
	switch (compression)
	{
	case Enum::Compression::Raw:
		return std::move(uncompressedIn);
	case Enum::Compression::Rle:
		return 
	}
}*/



PSAPI_NAMESPACE_END