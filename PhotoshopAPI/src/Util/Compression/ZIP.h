#pragma once

#include "Macros.h"
#include "Logger.h"
#include "Endian/EndianByteSwap.h"
#include "Struct/ByteStream.h"
#include "Profiling/Perf/Instrumentor.h"

#include "zlib-ng.h"

#include <algorithm>
#include <execution>
#include <vector>
#include <span>
#include <tuple>

PSAPI_NAMESPACE_BEGIN

namespace {

	// Use zlib-ng to inflate the compressed input data to the expected output size
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<T> UnZip(const std::vector<uint8_t>& compressedData, const uint64_t decompressedSize)
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
		std::vector<T> decompressedData(decompressedSize);
		stream.avail_out = decompressedData.size() * sizeof(T);
		stream.next_out = reinterpret_cast<uint8_t*>(decompressedData.data());


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

	// Use zlib-ng to deflate the uncompressed input data
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<uint8_t> Zip(const std::vector<T>& uncompressedData)
	{
		std::vector<uint8_t> compressedData;
		std::vector<uint8_t> buffer(128 * 1024);

		PROFILE_FUNCTION();
		zng_stream stream{};
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = uncompressedData.size();
		stream.next_in = uncompressedData.data();
		stream.next_out = reinterpret_cast<uint8_t*>(buffer.data());
		stream.avail_out = buffer.size();

		if (zng_deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK)
		{
			PSAPI_LOG_ERROR("Zip", "Deflate init failed")
		}

		// Continuously iterate the input stream and compress in chunks into our buffer after which
		// we copy the buffer to the compressedData vec
		while (stream.avail_in != 0)
		{
			if(zng_deflate(&stream, Z_NO_FLUSH) != Z_OK);
			{
				PSAPI_LOG_ERROR("Zip", "Unable to call deflate with Z_NO_FLUSH on the input data")
			}
			if (stream.avail_out == 0)
			{
				compressedData.insert(compressedData.end(), std::begin(buffer), std::end(buffer));
				stream.next_out = reinterpret_cast<uint8_t*>(buffer.data());
				stream.avail_out = buffer.size();
			}
		}

		// Deal with the remaining chunks
		int deflate_res = Z_OK;
		while (deflate_res == Z_OK)
		{
			if (stream.avail_out == 0)
			{
				compressedData.insert(compressedData.end(), std::begin(buffer), std::end(buffer));
				stream.next_out = reinterpret_cast<uint8_t*>(buffer.data());
				stream.avail_out = buffer.size();
			}
			deflate_res = zng_deflate(&stream, Z_FINISH);
		}

		// Copy over the partial chunk
		size_t compressedSize = buffer.size() - stream.avail_out;
		compressedData.insert(compressedData.end(), std::begin(buffer), std::begin(buffer) + compressedSize);

		if (zng_deflateEnd(&stream) != Z_OK)
		{
			PSAPI_LOG_ERROR("Zip", "Deflate cleanup failed")
		}

		return compressedData;
	}

	// Creates two vectors that can be used as iterators for an image by height or width. 
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	inline std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> createImageIterators(const uint32_t width, const uint32_t height)
	{
		std::vector<uint32_t> horizontalIter;
		std::vector<uint32_t> verticalIter;
		horizontalIter.resize(width);
		verticalIter.resize(height);

		for (uint32_t i = 0; i < width; ++i)
		{
			horizontalIter[i] = i;
		}
		for (uint32_t i = 0; i < height; ++i)
		{
			verticalIter[i] = i;
		};

		return std::make_tuple(horizontalIter, verticalIter);
	}

	// Creates one vector that can be used as iterators for an image by height.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	inline std::vector<uint32_t> createVerticalImageIterator(const uint32_t height)
	{
		std::vector<uint32_t> verticalIter;
		verticalIter.resize(height);
		for (uint32_t i = 0; i < height; ++i)
		{
			verticalIter[i] = i;
		};

		return verticalIter;
	}
}


