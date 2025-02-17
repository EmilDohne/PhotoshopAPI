#pragma once

#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Util.h"
#include "Util/Logger.h"
#include "Core/Endian/EndianByteSwap.h"
#include "Core/Endian/EndianByteSwapArr.h"
#include "Core/Struct/File.h"
#include "Core/Struct/ByteStream.h"
#include "PhotoshopFile/FileHeader.h"
#include "Util/Profiling/Perf/Instrumentor.h"

#include <vector>
#include <limits>

#include <cstring>
#include <inttypes.h>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif


PSAPI_NAMESPACE_BEGIN


namespace RLE_Impl
{
    // This is the packbits algorithm described here: https://en.wikipedia.org/wiki/PackBits we iterate byte by byte and 
    // compress. The logic is heavily adapted from MolecularMatters and credit goes to them:
    // https://github.com/MolecularMatters/psd_sdk/blob/master/src/Psd/PsdDecompressRle.cpp
    // We assume a compression of a single scanline as they are all independant of each other
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    inline std::vector<uint8_t> CompressPackBits(const std::span<const uint8_t> uncompressedScanline, uint32_t& scanlineSize)
    {
        // We assume a ~4x compression ratio for RLE to avoid continuously reserving more size
        std::vector<uint8_t> compressedData;
        compressedData.reserve(uncompressedScanline.size() / 4);

        // Keep track of how long our run / no run is
        uint8_t runLen = 0u;
        uint8_t nonRunLen = 0u;

        for (int i = 1; i < uncompressedScanline.size(); ++i)
        {
            const uint8_t prev = uncompressedScanline[i - 1];
            const uint8_t curr = uncompressedScanline[i];

            // We have a run of at least 2 bytes
            if (prev == curr)
            {
                if (nonRunLen != 0)
                {
                    // End the non run len and store how long our non run length is in the header byte
                    compressedData.push_back(nonRunLen - 1u);
                    for (int j = 0; j < nonRunLen; ++j)
                    {
                        compressedData.push_back(uncompressedScanline[i - nonRunLen - 1u + j]);
                    }
                    nonRunLen = 0;
                }

                ++runLen;

                // runs cant be any longer than this due to the way that they are encoded so we are forced to terminate here
                if (runLen == 128u)
                {
                    compressedData.push_back(static_cast<uint8_t>(257u - runLen));
                    compressedData.push_back(curr);
                    runLen = 0u;
                }
            }
            else
            {
                // End the run if there is one going on
                if (runLen != 0)
                {
                    ++runLen;

                    compressedData.push_back(static_cast<uint8_t>(257u - runLen));
                    compressedData.push_back(prev);
                    runLen = 0u;
                }
                else
                {
                    ++nonRunLen;
                }

                // Same as the termination condition on run lengths
                if (nonRunLen == 128u)
                {
                    compressedData.push_back(nonRunLen - 1u);
                    for (int j = 0; j < nonRunLen; ++j)
                    {
                        compressedData.push_back(uncompressedScanline[i - nonRunLen + j]);
                    }
                    nonRunLen = 0;
                }
            }
        }

        // After having iterated all the items we must now encode the last item
        if (runLen != 0)
        {
            ++runLen;
            // Push back the last element as the run
            compressedData.push_back(static_cast<uint8_t>(257u - runLen));
            compressedData.push_back(uncompressedScanline[uncompressedScanline.size() - 1u]);
        }
        else
        {
            ++nonRunLen;
            compressedData.push_back(nonRunLen - 1u);
            for (int j = 0; j < nonRunLen; ++j)
            {
                compressedData.push_back(uncompressedScanline[uncompressedScanline.size() - nonRunLen + j]);
            }
            nonRunLen = 0;
        }

        // The section is padded to 2 bytes, if we need to insert a padding byte we use the no-op
        // value of 128
        if (compressedData.size() % 2 != 0)
        {
            compressedData.push_back(128u);
        }

        // Store and return
        scanlineSize = static_cast<uint32_t>(compressedData.size());
        return compressedData;
    }


