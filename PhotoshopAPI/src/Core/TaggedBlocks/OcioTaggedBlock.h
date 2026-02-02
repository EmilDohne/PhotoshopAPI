#pragma once

#include "Core/Struct/File.h"
#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/Signature.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

PSAPI_NAMESPACE_BEGIN

/// This supposedly supersedes the PlacedLayerTaggedBlock since Photoshop CS3 but it appears that those two are always there in conjunction.
/// Likely to keep backwards compatibility
struct OcioTaggedBlock : TaggedBlock
{
	std::unique_ptr<Descriptors::Descriptor> m_Descriptor = std::make_unique<Descriptors::Descriptor>();

	OcioTaggedBlock() = default;
	OcioTaggedBlock(std::unique_ptr<Descriptors::Descriptor> descriptor)
		: m_Descriptor(std::move(descriptor)) {}

	OcioTaggedBlock(const OcioTaggedBlock&) = delete;
	OcioTaggedBlock& operator=(const OcioTaggedBlock&) = delete;
	OcioTaggedBlock(OcioTaggedBlock&&) noexcept = default;
	OcioTaggedBlock& operator=(OcioTaggedBlock&&) noexcept = default;

	void read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;

private:
};

PSAPI_NAMESPACE_END