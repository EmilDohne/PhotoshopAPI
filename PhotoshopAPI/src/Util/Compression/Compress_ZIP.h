#pragma once

#include "Macros.h"
#include "Logger.h"
#include "CompressionUtil.h"
#include "Endian/EndianByteSwap.h"
#include "Endian/EndianByteSwapArr.h"
#include "Struct/ByteStream.h"
#include "Profiling/Perf/Instrumentor.h"

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
	// Prediction encode the data per scanline while also big endian converting it
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<T> PredictionEncode(std::vector<T>& data, const uint32_t width, const uint32_t height)
	{
		PROFILE_FUNCTION();
		// Here we must first prediction encode per scanline and then do the big endian conversion afterwards
		// This operation unfortunately cannot be done in-place but we can use a temporary buffer to avoid unnecessary memory overhead
		std::vector<T> buffer(width);
		for (int y = 0; y < height; ++y)
		{
			// We must set the initial value manually as the prediction encoding goes from the first value
			buffer[0] = data[static_cast<uint64_t>(width) * y];
			for (uint32_t x = 1; x < width; ++x)
			{
				// Generate the difference between the current value and the next one and store it
				buffer[x] = data[static_cast<uint64_t>(width) * y + x] - data[static_cast<uint64_t>(width) * y + x - 1];
			}
			uint64_t destOffset = static_cast<uint64_t>(width) * y;
			std::memcpy(reinterpret_cast<void*>(data.data() + destOffset), reinterpret_cast<void*>(buffer.data()), width * sizeof(T));
		}

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
		// Additionally we endian encode the data by reversing the order in which we access the original bytes.
		// TODO this can actually be done in-place with vectorization
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

					firstRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 0];	// Byte 0
					secondRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 1];	// Byte 1
					thirdRowView[x] = byteDataView[rowCoord + x * sizeof(float32_t) + 2];	// Byte 2
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


	// Use libdeflate to deflate the incoming uncompressed data into the provided buffer using the compressor after which
	// we insert the compressed data into an appropriately sized vector which we return.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<uint8_t> Compress(const std::vector<T>& uncompressedData, std::span<uint8_t> buffer, libdeflate_compressor* compressor)
	{
		std::vector<uint8_t> compressedData;
		// These represent the header bytes of the zlib stream
		const uint8_t compressionType = 0x78;
		uint8_t compressionByte;
		if (ZIP_COMPRESSION_LVL < 2)
			compressionByte = 0x01;
		else if (ZIP_COMPRESSION_LVL < 6)
			compressionByte = 0x5E;
		else if (ZIP_COMPRESSION_LVL < 8)
			compressionByte = 0x9C;
		else
			compressionByte = 0xDA;
		// Manually write the zlib header 
		compressedData.push_back(compressionType);
		compressedData.push_back(compressionByte);


		const uint8_t* inputBuffer = reinterpret_cast<const uint8_t*>(uncompressedData.data());
		size_t inputBytes = uncompressedData.size() * sizeof(T);
		size_t bytesUsed;
		{
			PROFILE_SCOPE("Zip Deflate");
			bytesUsed = libdeflate_deflate_compress(compressor, inputBuffer, inputBytes,
				buffer.data(), buffer.size());
			if (bytesUsed == 0) {
				PSAPI_LOG_ERROR("Zip", "Compression failed");
			}
		}

		// Adjust the size of the output buffer to the actual size of compressed data
		{
			PROFILE_SCOPE("Zip Insert buffer");
			compressedData.insert(compressedData.end(), buffer.begin(), buffer.begin() + bytesUsed);
		}

		// Add the adler-32 checksum as big endian value
		uint32_t adler32Checksum = libdeflate_adler32(1, reinterpret_cast<const uint8_t*>(uncompressedData.data()), uncompressedData.size() * sizeof(T));
		// Push back the individual bytes of the adler checksum at the end
		if constexpr (std::endian::native == std::endian::little)
		{
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[3]);
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[2]);
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[1]);
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[0]);
		}
		else
		{
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[0]);
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[1]);
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[2]);
			compressedData.push_back(reinterpret_cast<uint8_t*>(&adler32Checksum)[3]);
		}

		return compressedData;
	}
}


// Compress a vector using the Deflate algorithm with default compression level. This is the optimized but less abstracted
// version of this function taking a swap buffer as well as a pre-allocated compressor
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIP(std::vector<T>& uncompressedIn, std::span<uint8_t> buffer, libdeflate_compressor* compressor)
{
	PROFILE_FUNCTION();
	// Convert uncompressed data to native endianness in-place
	endianEncodeBEArray<T>(uncompressedIn);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = ZIP_Impl::Compress<T>(uncompressedIn, buffer, compressor);

	return compressedData;
}


// Compress a vector using the Deflate algorithm with default compression level. This is the generic function taking
// data and compressing it without any further information on what is used
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIP(std::vector<T>& uncompressedIn)
{
	PROFILE_FUNCTION();
	// Convert uncompressed data to native endianness in-place
	endianEncodeBEArray<T>(uncompressedIn);

	// Allocate the compressor as well as a sufficiently large swap buffer
	libdeflate_compressor* compressor = libdeflate_alloc_compressor(ZIP_COMPRESSION_LVL);
	std::vector<uint8_t> buffer(libdeflate_zlib_compress_bound(compressor, uncompressedIn.size() * sizeof(T)));

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = ZIP_Impl::Compress<T>(uncompressedIn, buffer, compressor);

	libdeflate_free_compressor(compressor);

	return compressedData;
}


// Compress a vector using the Deflate algorithm with default compression level while prediction encoding the data. This 
// is the optimized but less abstracted version of this function taking a swap buffer as well as a pre-allocated compressor.
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIPPrediction(std::vector<T>& uncompressedIn, std::span<uint8_t> buffer, libdeflate_compressor* compressor, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	// Prediction encode as well as byteswapping
	std::vector<T> predictionEncodedData = ZIP_Impl::PredictionEncode<T>(uncompressedIn, width, height);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = ZIP_Impl::Compress<T>(predictionEncodedData, buffer, compressor);

	return compressedData;
}


// Compress a vector using the Deflate algorithm with default compression level while prediction encoding the data. This 
// is the optimized but less abstracted version of this function taking a swap buffer as well as a pre-allocated compressor.
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<uint8_t> CompressZIPPrediction(std::vector<T>& uncompressedIn, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	// Prediction encode as well as byteswapping
	std::vector<T> predictionEncodedData = ZIP_Impl::PredictionEncode<T>(uncompressedIn, width, height);

	// Allocate the compressor as well as a sufficiently large swap buffer
	libdeflate_compressor* compressor = libdeflate_alloc_compressor(ZIP_COMPRESSION_LVL);
	std::vector<uint8_t> buffer(libdeflate_zlib_compress_bound(compressor, predictionEncodedData.size() * sizeof(T)));

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = ZIP_Impl::Compress<T>(predictionEncodedData, buffer, compressor);

	libdeflate_free_compressor(compressor);

	return compressedData;
}


PSAPI_NAMESPACE_END