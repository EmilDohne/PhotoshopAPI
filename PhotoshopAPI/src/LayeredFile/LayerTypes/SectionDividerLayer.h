#pragma once

#include "Macros.h"
#include "Layer.h"
#include "Enum.h"
#include "Logger.h"
#include "Core/Struct/TaggedBlock.h"

#include <vector>
#include <tuple>
#include <optional>

PSAPI_NAMESPACE_BEGIN

// This struct holds no data, we just use it to identify its type
// We dont actually even store these and only use them when going from
// flat -> nested
template <typename T>
struct SectionDividerLayer : Layer<T>
{
protected:
	// Generates an additional LrSectionDivider Tagged Block 
	std::vector<std::shared_ptr<TaggedBlock>> generateTaggedBlocks() override
	{
		auto blockVec = Layer<T>::generateTaggedBlocks();
		LrSectionTaggedBlock sectionBlock{ Enum::SectionDivider::BoundingSection, std::nullopt };
		blockVec.push_back(std::make_shared<LrSectionTaggedBlock>(sectionBlock));

		return blockVec;
	}

public:

	SectionDividerLayer() = default;

	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, [[maybe_unused]] const FileHeader& header) override
	{
		auto blockVec = this->generateTaggedBlocks();
		std::optional<AdditionalLayerInfo> taggedBlocks = std::nullopt;
		if (blockVec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { blockVec };
			taggedBlocks.emplace(blockStorage);
		}

		// Initialize the channelInfo. Note that if the data is to be compressed the channel size gets update
		// again later
		std::vector<LayerRecords::ChannelInformation> channelInfoVec;
		std::vector<std::unique_ptr<ImageChannel>> channelDataVec;

		// Applications such as krita expect empty channels to be in-place for the given colormode
		// to actually parse the file. 
		Layer<T>::generateEmptyChannels(channelInfoVec, channelDataVec, colorMode);

		LayerRecord lrRecord(
			PascalString("", 4u),	// Photoshop does sometimes explicitly write out the name such as '</Group 1>' to indicate what it belongs to 
			0,		// top
			0,		// left
			0,		// bottom
			0,		// right
			static_cast<uint16_t>(channelInfoVec.size()),
			channelInfoVec,
			Enum::BlendMode::Normal,
			255u,	// Opacity
			0u,		// Clipping
			LayerRecords::BitFlags{},
			std::nullopt,
			Layer<T>::generateBlendingRanges(),	// Generate some defaults
			std::move(taggedBlocks)
		);

		return std::make_tuple(std::move(lrRecord), ChannelImageData(std::move(channelDataVec)));
	}
};


PSAPI_NAMESPACE_END