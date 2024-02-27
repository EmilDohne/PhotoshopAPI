#pragma once

#include "Macros.h"
#include "Logger.h"
#include "Endian/EndianByteSwap.h"
#include "Endian/EndianByteSwapArr.h"
#include "Struct/ByteStream.h"
#include "Profiling/Perf/Instrumentor.h"

#include "zlib-ng.h"

#include <algorithm>
#include <execution>
#include <vector>
#include <span>
#include <tuple>

#include <cstring>

PSAPI_NAMESPACE_BEGIN

// Implementation of the calls to Inflate and Deflate through an abstraction layer
namespace {

	// Use zlib-ng to inflate the compressed input data to the expected output size. decompressedSize is in number of elements
	// not in bytes!
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<T> UnZip(const std::span<uint8_t> compressedData, const uint64_t decompressedSize)
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
			PSAPI_LOG_ERROR("UnZip", "Inflate initialization failed");
		}

		// The size being fixed to exactly what we expect the decompressed data to be sized
		// makes sure we catch any errors in our calculations of decompressedSize or in the input
		// byte stream
		std::vector<T> decompressedData(decompressedSize);
		stream.avail_out = decompressedData.size() * sizeof(T);
		stream.next_out = reinterpret_cast<uint8_t*>(decompressedData.data());


		if (zng_inflate(&stream, Z_FINISH) != Z_STREAM_END)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate decompression failed");
		}

		if (zng_inflateEnd(&stream) != Z_OK)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate cleanup failed");
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
		std::vector<uint8_t> buffer(16 * 1024);

		PROFILE_FUNCTION();
		zng_stream stream{};
		stream.zfree = Z_NULL;
		stream.zalloc = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = uncompressedData.size() * sizeof(T);
		stream.next_in = reinterpret_cast<const uint8_t*>(uncompressedData.data());
		stream.next_out = reinterpret_cast<uint8_t*>(buffer.data());
		stream.avail_out = buffer.size();

		if (zng_deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK)
		{
			PSAPI_LOG_ERROR("Zip", "Deflate init failed");
			return compressedData;
		}

		// Continuously iterate the input stream and compress in chunks into our buffer after which
		// we copy the buffer to the compressedData vec
		int flushState = Z_NO_FLUSH;
		int result = Z_OK;
		do
		{
			// Switch the flag to Z_FINISH if the amount of bytes available is less
			// than our buffer size
			flushState = (stream.avail_in > buffer.size()) ? Z_NO_FLUSH : Z_FINISH;

			do
			{
				stream.next_out = reinterpret_cast<uint8_t*>(buffer.data());
				stream.avail_out = buffer.size();
				result = zng_deflate(&stream, flushState);
				if (result == Z_STREAM_ERROR)
				{
					zng_deflateEnd(&stream);
					PSAPI_LOG_ERROR("Zip", "Unable to call deflate on the input data");
					return compressedData;
				}
				size_t bytesUsed = buffer.size() - stream.avail_out;
				compressedData.insert(compressedData.end(), buffer.begin(), buffer.begin() + bytesUsed);
			} while (stream.avail_out == 0);
		} while (flushState != Z_FINISH);
		if (result != Z_STREAM_END)
		{
			PSAPI_LOG_ERROR("Zip", "Did not compress the whole buffer, there is still %i bytes remaining", stream.avail_in);
		}
		if (zng_deflateEnd(&stream) != Z_OK)
		{
			PSAPI_LOG_ERROR("Zip", "Deflate cleanup failed");
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


// Implementation of the Prediction encoding and decoding algorithms used for ZipPrediction
namespace {

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
		std::vector<float32_t> deinterleavedData(static_cast<uint64_t>(width) * height);
		std::span<uint8_t> deinterleavedDataView(reinterpret_cast<uint8_t*>(deinterleavedData.data()), deinterleavedData.size() * sizeof(float32_t));
		for (uint64_t y = 0; y < height; ++y)
		{
			for (uint64_t x = 0; x < width; ++x)
			{
				uint64_t rowIndex = y * width * sizeof(float32_t);
				deinterleavedDataView[rowIndex + x * sizeof(float32_t) + 0] = predictionDecodedData[rowIndex + x];
				deinterleavedDataView[rowIndex + x * sizeof(float32_t) + 1] = predictionDecodedData[rowIndex + static_cast<uint64_t>(width) + x];
				deinterleavedDataView[rowIndex + x * sizeof(float32_t) + 2] = predictionDecodedData[rowIndex + static_cast<uint64_t>(width) * 2 + x];
				deinterleavedDataView[rowIndex + x * sizeof(float32_t) + 3] = predictionDecodedData[rowIndex + static_cast<uint64_t>(width) * 3 + x];
			}
		}

		endianDecodeBEArray<float32_t>(deinterleavedData);

		return deinterleavedData;
	}


	// Prediction encode the data per scanline while also big endian converting it
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<T> PredictionEncode(std::vector<T>& data, const uint32_t width, const uint32_t height)
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
				uint64_t destOffset = static_cast<uint64_t>(width) * y;
				std::memcpy(reinterpret_cast<void*>(data.data() + destOffset), reinterpret_cast<void*>(buffer.data()), width * sizeof(T));
			});

		endianEncodeBEArray<T>(data);

		return std::move(data);
	}


	// We need to specialize here as 32-bit files have their bytes de-interleaved (i.e. from 1234 1234 1234 1234 byte order to 1111 2222 3333 4444)
	// And we need to do this de-interleaving separately. Thanks to both psd_sdk and psd-tools for having found this out
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <>
	inline std::vector<float32_t> PredictionEncode(std::vector<float32_t>& data, const uint32_t width, const uint32_t height)
	{
		PROFILE_FUNCTION();

		// Endian encode the data first unlike the regular prediction encoding. This is because we operate on a byte level here for 
		// the prediction encoding rather than on the actual values
		endianEncodeBEArray(data);

		std::span<uint8_t> byteDataView(reinterpret_cast<uint8_t*>(data.data()), data.size() * sizeof(float32_t));

		// First de-interleave the data to planar byte order, i.e. going from 1234 1234 1234 1234 to 1111 2222 3333 4444
		// We essentially split each scanline into 4 equal parts each holding the first, second, third and fourth of the original bytes
		// Additionally we endian encode the data by reversing the order in which we access the original bytes
		std::vector<uint32_t> verticalIter = createVerticalImageIterator(height);
		std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
			[&](uint32_t y)
			{
				// this buffer will hold the interleaved bytes of each scanline
				std::vector<uint8_t> buffer(width * sizeof(float32_t));
				// convenient views into the individual sectors of the data
				std::span<uint8_t> firstRowView(buffer.data() + width * 0, width);
				std::span<uint8_t> secondRowView(buffer.data() + width * 1, width);
				std::span<uint8_t> thirdRowView(buffer.data() + width * 2, width);
				std::span<uint8_t> fourthRowView(buffer.data() + width * 3, width);

				for (uint64_t x = 0; x < width; ++x)
				{
					uint64_t rowCoord = static_cast<uint64_t>(y) * width * sizeof(float32_t);
					
					firstRowView[x] =  byteDataView[rowCoord + x * sizeof(float32_t) + 0];	// Byte 0
					secondRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 1];	// Byte 1
					thirdRowView[x] =  byteDataView[rowCoord + x * sizeof(float32_t) + 2];	// Byte 2
					fourthRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 3];	// Byte 3
				}

				// Copy the row back over, no need to lock here as the ranges are not overlapping
				std::memcpy(reinterpret_cast<void*>(data.data() + y * width), reinterpret_cast<void*>(buffer.data()), buffer.size());
			});

		// Perform the prediction encoding of the data, keep in mind that this is done byte by byte
		std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
			[&](uint32_t y)
			{
				std::vector<uint8_t> buffer(width * sizeof(float32_t));
				// Generate a non-overlapping view of each row of the data
				uint8_t* dataRowOffset = reinterpret_cast<uint8_t*>(data.data() + static_cast<uint64_t>(y) * width);
				std::span<uint8_t> dataView(dataRowOffset, width * sizeof(float32_t));
				// We must set the initial value manually as the prediction encoding goes from the first value
				buffer[0] = dataView[0];
				for (uint32_t x = 1; x < width * sizeof(float32_t); ++x)
				{
					// Generate the difference between the current value and the next one and store it
					buffer[x] = dataView[x] - dataView[x - 1];
				}
				std::memcpy(reinterpret_cast<void*>(dataView.data()), reinterpret_cast<void*>(buffer.data()), width * sizeof(float32_t));
			});


		return std::move(data);
	}
}