// Reverse the prediction encoding after having decompressed the zip compressed byte stream as well as converting from BE to native
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> RemovePredictionEncoding(std::vector<T> decompressedData, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();
	// Convert decompressed data to native endianness in-place
	endianDecodeBEArray<T>(decompressedData);

	std::vector<uint32_t> verticalIter = createVerticalImageIterator(height);

	// Perform prediction decoding per scanline of data in-place
	std::for_each(std::execution::par_unseq, verticalIter.begin(), verticalIter.end(),
		[&](uint32_t y) 
		{
			for (uint64_t x = 1; x < width; ++x)
			{
				// Simple differencing: decode by adding the difference to the previous value
				decompressedData[static_cast<uint64_t>(width) * y + x] += decompressedData[static_cast<uint64_t>(width) * y + x - 1];
			}
		});

	return std::move(decompressedData);
}


// We need to specialize here as 32-bit files have their bytes de-interleaved (i.e. from 1234 1234 1234 1234 byte order to 1111 2222 3333 4444)
// And we need to do this de-interleaving separately. Thanks to both psd_sdk and psd-tools for having found this out
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
inline std::vector<float32_t> RemovePredictionEncoding(std::vector<float32_t> decompressedData, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	// We simply alias the vector to a span of uint8_t to perform our bytewise operations which modify decompressedData in place
	const uint64_t dataSize = static_cast<uint64_t>(width) * height * sizeof(float32_t);
	std::span<uint8_t> predictionDecodedData(reinterpret_cast<uint8_t*>(decompressedData.data()), dataSize);

	// Perform prediction decoding per scanline of data in-place
	uint64_t index = 0;
	for (uint64_t y = 0; y < height; ++y)
	{
		++index;
		for (uint64_t x = 1; x < static_cast<uint64_t>(width) * sizeof(float32_t); ++x)
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
			// Therefore no separate call is needed. This should work platform independant on LE and BE platforms
			// TODO this can be done with AVX2 as well but we need to make sure that we have a buffer
			// per interleaved 4-scanline pair
			buffer[3] = predictionDecodedData[y * width * sizeof(float32_t) + x];
			buffer[2] = predictionDecodedData[y * width * sizeof(float32_t) + static_cast<uint64_t>(width) + x];
			buffer[1] = predictionDecodedData[y * width * sizeof(float32_t) + static_cast<uint64_t>(width) * 2 + x];
			buffer[0] = predictionDecodedData[y * width * sizeof(float32_t) + static_cast<uint64_t>(width) * 3 + x];

			// Reinterpret the byte array to be a float32_t. This can be considered safe as we limit size of buffer to 4;
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


// Prediction encode the data per scanline while also big endian converting it
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> PredictionEncode(std::vector<T> data, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();
	// Here we must first prediction encode per scanline and then do the big endian conversion afterwards

	std::vector<uint32_t> verticalIter = createVerticalImageIterator(height);

	// This operation unfortunately cannot be done in-place but we can use a temporary buffer to avoid unnecessary memory overhead
	std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
		[&](uint32_t y)
		{
			std::vector<T> buffer(width);
			// We must set the initial value manually as the prediction encoding goes from the first value
			buffer[0] = data[static_cast<uint64_t>(width) * y];
			for (uint32_t x = 1; x < width; ++x)
			{
				// Generate the difference between the current value and the next one and store it
				buffer[x] = data[static_cast<uint64_t>(width) * y + x] - data[static_cast<uint64_t>(width) * y + x - 1];
			}
			std::memcpy(reinterpret_cast<void*>(data.data() + static_cast<uint64_t>(width) * y), reinterpret_cast<void*>(buffer.data()), width);
		});

	endianEncodeBEArray<T>(data);

	return std::move(data);
}


// We need to specialize here as 32-bit files have their bytes de-interleaved (i.e. from 1234 1234 1234 1234 byte order to 1111 2222 3333 4444)
// And we need to do this de-interleaving separately. Thanks to both psd_sdk and psd-tools for having found this out
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
inline std::vector<float32_t> PredictionEncode(std::vector<float32_t> data, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	std::span<uint8_t> byteDataView(reinterpret_cast<uint8_t*>(data.data()), data.size() * sizeof(float32_t));

	// First de-interleave the data to planar byte order, i.e. going from 1234 1234 1234 1234 to 1111 2222 3333 4444
	// We essentially split each scanline into 4 equal parts each holding the first, second, third and fourth of the original bytes
	std::vector<uint32_t> verticalIter = createVerticalImageIterator(height);
	std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
		[&](uint32_t y)
		{
			// this buffer will hold the interleaved bytes of each scanline
			std::vector<uint8_t> buffer(width * sizeof(float32_t));
			std::span<uint8_t> firstRowView(buffer.data(), width);
			std::span<uint8_t> secondRowView(buffer.data() + width, width);
			std::span<uint8_t> thirdRowView(buffer.data() + width * 2, width);
			std::span<uint8_t> fourthRowView(buffer.data() + width * 3, width);

			for (uint64_t x = 0; x < width; ++x)
			{
				// While we loop over the items as if they were float32_t, we actually access 4 bytes at once allowing us to 
				// simplify the loop. We first get the coordinate at the start of the row and then generate 
				// the individual byte offsets
				uint64_t rowCoord = static_cast<uint64_t>(y) * width * sizeof(float32_t);
				firstRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t)];		// Byte 0 
				secondRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 1];	// Byte 1
				thirdRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 2];	// Byte 2
				fourthRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 3];	// Byte 3
			}

			// Copy the row back over, no need to lock here as the ranges are not overlapping
			std::memcpy(reinterpret_cast<void*>(data.data() + y * width), reinterpret_cast<void*>(buffer.data()), buffer.size());
		});

	// Perform the prediction encoding of the data
	std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
		[&](uint32_t y)
		{
			std::vector<float32_t> buffer(width);
			// We must set the initial value manually as the prediction encoding goes from the first value
			buffer[0] = data[static_cast<uint64_t>(width) * y];
			for (uint32_t x = 1; x < width; ++x)
			{
				// Generate the difference between the current value and the next one and store it
				buffer[x] = data[static_cast<uint64_t>(width) * y + x] - data[static_cast<uint64_t>(width) * y + x - 1];
			}
			std::memcpy(reinterpret_cast<void*>(data.data() + static_cast<uint64_t>(width) * y), reinterpret_cast<void*>(buffer.data()), width);
		});

	// Finally, endian encode and return the data
	endianEncodeBEArray(data);
	return std::move(data);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIP(ByteStream& stream, uint64_t offset, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(compressedSize);
	stream.setOffsetAndRead(reinterpret_cast<char*>(compressedData.data()), offset, compressedSize);

	// Decompress using Inflate ZIP
	std::vector<T> decompressedData = UnZip<T>(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));

	// Convert decompressed data to native endianness in-place
	endianDecodeBEArray<T>(decompressedData);

	return decompressedData;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIP(std::vector<T>& uncompressedIn, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();
	// Convert uncompressed data to native endianness in-place
	endianEncodeBEArray<T>(uncompressedIn);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = Zip<T>(uncompressedIn);

	return compressedData;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIPPrediction(ByteStream& stream, uint64_t offset, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(compressedSize);
	stream.setOffsetAndRead(reinterpret_cast<char*>(compressedData.data()), offset, compressedSize);

	// Decompress using Inflate ZIP
	std::vector<T> decompressedData = UnZip<T>(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));

	// Remove the prediction encoding from the data as well as converting to native endianness
	std::vector<T> decodedData = RemovePredictionEncoding<T>(std::move(decompressedData), width, height);

	return decodedData;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIPPrediction(std::vector<T>& uncompressedIn, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	// Prediction encode as well as byteswapping
	std::vector<T> predictionEncodedData = PredictionEncode<T>(std::move(uncompressedIn), width, height);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = Zip<T>(predictionEncodedData);

	return compressedData;
}


PSAPI_NAMESPACE_END