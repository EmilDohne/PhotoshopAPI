#pragma once

#include "Macros.h"
#include "Util/Logger.h"
#include "Core/Endian/EndianByteSwap.h"
#include "Core/Endian/EndianByteSwapArr.h"
#include "CompressionUtil.h"
#include "Core/Struct/ByteStream.h"
#include "Util/Profiling/Perf/Instrumentor.h"

#include "libdeflate.h"

#include <algorithm>
#include <execution>
#include <vector>
#include <cstring>
#include <bit>


// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif


PSAPI_NAMESPACE_BEGIN


namespace ZIP_Impl
{
	// Use libdeflate to decompress the incoming data into the provided buffer. The decompressedSize parameter refers to the 
	// number of bytes, not to the number of elements
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void Decompress(const std::span<uint8_t> compressedData, std::span<T> buffer, const uint64_t decompressedSize)
	{
		PSAPI_PROFILE_FUNCTION();

		libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();
		if (!decompressor) {
			PSAPI_LOG_ERROR("UnZip", "Cannot allocate decompressor");
		}

		size_t bytesDecompressed;	// The actual uncompressed bytes, for now unused
		auto res = libdeflate_zlib_decompress(
			decompressor,
			compressedData.data(),
			compressedData.size(),
			reinterpret_cast<uint8_t*>(buffer.data()),
			decompressedSize * sizeof(T),
			&bytesDecompressed);

		if (res == LIBDEFLATE_BAD_DATA)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate decompression failed due to invalid input data");
		}
		if (res == LIBDEFLATE_SHORT_OUTPUT)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate decompression failed due to finishing with less bytes than expected.");
		}
		if (res == LIBDEFLATE_INSUFFICIENT_SPACE)
		{
			PSAPI_LOG_ERROR("UnZip", "Inflate decompression failed due to having insufficient output space.");
		}

		libdeflate_free_decompressor(decompressor);
	}


	// Reverse the prediction encoding after having decompressed the zip compressed byte stream as well as converting from BE to native
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void RemovePredictionEncoding(std::span<T> decompressedData, const uint32_t width, const uint32_t height)
	{
		PSAPI_PROFILE_FUNCTION();
		// Convert decompressed data to native endianness in-place
		endianDecodeBEArray<T>(decompressedData);

		std::vector<uint32_t> verticalIter = createVerticalImageIterator(height);

		// Perform prediction decoding per scanline of data in-place
		std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
			[&](uint32_t y)
			{
				for (uint64_t x = 1; x < width; ++x)
				{
					// Simple differencing: decode by adding the difference to the previous value
					decompressedData[static_cast<uint64_t>(width) * y + x] += decompressedData[static_cast<uint64_t>(width) * y + x - 1];
				}
			});
	}


	// We need to specialize here as 32-bit files have their bytes de-interleaved (i.e. from 1234 1234 1234 1234 byte order to 1111 2222 3333 4444)
	// And we need to do this de-interleaving separately. Thanks to both psd_sdk and psd-tools for having found this out
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <>
	inline void RemovePredictionEncoding(std::span<float32_t> decompressedData, const uint32_t width, const uint32_t height)
	{
		PSAPI_PROFILE_FUNCTION();

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
		std::memcpy(reinterpret_cast<char*>(decompressedData.data()), reinterpret_cast<char*>(deinterleavedData.data()), deinterleavedData.size() * sizeof(float32_t));
	}

}


// Decompress an incoming filestream using the Inflate algorithm from the given offset into a buffer equivalent to the size of width * height 
// and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void DecompressZIP(ByteStream& stream, std::span<T> buffer, uint64_t offset, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PSAPI_PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::span<uint8_t> compressedData = stream.read(offset, compressedSize);

	// Decompress using Inflate ZIP into the buffer
	ZIP_Impl::Decompress<T>(compressedData, buffer, static_cast<uint64_t>(width) * height);

	// Convert decompressed data to native endianness in-place
	endianDecodeBEArray<T>(buffer);
}


// Decompress the given buffer using the Inflate algorithm into a buffer equivalent to the size of width * height 
// and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIP(std::vector<uint8_t>& compressedData, const uint32_t width, const uint32_t height)
{
	PSAPI_PROFILE_FUNCTION();
	// Decompress using Inflate ZIP
	std::vector<T> decompressedData(static_cast<uint64_t>(width) * static_cast<uint64_t>(height));
	ZIP_Impl::Decompress<T>(compressedData, std::span<T>(decompressedData), decompressedData.size());
	// Convert decompressed data to native endianness in-place
	endianDecodeBEArray<T>(decompressedData);

	return decompressedData;
}


// Decompress an incoming filestream using the Inflate algorithm with prediction decoding from the given offset into a buffer equivalent to the 
// size of width * height and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void DecompressZIPPrediction(ByteStream& stream, std::span<T> buffer, uint64_t offset, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	PSAPI_PROFILE_FUNCTION();
	// Read the data without converting from BE to native as we need to decompress first
	std::span<uint8_t> compressedData = stream.read(offset, compressedSize);

	// Decompress using Inflate ZIP into the buffer
	ZIP_Impl::Decompress<T>(compressedData, buffer, static_cast<uint64_t>(width) * height);

	// Remove the prediction encoding from the data as well as converting to native endianness
	ZIP_Impl::RemovePredictionEncoding<T>(buffer, width, height);
}


// Decompress the given buffer using the Inflate algorithm with prediction decoding into a buffer equivalent to the size  
// of width * height and return it. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> DecompressZIPPrediction(std::vector<uint8_t>& compressedData, const uint32_t width, const uint32_t height)
{
	PSAPI_PROFILE_FUNCTION();

	// Decompress using Inflate ZIP
	std::vector<T> decompressedData(static_cast<uint64_t>(width) * height);
	ZIP_Impl::Decompress<T>(compressedData, std::span<T>(decompressedData), static_cast<uint64_t>(width) * height);

	// Remove the prediction encoding from the data as well as converting to native endianness
	ZIP_Impl::RemovePredictionEncoding<T>(decompressedData, width, height);

	return decompressedData;
}



PSAPI_NAMESPACE_END