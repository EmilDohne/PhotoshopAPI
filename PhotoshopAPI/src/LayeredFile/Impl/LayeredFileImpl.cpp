#include "LayeredFileImpl.h"


PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ICCProfile LayeredFileImpl::read_icc_profile(const PhotoshopFile* file)
{
	const auto blockPtr = file->m_ImageResources.getResourceBlockView<ICCProfileBlock>(Enum::ImageResource::ICCProfile);
	if (blockPtr)
	{
		return ICCProfile{ blockPtr->m_RawICCProfile };
	}
	return ICCProfile{};
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
float LayeredFileImpl::read_dpi(const PhotoshopFile* file)
{
	const auto blockPtr = file->m_ImageResources.getResourceBlockView<ResolutionInfoBlock>(Enum::ImageResource::ResolutionInfo);
	if (blockPtr)
	{
		// We dont actually have to do any back and forth conversions here since the value is always stored as DPI and never as 
		// DPCM
		return blockPtr->m_HorizontalRes.getFloat();
	}
	return 72.0f;
}

PSAPI_NAMESPACE_END