    // This is the packbits algorithm described here: https://en.wikipedia.org/wiki/PackBits we iterate byte by byte and 
    // compress. We use the buffer to store our compressed data and return a span at the same memory address but with the 
    // actual compressed size
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    inline std::span<uint8_t> CompressPackBits(const std::span<const uint8_t> uncompressedScanline, std::span<uint8_t> buffer)
    {
        // Total bytes written, used to generate a new span in the end with the correct sizing
        uint32_t bytesWritten = 0u;
        // Keep track of how long our run / no run is
        uint8_t runLen = 0u;
        uint8_t nonRunLen = 0u;

        for (int i = 1; i < uncompressedScanline.size(); ++i)
        {
            const uint8_t prev = uncompressedScanline[i - 1];
            const uint8_t curr = uncompressedScanline[i];

            // We have a run of at least 2 bytes
            if (prev == curr)
            {
                if (nonRunLen != 0)
                {
                    // End the non run len and store how long our non run length is in the header byte
                    buffer[bytesWritten] = (nonRunLen - 1u);
                    ++bytesWritten;
                    for (int j = 0; j < nonRunLen; ++j)
                    {
                        buffer[bytesWritten] = uncompressedScanline[i - nonRunLen - 1u + j];
                        ++bytesWritten;
                    }
                    nonRunLen = 0;
                }
                ++runLen;

                // runs cant be any longer than this due to the way that they are encoded so we are forced to terminate here
                if (runLen == 128u)
                {
                    buffer[bytesWritten] = static_cast<uint8_t>(257u - runLen);
                    ++bytesWritten;
                    buffer[bytesWritten] = curr;
                    ++bytesWritten;
                    runLen = 0u;
                }
            }
            else
            {
                // End the run if there is one going on
                if (runLen != 0)
                {
                    ++runLen;

                    buffer[bytesWritten] = static_cast<uint8_t>(257u - runLen);
                    ++bytesWritten;
                    buffer[bytesWritten] = prev;
                    ++bytesWritten;

                    runLen = 0u;
                }
                else
                {
                    ++nonRunLen;
                }

                // Same as the termination condition on run lengths
                if (nonRunLen == 128u)
                {
                    buffer[bytesWritten] = (nonRunLen - 1u);
                    ++bytesWritten;
                    for (int j = 0; j < nonRunLen; ++j)
                    {
                        buffer[bytesWritten] = uncompressedScanline[i - nonRunLen + j];
                        ++bytesWritten;
                    }
                    nonRunLen = 0;
                }
            }
        }

        // After having iterated all the items we must now encode the last item
        if (runLen != 0)
        {
            ++runLen;
            // Push back the last element as the run
            buffer[bytesWritten] = static_cast<uint8_t>(257u - runLen);
            ++bytesWritten;
            buffer[bytesWritten] = uncompressedScanline[uncompressedScanline.size() - 1u];
            ++bytesWritten;
        }
        else
        {
            ++nonRunLen;
            buffer[bytesWritten] = nonRunLen - 1u;
            ++bytesWritten;
            for (int j = 0; j < nonRunLen; ++j)
            {
                
                buffer[bytesWritten] = uncompressedScanline[uncompressedScanline.size() - nonRunLen + j];
                ++bytesWritten;
            }
            nonRunLen = 0;
        }

        // The section is padded to 2 bytes, if we need to insert a padding byte we use the no-op
        // value of 128
        if (bytesWritten % 2 != 0)
        {
            buffer[bytesWritten] = 128u;
            ++bytesWritten;
        }

        // Generate a "resized" span and return it
        return std::span<uint8_t>(buffer.data(), bytesWritten);
    }


