#pragma once

#include "../../Macros.h"
#include "../Logger.h"
#include "../EndianByteSwap.h"

#include "zlib-ng.h"


PSAPI_NAMESPACE_BEGIN

namespace {
	inline std::vector<uint8_t> UnZip(const std::vector<uint8_t>& compressedData, const uint64_t decompressedSize)
	{
		// Inflate the data
		zng_stream stream{};
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = compressedData.size();
		stream.next_in = compressedData.data();

		if (zng_inflateInit(&stream) != Z_OK)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate initialization failed")
		}

		// We do actually want to initialize the data here
		std::vector<uint8_t> decompressedData(decompressedSize);

		stream.avail_out = decompressedData.size();
		stream.next_out = decompressedData.data();

		if (zng_inflate(&stream, Z_FINISH) != Z_STREAM_END)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate decompression failed")
		}

		if (zng_inflateEnd(&stream) != Z_OK)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate cleanup failed")
		}

		return decompressedData;
	}
}


template <typename T>
std::vector<T> DecompressZIP(File& document, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(compressedSize);
	document.read(reinterpret_cast<char*>(compressedData.data()), compressedSize);

	// Decompress using Inflate ZIP
	std::vector<uint8_t> decompressedData = UnZip(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * sizeof(T));

	// Convert decompressed data to native endianness
	std::vector<T> bitShiftedData = endianDecodeBEBinaryArray(decompressedData);

	return bitShiftedData;
}


// Reverse the prediction encoding after having decompressed the zip as well as converting from BE to native
// we do this step here as the 32 bit specialization needs 
template <typename T>
std::vector<T> RemovePredictionEncoding(const std::vector<uint8_t>& decompressedData, const uint32_t width, const uint32_t height)
{
	// Convert decompressed data to native endianness
	std::vector<T> bitShiftedData = endianDecodeBEBinaryArray<T>(decompressedData);

	// Perform prediction decoding
	for (uint64_t i = 1; i < bitShiftedData.size(); i++)
	{
		// Simple differencing: decode by adding the difference to the previous value
		bitShiftedData[i] = bitShiftedData[i - 1];
	}

	return bitShiftedData;
}

template <typename T>
std::vector<T> DecompressZIPPrediction(File& document, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(compressedSize);
	document.read(reinterpret_cast<char*>(compressedData.data()), compressedSize);

	// Decompress using Inflate ZIP
	std::vector<uint8_t> decompressedData = UnZip(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * sizeof(T));

	// Remove the prediction encoding from the data
	// as well as converting to native endianness
	std::vector<T> outData = RemovePredictionEncoding(decompressedData, width, height);

	return outData;
}

PSAPI_NAMESPACE_END