// Decompress an incoming filestream using the Inflate algorithm from the given offset into a buffer equivalent to the size of width * height 
// and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIP(ByteStream& stream, uint64_t offset, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::span<uint8_t> compressedData = stream.read(offset, compressedSize);

	// Decompress using Inflate ZIP
	std::vector<T> decompressedData = UnZip<T>(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));

	// Convert decompressed data to native endianness in-place
	endianDecodeBEArray<T>(decompressedData);

	return decompressedData;
}


// Decompress the given buffer using the Inflate algorithm into a buffer equivalent to the size of width * height 
// and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIP(std::vector<uint8_t>& compressedData, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();
	// Decompress using Inflate ZIP
	std::vector<T> decompressedData = UnZip<T>(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));
	// Convert decompressed data to native endianness in-place
	endianDecodeBEArray<T>(decompressedData);

	return decompressedData;
}


// Compress a vector using the Deflate algorithm with default compression level
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIP(std::vector<T>& uncompressedIn)
{
	PROFILE_FUNCTION();
	// Convert uncompressed data to native endianness in-place
	endianEncodeBEArray<T>(uncompressedIn);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = Zip<T>(uncompressedIn);

	return compressedData;
}


// Decompress an incoming filestream using the Inflate algorithm with prediction decoding from the given offset into a buffer equivalent to the 
// size of width * height and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIPPrediction(ByteStream& stream, uint64_t offset, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::span<uint8_t> compressedData = stream.read(offset, compressedSize);

	// Decompress using Inflate ZIP
	std::vector<T> decompressedData = UnZip<T>(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));

	// Remove the prediction encoding from the data as well as converting to native endianness
	std::vector<T> decodedData = RemovePredictionEncoding<T>(std::move(decompressedData), width, height);

	return decodedData;
}


// Decompress the given buffer using the Inflate algorithm with prediction decoding into a buffer equivalent to the size  
// of width * height and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIPPrediction(std::vector<uint8_t>& compressedData, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	// Decompress using Inflate ZIP
	std::vector<T> decompressedData = UnZip<T>(compressedData, static_cast<uint64_t>(width) * static_cast<uint64_t>(height));

	// Remove the prediction encoding from the data as well as converting to native endianness
	std::vector<T> decodedData = RemovePredictionEncoding<T>(std::move(decompressedData), width, height);

	return decodedData;
}


// Compress a vector using the Deflate algorithm with default compression level while prediction encoding the data
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIPPrediction(std::vector<T>& uncompressedIn, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	// Prediction encode as well as byteswapping
	std::vector<T> predictionEncodedData = PredictionEncode<T>(uncompressedIn, width, height);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = Zip<T>(predictionEncodedData);

	return compressedData;
}


PSAPI_NAMESPACE_END