#pragma once

#include "Macros.h"
#include "PhotoshopFile/ImageResources.h"
#include "Core/Struct/ResourceBlock.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


/// Generate an ImageResources section based on the options set by the layeredFile
template <typename T>
ImageResources generate_imageresources(LayeredFile<T>& layeredFile)
{
	// Currently there is only 2 Image Resources we parse which is either an ICC profile or DPI
	std::vector<std::unique_ptr<ResourceBlock>> blockVec;

	// Only store the ICC Profile if we actually have data stored on it
	if (layeredFile.icc_profile().data_size() > 0)
	{
		auto iccBlock = ICCProfileBlock(std::move(layeredFile.icc_profile().data()));
		blockVec.push_back(std::make_unique<ICCProfileBlock>(iccBlock));
	}

	auto dpiBlock = ResolutionInfoBlock(layeredFile.dpi());
	blockVec.push_back(std::make_unique<ResolutionInfoBlock>(dpiBlock));

	return ImageResources(std::move(blockVec));
}

PSAPI_NAMESPACE_END