    // Calculate the maximum overhead for a RLE compressed stream given that none of the bytes could be compressed
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    template <typename T>
    size_t MaxCompressedSize(const FileHeader& header, const size_t height, const size_t width, bool includeScanlineSize = true)
    {
        size_t byteCount = height * width * sizeof(T);
        // Add the scanline sizes
        if (includeScanlineSize)
        {
            if (header.m_Version == Enum::Version::Psd)
                byteCount += height * sizeof(uint16_t);
            else
                byteCount += height * sizeof(uint32_t);
        }

        // There is two cases one could encounter with RLE compression, either a run or a no-run. A run is self-contained
        // as it goes from a run of at least 2 bytes which means even if its only two bytes we have net the same size. 
        // The worst case scenario for packbits is however if we have a run right next to a non-run so i.e.
        // 127 127 237 127 127 237...
        // since then we would have to add an additional non-run byte for every in-between byte. The sequence would then look
        // as follows:
        // 255 127 0 237 255 127 0 237
        // This means at worst RLE produces a 33% larger file although this is very uncommon
        size_t numWorstCase = width / 3u;
        // If it has a run on an end and we have another number at the end this byte is duplicated
        if (width % 3 != 0)
            ++numWorstCase;
        // The section is padded to 2 bytes
        if ((width + numWorstCase) % 2 != 0)
            ++numWorstCase;

        byteCount += numWorstCase * height;
        return byteCount;
    }
}


// Compresses a single channel using the packbits algorithm into a binary array as well as big endian encoding it. Returns a binary vector of data
// with the size of each scanline as either a 2- or 4-byte unsigned int preceding it
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::vector<uint8_t> CompressRLE(std::span<T> uncompressedData, std::span<uint8_t> buffer, const FileHeader& header, const uint32_t width, const uint32_t height)
{
    PSAPI_PROFILE_FUNCTION();
    endianEncodeBEArray(uncompressedData);

    // Generate spans for uncompressed as well as compressed data such that each thread
    // can decode in parallel and we can memcpy at the end 
    size_t maxScanlineSize = RLE_Impl::MaxCompressedSize<T>(header, 1, width, false);
    if (maxScanlineSize * height > buffer.size())
        PSAPI_LOG_ERROR("RLE", "Was passed incorrectly sized buffer, expected at least %zu bytes but instead got %zu bytes", maxScanlineSize * height, buffer.size());
    std::vector<std::span<const uint8_t>> uncompressedDataViews;
    std::vector<std::span<uint8_t>> compressedDataViews;
    std::vector<uint32_t> verticalIter;
    for (uint32_t i = 0; i < height; ++i)
    {
        std::span<const uint8_t> uncompressedView(reinterpret_cast<const uint8_t*>(uncompressedData.data() + width * i), width * sizeof(T));
        uncompressedDataViews.push_back(uncompressedView);
        std::span<uint8_t> compressedView(buffer.data() + i * maxScanlineSize, maxScanlineSize);
        compressedDataViews.push_back(compressedView);
        verticalIter.push_back(i);
    }

    // Compress each scanline while additionally overwriting the compressedDataViews with a properly sized span
    // rather than aligned to maxScanlineSize
    std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(), [&](const auto index)
        {
            compressedDataViews[index] = RLE_Impl::CompressPackBits(uncompressedDataViews[index], compressedDataViews[index]);
        });

    // We now take the data out of the buffer and copy them over in parallel
    std::vector<uint8_t> compressedData;
    if (header.m_Version == Enum::Version::Psd)
    {
        std::vector<size_t> scanlineOffsets;
        std::vector<uint16_t> scanlineSizes;
        size_t totalSize = height * sizeof(uint16_t);
        for (uint32_t y = 0; y < height; ++y)
        {
            scanlineOffsets.push_back(totalSize);
            scanlineSizes.push_back(static_cast<uint16_t>(compressedDataViews[y].size()));
            totalSize += compressedDataViews[y].size();
        }
        compressedData = std::vector<uint8_t>(totalSize);

        std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(), [&](const auto index)
            {
                const uint8_t* srcAddress = compressedDataViews[index].data();
                uint8_t* dstAddress = compressedData.data() + scanlineOffsets[index];
                std::memcpy(dstAddress, srcAddress, compressedDataViews[index].size());
            });
        // We deliberately only copy over the scanline sizes at the end since they need to be endian swapped first
        endianEncodeBEArray(std::span<uint16_t>(scanlineSizes));
        std::memcpy(compressedData.data(), reinterpret_cast<uint8_t*>(scanlineSizes.data()), height * sizeof(uint16_t));
    }
    else
    {
        std::vector<size_t> scanlineOffsets;
        std::vector<uint32_t> scanlineSizes;
        size_t totalSize = height * sizeof(uint32_t);
        for (uint32_t y = 0; y < height; ++y)
        {
            scanlineOffsets.push_back(totalSize);
            scanlineSizes.push_back(static_cast<uint32_t>(compressedDataViews[y].size()));
            totalSize += compressedDataViews[y].size();
        }
        compressedData = std::vector<uint8_t>(totalSize);

        std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(), [&](auto& index)
            {
                const uint8_t* srcAddress = compressedDataViews[index].data();
                uint8_t* dstAddress = compressedData.data() + scanlineOffsets[index];
                std::memcpy(dstAddress, srcAddress, compressedDataViews[index].size());
            });
        // We deliberately only copy over the scanline sizes at the end since they need to be endian swapped first
        endianEncodeBEArray(std::span<uint32_t>(scanlineSizes));
        std::memcpy(compressedData.data(), reinterpret_cast<uint8_t*>(scanlineSizes.data()), height * sizeof(uint32_t));
    }

    return compressedData;
}



