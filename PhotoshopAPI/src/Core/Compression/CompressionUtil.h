#pragma once


PSAPI_NAMESPACE_BEGIN

// We express the zip compression level as constexpr here but this is subject to change
constexpr auto ZIP_COMPRESSION_LVL = 4;


// Creates two vectors that can be used as iterators for an image by height or width. 
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> createImageIterators(const uint32_t width, const uint32_t height)
{
	std::vector<uint32_t> horizontalIter(width);
	std::vector<uint32_t> verticalIter(height);

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


// Creates one vector that can be used as iterators for an image by height/scanline.
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline std::vector<uint32_t> createVerticalImageIterator(const uint32_t height)
{
	std::vector<uint32_t> verticalIter(height);
	for (uint32_t i = 0; i < height; ++i)
	{
		verticalIter[i] = i;
	};

	return verticalIter;
}

PSAPI_NAMESPACE_END