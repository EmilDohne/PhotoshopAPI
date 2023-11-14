#include "../../Macros.h"
#include "../Read.h"
#include "../Struct/File.h"
#include "../../PhotoshopFile/FileHeader.h"



// Decompresses an image buffer encoded in RLE format,
// Assumes that data is already converted from BE into native
template<typename T>
inline std::vector<T> DecompressRLE(File& document, const FileHeader& header, const uint32_t width, const uint32_t height)
{
	std::vector<T> decompressedData(sizeof(T) * static_cast<uint64_t>(width) * static_cast<uint64_t>(height));

	// Photoshop first stores the byte counts of all the scanlines, this is 2 or 4 bytes depending on 
	// if the document is PSD or PSB
	std::vector<uint32_t> scanlineSizes(height);
	uint64_t scanlineTotalSize = 0u;
	for (int i = 0; i < height; ++i)
	{
		scanlineSizes.push_back(ExtractWidestValue<uint16_t, uint32_t>(ReadBinaryDataVariadic<uint16_t, uint32_t>(document)));
		scanlineTotalSize += scanlineSizes[i];
	}

	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> data(scanlineTotalSize);
	document.read(reinterpret_cast<char*>(data.data()), scanlineTotalSize);


	// Decompress using the PackBits algorithm
	{
		size_t i = 0;
		while (i < data.size()) {
			uint8_t value = data[i];

			if (value == 128) {
				// Do nothing, nop
			}
			else if (value > 128) {
				// Repeated byte
				value = 256 - value;
				decompressedData.insert(decompressedData.end(), value + 1, data[i + 1]);
				++i;
			}
			else {
				// Literal bytes
				decompressedData.insert(decompressedData.end(), data.begin() + i + 1, data.begin() + i + value + 1);
				i += value;
			}
		}
	}

	return std::move(decompressedData);
};