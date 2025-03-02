#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Geometry/BoundingBox.h"
#include "Core/Struct/DescriptorStructure.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

PSAPI_NAMESPACE_BEGIN


struct TypeToolTaggedBlock : TaggedBlock
{
	Geometry::BoundingBox<double> m_BoundingBox{};

	std::array<double, 9> m_TransformationMatrix{};

	Descriptors::Descriptor m_TextData{};
	Descriptors::Descriptor m_WarpData{};


	TypeToolTaggedBlock() = default;

	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};

PSAPI_NAMESPACE_END