#include "SectionDividerLayer.h"

#include "Macros.h"
#include "Logger.h"
#include "Enum.h"
#include "Struct/TaggedBlock.h"

#include <vector>
#include <tuple>
#include <optional>


PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for SectionDividerLayer
template struct SectionDividerLayer<uint8_t>;
template struct SectionDividerLayer<uint16_t>;
template struct SectionDividerLayer<float32_t>;

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, ChannelImageData> SectionDividerLayer<T>::toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy)
{
	std::vector<LayerRecords::ChannelInformation> channelInfo{};	// Just have this be empty
	ChannelImageData channelData{};
	
	LrSectionTaggedBlock sectionBlock{ Enum::SectionDivider::BoundingSection, std::nullopt };
	std::vector<std::shared_ptr<TaggedBlock>> blockVec;
	blockVec.push_back(std::make_shared<TaggedBlock>(sectionBlock));
	TaggedBlockStorage blockStorage(blockVec);
	AdditionalLayerInfo lrInfo(blockStorage);

	LayerRecord lrRecord(
		PascalString("", 4u),	// Photoshop does sometimes explicitly write out the name such as '</Group 1>' to indicate what it belongs to 
		0,		// top
		0,		// left
		0,		// bottom
		0,		// right
		0u,		// Number of channels, photoshop does appear to actually write out all the channels with 0 length, we will see later if that is a requirement
		channelInfo,
		Enum::BlendMode::Normal,
		255u,	// Opacity
		0u,		// Clipping
		LayerRecords::BitFlags{},
		std::nullopt,
		Layer<T>::generateBlendingRanges(colorMode),	// Generate some defaults
		std::make_optional(lrInfo)
	);

	return std::make_tuple(lrRecord, channelData);
}


PSAPI_NAMESPACE_END