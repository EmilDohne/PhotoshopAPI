#pragma once

#include "RLE.h"
#include "ZIP.h"
#include "Macros.h"
#include "Read.h"
#include "Enum.h"
#include "Struct/ByteStream.h"


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
template <typename T>
inline std::vector<T> DecompressData(ByteStream& stream, const Enum::Compression& compression, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	switch (compression)
	{
	case Enum::Compression::Raw:
		return ReadBinaryArray<T>(stream, compressedSize);
	case Enum::Compression::Rle:
		return DecompressRLE<T>(stream, header, width, height, compressedSize);
	case Enum::Compression::Zip:
		return DecompressZIP<T>(stream, header, width, height, compressedSize);
	case Enum::Compression::ZipPrediction:
		return DecompressZIPPrediction<T>(stream, header, width, height, compressedSize);
	default:
		return ReadBinaryArray<T>(stream, compressedSize);
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