// Compresses a single channel using the packbits algorithm into a binary array as well as big endian encoding it. Returns a binary vector of data
// with the size of each scanline as either a 2- or 4-byte unsigned int preceding it
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::vector<uint8_t> CompressRLE(std::vector<T>& uncompressedData, const FileHeader& header, const uint32_t width, const uint32_t height)
{
    PSAPI_PROFILE_FUNCTION();
    endianEncodeBEArray(std::span<T>(uncompressedData));

    std::vector<std::span<uint8_t>> uncompressedDataViews;
    for (int i = 0; i < height; ++i)
    {
        // Generate a span for each scanline
        std::span<uint8_t> data(reinterpret_cast<uint8_t*>(uncompressedData.data() + width * i), width * sizeof(T));
        uncompressedDataViews.push_back(data);
    }

    // Allocate the size required for all the scanline sizes ahead of time
    std::vector<uint8_t> compressedData(SwapPsdPsb<uint16_t, uint32_t>(header.m_Version) * height, 0u);
    // Compress each scanline of the uncompressed data individually and push it into the compressed data
    // While also filling out the scanlineSizes vector
    for (int i = 0; i < uncompressedDataViews.size(); ++i)
    {
        uint32_t scanlineSize = 0u;
        std::vector<uint8_t> data = RLE_Impl::CompressPackBits(uncompressedDataViews[i], scanlineSize);

        // Insert the scanline size at the start of the data in our pre-allocated buffer.
        // For PSD we must shrink the value to uint16_t
        const size_t scanlineIndex = i * SwapPsdPsb<uint16_t, uint32_t>(header.m_Version);
        if (header.m_Version == Enum::Version::Psd)
        {
            if (scanlineSize > (std::numeric_limits<uint16_t>::max)()) [[unlikely]]
                {
                    PSAPI_LOG_ERROR("CompressRLE", "Scanline sizes cannot exceed the numeric limits of 16-bit values when writing a PSD file");
                }
                uint16_t scanlineSizeu16 = static_cast<uint16_t>(scanlineSize);
                scanlineSizeu16 = endian_encode_be(scanlineSizeu16);
                // Set the data at the correct index
                std::memcpy(reinterpret_cast<void*>(compressedData.data() + scanlineIndex), &scanlineSizeu16, sizeof(uint16_t));
        }
        else
        {
            scanlineSize = endian_encode_be(scanlineSize);
            std::memcpy(reinterpret_cast<void*>(compressedData.data() + scanlineIndex), &scanlineSize, sizeof(uint32_t));
        }

        // Since our compressed data has the scanline sizes preallocated we can just insert at the end and that will be correct
        compressedData.insert(std::end(compressedData), std::begin(data), std::end(data));
    }


    return compressedData;
}


