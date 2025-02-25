#pragma once

#include "Macros.h"
#include "Layer.h"
#include "Util/Enum.h"
#include "Util/Logger.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "LayeredFile/concepts.h"

#include <vector>
#include <tuple>
#include <optional>

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, this is just how photoshop
/// stores the end of a group as it doesn't really have a concept of nesting otherwise.
/// These are only created on write and not actually stored in the layer hierarchy of the
/// file as we use nested layers to denote hierarchies.
template <typename T>
	requires concepts::bit_depth<T>
struct SectionDividerLayer : Layer<T>
{
protected:
	// Generates an additional LrSectionDivider Tagged Block 
	std::vector<std::shared_ptr<TaggedBlock>> generate_tagged_blocks() override
	{
		auto blockVec = Layer<T>::generate_tagged_blocks();
		LrSectionTaggedBlock sectionBlock{ Enum::SectionDivider::BoundingSection, std::nullopt };
		blockVec.push_back(std::make_shared<LrSectionTaggedBlock>(sectionBlock));

		return blockVec;
	}

public:

	SectionDividerLayer() = default;

	std::tuple<LayerRecord, ChannelImageData> to_photoshop() override
	{
		auto blockVec = this->generate_tagged_blocks();
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
		Layer<T>::generate_empty_channels(channelInfoVec, channelDataVec, Layer<T>::m_ColorMode);

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
			Layer<T>::generate_blending_ranges(),	// Generate some defaults
			std::move(taggedBlocks)
		);

		return std::make_tuple(std::move(lrRecord), ChannelImageData(std::move(channelDataVec)));
	}
};


extern template struct SectionDividerLayer<bpp8_t>;
extern template struct SectionDividerLayer<bpp16_t>;
extern template struct SectionDividerLayer<bpp32_t>;

PSAPI_NAMESPACE_END