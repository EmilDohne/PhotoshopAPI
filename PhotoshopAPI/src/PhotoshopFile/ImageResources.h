#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "Struct/ResourceBlock.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

struct ImageResources : public FileSection
{
	std::vector<ResourceBlock> m_ResourceBlocks;

	ImageResources() { m_Size = 4u; };
	ImageResources(std::vector<ResourceBlock> resourceBlocks);

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	void read(File& document, const uint64_t offset);
	void write(File& document);
};


PSAPI_NAMESPACE_END