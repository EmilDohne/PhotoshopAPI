#pragma once

#include "Macros.h"
#include "Logger.h"
#include "EndianByteSwap.h"
#include "Struct/ByteStream.h"
#include "Profiling/Perf/Instrumentor.h"

#include "zlib-ng.h"

#include <algorithm>

PSAPI_NAMESPACE_BEGIN

namespace {

	/// Use zlib-ng to inflate the compressed input data to the expected output size
	inline std::vector<uint8_t> UnZip(const std::vector<uint8_t>& compressedData, const uint64_t decompressedSize)
	{
		PROFILE_FUNCTION();
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
std::vector<T> DecompressZIP(ByteStream& stream, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(compressedSize);
	stream.read(reinterpret_cast<char*>(compressedData.data()), compressedSize);

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
	PROFILE_FUNCTION();
	// Convert decompressed data to native endianness
	std::vector<T> bitShiftedData = endianDecodeBEBinaryArray<T>(decompressedData);

	if (bitShiftedData.size() != static_cast<uint64_t>(height) * static_cast<uint64_t>(width))
	{
		PSAPI_LOG_ERROR("RemovePredictionEncoding", "Endian Decoded data does not match expected size, expected %" PRIu64 ", got %i",
			static_cast<uint64_t>(height) * static_cast<uint64_t>(width),
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


// We need to specialize here as 32-bit files have their bytes interleaved (i.e. from 1234 1234 1234 1234 byte order to 1111 2222 3333 4444)
// And we need to do this de-interleaving separately. Thanks to both psd_sdk and psd-tools for having found this out
template <>
inline std::vector<float32_t> RemovePredictionEncoding(const std::vector<uint8_t>& decompressedData, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();
	std::vector<uint8_t> predictionDecodedData = decompressedData;

	// Perform prediction decoding per scanline of data in-place
	uint64_t index = 0;
	for (uint64_t y = 0; y < height; ++y)
	{
		++index;
		for (uint64_t x = 1; x < static_cast<uint64_t>(width) * 4u; ++x)
		{
			// Simple differencing: decode by adding the difference to the previous value
			uint8_t value = predictionDecodedData[index] + predictionDecodedData[index - 1];
			predictionDecodedData[index] = value;

			++index;
		}
	}

	// We now need to shuffle the byte order back in its normal place as photoshop stores the bytes deinterleaved for 32 bit types.
	// Imagine the following sequence of bytes, 1234 1234 1234 1234. If we would encode them literally it wouldnt give us much compression
	// which is why Photoshop deinterleaves them to 1111 2222 3333 4444 to get better compression. We now need to reverse this interleaving that is done row-by-row
	std::vector<float32_t> bitShiftedData;
	bitShiftedData.reserve(static_cast<uint64_t>(width) * height);

	// Create a temporary byte buffer that we reuse to interleave and then immediately endianDecode
	std::vector<uint8_t> buffer(sizeof(float32_t), 0);
	for (uint64_t y = 0; y < height; ++y)
	{
		for (uint64_t x = 0; x < width; ++x)
		{
			// By specifying these in reverse we already take care of endian decoding
			// Therefore no separate call is needed
			buffer[3] = predictionDecodedData[y * width * 4 + x];
			buffer[2] = predictionDecodedData[y * width * 4 + width + x];
			buffer[1] = predictionDecodedData[y * width * 4 + width * 2 + x];
			buffer[0] = predictionDecodedData[y * width * 4 + width * 3 + x];

			// Reinterpret the byte array to be a float32_t. This can be considered safe as we limit size of buffer to 0;
			float32_t tmp = reinterpret_cast<float32_t&>(buffer[0]);
			bitShiftedData.push_back(tmp);
		}
	}

	if (bitShiftedData.size() != static_cast<uint64_t>(height) * static_cast<uint64_t>(width))
	{
		PSAPI_LOG_ERROR("RemovePredictionEncoding", "Endian Decoded data does not match expected size, expected %" PRIu64 ", got %i",
			static_cast<uint64_t>(height) * static_cast<uint64_t>(width),
			bitShiftedData.size())
	}
	
	return bitShiftedData;
}



template <typename T>
std::vector<T> DecompressZIPPrediction(ByteStream& stream, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(compressedSize);
	stream.read(reinterpret_cast<char*>(compressedData.data()), compressedSize);

	// Decompress using Inflate ZIP
	std::vector<uint8_t> decompressedData = UnZip(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * sizeof(T));

	// Remove the prediction encoding from the data as well as converting to native endianness
	std::vector<T> outData = RemovePredictionEncoding<T>(decompressedData, width, height);

	return outData;
}

PSAPI_NAMESPACE_END