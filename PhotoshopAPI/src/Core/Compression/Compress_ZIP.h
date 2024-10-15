#pragma once

#include "Macros.h"
#include "Logger.h"
#include "CompressionUtil.h"
#include "InterleavedToPlanar.h"
#include "Core/Endian/EndianByteSwap.h"
#include "Core/Endian/EndianByteSwapArr.h"
#include "Core/Struct/ByteStream.h"
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
	// The buffer parameter must match the bytesize of the data vector
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void PredictionEncode(std::span<T> data, std::span<uint8_t> buffer, const uint32_t width, const uint32_t height)
	{
		if (data.size() > buffer.size() * sizeof(T))
			PSAPI_LOG_ERROR("PredictionEncode", "Buffer size does not match data size, expected at least %zu bytes but got %zu instead", data.size() * sizeof(T), buffer.size());
		PROFILE_FUNCTION();
		for (uint32_t y = 0; y < height; ++y)
		{
			// Initialize the prediction encoding for the current scanline
			T prev = data[static_cast<uint64_t>(width) * y];
			for (uint32_t x = 1; x < width; ++x)
			{
				// Perform prediction encoding in-place
				T current = data[static_cast<uint64_t>(width) * y + x];
				data[static_cast<uint64_t>(width) * y + x] = current - prev;
				prev = current;
			}
		}

		// After performing prediction encoding, apply big-endian conversion
		endianEncodeBEArray<T>(data);
	}


	// We need to specialize here as 32-bit files have their bytes de-interleaved (i.e. from 1234 1234 1234 1234 byte order to 1111 2222 3333 4444)
	// And we need to do this de-interleaving separately. Thanks to both psd_sdk and psd-tools for having found this out.
	// The buffer parameter must match the bytesize of the data vector
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <>
	inline void PredictionEncode(std::span<float32_t> data, std::span<uint8_t> buffer, const uint32_t width, const uint32_t height)
	{
		if (data.size() > buffer.size() * sizeof(float32_t))
			PSAPI_LOG_ERROR("PredictionEncode", "Buffer size does not match data size, expected at least %zu bytes but got %zu instead", data.size() * sizeof(float32_t), buffer.size());
		PROFILE_FUNCTION();

		std::span<uint8_t> byteDataView(reinterpret_cast<uint8_t*>(data.data()), data.size() * sizeof(float32_t));
		std::vector<uint32_t> verticalIter = createVerticalImageIterator(height);
		{
			PROFILE_SCOPE("32-bit binary de-interleave");
			// First de-interleave the data to planar byte order, i.e. going from 1234 1234 1234 1234 to 1111 2222 3333 4444
			// We essentially split each scanline into 4 equal parts each holding the first, second, third and fourth of the original bytes
			// We also convert to big endian order which is what is stored on disk
			std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(), [&](uint32_t y)
				{
					// Binary view into the whole scanline
					std::span<uint8_t> scanlineView(byteDataView.data() + y * width * sizeof(float32_t), width * sizeof(float32_t));
					std::span<uint8_t> scanlineBuffer(buffer.data() + y * width * sizeof(float32_t), width * sizeof(float32_t));
					interleavedToPlanarFloat(scanlineView, scanlineBuffer, width);
					// Copy over the memory from the buffer into the data
					std::memcpy(scanlineView.data(), scanlineBuffer.data(), scanlineView.size());
				});
		}

		{
			PROFILE_SCOPE("32-bit binary prediction encode");
			// Perform the prediction encoding of the data, keep in mind that this is done byte by byte
			std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
				[&](uint32_t y)
				{
					// Generate a non-overlapping view of each row of the data
					uint8_t* dataRowOffset = reinterpret_cast<uint8_t*>(data.data() + static_cast<uint64_t>(y) * width);
					std::span<uint8_t> dataView(dataRowOffset, width * sizeof(float32_t));

					uint8_t prev = dataView[0];
					for (uint32_t x = 1; x < width * sizeof(float32_t); ++x)
					{
						uint8_t curr = dataView[x];
						// Generate the difference between the current value and the next one and store it
						dataView[x] = curr - prev;
						prev = curr;
					}
				});
		}
	}


	// Use libdeflate to deflate the incoming uncompressed data into the provided buffer using the compressor after which
	// we insert the compressed data into an appropriately sized vector which we return.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<uint8_t> Compress(const std::span<T> uncompressedData, std::span<uint8_t> buffer, libdeflate_compressor* compressor)
	{
		PROFILE_FUNCTION();
		std::vector<uint8_t> compressedData;
		// These represent the header bytes of the zlib stream
		const uint8_t compressionType = 0x78;
		uint8_t compressionByte;
		if constexpr (ZIP_COMPRESSION_LVL < 2)
		{
			compressionByte = 0x01;
		}
		else if constexpr(ZIP_COMPRESSION_LVL < 6)
		{
			compressionByte = 0x5E;
		}
		else if constexpr (ZIP_COMPRESSION_LVL < 8)
		{
			compressionByte = 0x9C;
		}
		else
			compressionByte = 0xDA;
		// Manually write the zlib header 
		compressedData.push_back(compressionType);
		compressedData.push_back(compressionByte);


		const uint8_t* inputBuffer = reinterpret_cast<const uint8_t*>(uncompressedData.data());
		size_t inputBytes = uncompressedData.size() * sizeof(T);
		size_t bytesUsed;
		{
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
std::vector<uint8_t> CompressZIP(std::span<T> uncompressedIn, std::span<uint8_t> buffer, libdeflate_compressor* compressor)
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
std::vector<uint8_t> CompressZIPPrediction(std::span<T> uncompressedIn, std::span<uint8_t> buffer, libdeflate_compressor* compressor, const uint32_t width, const uint32_t height)
{
	PROFILE_FUNCTION();

	// Prediction encode as well as byteswapping in-place
	ZIP_Impl::PredictionEncode<T>(uncompressedIn, buffer, width, height);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = ZIP_Impl::Compress<T>(uncompressedIn, buffer, compressor);

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

	// Allocate the compressor as well as a sufficiently large swap buffer
	libdeflate_compressor* compressor = libdeflate_alloc_compressor(ZIP_COMPRESSION_LVL);
	std::vector<uint8_t> buffer(libdeflate_zlib_compress_bound(compressor, uncompressedIn.size() * sizeof(T)));

	// Prediction encode as well as byteswapping in-place
	ZIP_Impl::PredictionEncode<T>(uncompressedIn, buffer, width, height);

	// Compress using Deflate ZIP
	std::vector<uint8_t> compressedData = ZIP_Impl::Compress<T>(uncompressedIn, buffer, compressor);

	libdeflate_free_compressor(compressor);

	return compressedData;
}


PSAPI_NAMESPACE_END