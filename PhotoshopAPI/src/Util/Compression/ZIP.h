#pragma once

#include "../../Macros.h"

#include <zlib-ng.h>


PSAPI_NAMESPACE_BEGIN

template <typename T>
std::vector<T> DecompressZIP(File& document, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(scanlineTotalSize);
	document.read(reinterpret_cast<char*>(compressedData.data()), scanlineTotalSize);

	//
	zng_stream stream{};
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = static_cast<uint32_t>(compressedSize);
	stream.next_in = compressedData.data();

	if (zng_inflateInit(stream) != Z_OK)
	{
		PSAPI_LOG_ERROR("DecompressZIP", "Inflate initialization failed")
	}

	// We do actually want to initialize the data here
	std::vector<uint8_t> decompressedData(static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * sizeof(T));

	stream.avail_out = decompressedData.size();
	stream.next_out = decompressedData.data();

	if (zng_inflate(&stream, Z_FINISH) != Z_STREAM_END)
	{
		PSAPI_LOG_ERROR("DecompressZIP", "Inflate decompression failed")
	}

	if (zng_inflateEnd(&stream) != Z_OK))
	{
		PSAPI_LOG_ERROR("DecompressZIP", "Inflate cleanup failed")
	}

	// Convert decompressed data to native endianness
	std::vector<T> bitShiftedData;
	bitShiftedData.reserve(static_cast<uint64_t>(width) * static_cast<uint64_t>(height));
	for (int i = 0; i < (decompressedData.size() * sizeof(T)); i += sizeof(T))
	{
		bitShiftedData.push_back(endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&decompressedData[i])));
	}

	return std::move(bitShiftedData);
}