// Compress a channel of the ImageData section at the end of the file using PackBits and storing the size of the individual scanlines 
// in the scanlineSizes parameter. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::vector<uint8_t> CompressRLEImageDataPsd(std::vector<T>& uncompressedData, const uint32_t width, const uint32_t height, std::vector<uint16_t>& scanlineSizes)
{
    PSAPI_PROFILE_FUNCTION();
    endianEncodeBEArray(std::span<T>(uncompressedData));

    std::vector<std::span<uint8_t>> uncompressedDataViews;
    for (uint32_t i = 0; i < height; ++i)
    {
        // Generate a span for each scanline
        std::span<uint8_t> data(reinterpret_cast<uint8_t*>(uncompressedData.data() + width * i), width * sizeof(T));
        uncompressedDataViews.push_back(data);
    }

    std::vector<uint8_t> compressedData = {};
    // Compress each scanline of the uncompressed data individually and push it into the compressed data
    // While also filling out the scanlineSizes vector
    for (int i = 0; i < uncompressedDataViews.size(); ++i)
    {
        uint32_t scanlineSize = 0u;
        std::vector<uint8_t> data = RLE_Impl::CompressPackBits(uncompressedDataViews[i], scanlineSize);

        if (scanlineSize > (std::numeric_limits<uint16_t>::max)()) [[unlikely]]
            {
                PSAPI_LOG_ERROR("CompressRLE", "Scanline size would exceed the size of a uint16_t, this is not valid");
            }
            scanlineSizes.push_back(static_cast<uint16_t>(scanlineSize));

            // Since our compressed data has the scanline sizes preallocated we can just insert at the end and that will be correct
            compressedData.insert(std::end(compressedData), std::begin(data), std::end(data));
    }

    return compressedData;
}


// Compress a channel of the ImageData section at the end of the file using PackBits and storing the size of the individual scanlines 
// in the scanlineSizes parameter. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::vector<uint8_t> CompressRLEImageDataPsb(std::vector<T>& uncompressedData, const uint32_t width, const uint32_t height, std::vector<uint32_t>& scanlineSizes)
{
    PSAPI_PROFILE_FUNCTION();
    endianEncodeBEArray(std::span<T>(uncompressedData));

    std::vector<std::span<uint8_t>> uncompressedDataViews;
    for (uint32_t i = 0; i < height; ++i)
    {
        // Generate a span for each scanline
        std::span<uint8_t> data(reinterpret_cast<uint8_t*>(uncompressedData.data() + width * i), width * sizeof(T));
        uncompressedDataViews.push_back(data);
    }

    std::vector<uint8_t> compressedData = {};
    // Compress each scanline of the uncompressed data individually and push it into the compressed data
    // While also filling out the scanlineSizes vector
    for (int i = 0; i < uncompressedDataViews.size(); ++i)
    {
        uint32_t scanlineSize = 0u;
        std::vector<uint8_t> data = RLE_Impl::CompressPackBits(uncompressedDataViews[i], scanlineSize);

        scanlineSizes.push_back(scanlineSize);

        // Since our compressed data has the scanline sizes preallocated we can just insert at the end and that will be correct
        compressedData.insert(std::end(compressedData), std::begin(data), std::end(data));
    }

    return compressedData;
}


PSAPI_NAMESPACE_END