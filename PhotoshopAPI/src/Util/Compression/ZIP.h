#pragma once

#include "../../Macros.h"
#include "../Logger.h"
#include "../EndianByteSwap.h"

#include "zlib-ng.h"


PSAPI_NAMESPACE_BEGIN

namespace {

	/// Use zlib-ng to inflate the compressed input data to the expected output size
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

		// The size being fixed to exactly what we expect the decompressed data to be sized
		// makes sure we catch any errors in our calculations of decompressedSize or in the input
		// byte stream
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
	std::vector<T> bitShiftedData = endianDecodeBEBinaryArray<T>(decompressedData);

	return bitShiftedData;
}


// Reverse the prediction encoding after having decompressed the zip compressed byte stream as well as converting from BE to native
template <typename T>
std::vector<T> RemovePredictionEncoding(const std::vector<uint8_t>& decompressedData, const uint32_t width, const uint32_t height)
{
	// Convert decompressed data to native endianness
	std::vector<T> bitShiftedData = endianDecodeBEBinaryArray<T>(decompressedData);

	if (bitShiftedData.size() != static_cast<uint64_t>(height) * static_cast<uint64_t>(width))
	{
		PSAPI_LOG_ERROR("RemovePredictionEncoding", "Endian Decoded data does not match expected size, expected %" PRIu64 ", got %i",
			static_cast<uint64_t>(height) * static_cast<uint64_t>(width) * sizeof(T),
			bitShiftedData.size())
	}

	// Perform prediction decoding per scanline of data in-place
	for (uint64_t y = 0; y < height; ++y)
	{
		for (uint64_t x = 1; x < width; ++x)
		{
			// Simple differencing: decode by adding the difference to the previous value
			bitShiftedData[width * y + x] += bitShiftedData[width * y + x - 1];
		}
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

	// Remove the prediction encoding from the data as well as converting to native endianness
	std::vector<T> outData = RemovePredictionEncoding<T>(decompressedData, width, height);

	return outData;
}

PSAPI_NAMESPACE_END