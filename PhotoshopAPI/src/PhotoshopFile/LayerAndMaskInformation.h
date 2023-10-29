#pragma once

#include "../Macros.h"
#include "../Util/Enum.h"
#include "../Util/Struct/File.h"
#include "../Util/Struct/Section.h"
#include "../Util/Struct/ResourceBlock.h"

#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace {
	struct LayerRecord : public FileSection
	{
		bool read(File& document, const uint64_t offset);
	};


	struct GlobalLayerMaskInfo : public FileSection
	{
		bool read(File& document, const uint64_t offset);
	};


	struct ChannelImageData : public FileSection
	{
		bool read(File& document, const uint64_t offset);
	};

	struct AdditionaLayerInfo : public FileSection
	{
		bool read(File& document, const uint64_t offset);
	};

	struct LayerInfo : public FileSection
	{
		std::vector<LayerRecord> m_LayerRecords;
		std::vector<ChannelImageData> m_ChannelImageData;

		bool read(File& document, const uint64_t offset);
	};
}

struct LayerAndMaskInformation : public FileSection
{

	LayerInfo m_LayerInfo;
	GlobalLayerMaskInfo m_GlobalLayerMaskInfo;
	ChannelImageData m_ChannelImageData;
	AdditionaLayerInfo m_AdditionalLayerInfo;

	bool read(File& document, const uint64_t offset);
};



PSAPI_NAMESPACE_END