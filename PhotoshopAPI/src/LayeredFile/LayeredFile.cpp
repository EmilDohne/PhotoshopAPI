#include "LayeredFile.h"

#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "Struct/TaggedBlock.h"
#include "LayerTypes/Layer.h"
#include "LayerTypes/ImageLayer.h"
#include "LayerTypes/GroupLayer.h"
#include "LayerTypes/AdjustmentLayer.h"
#include "LayerTypes/ArtboardLayer.h"
#include "LayerTypes/SectionDividerLayer.h"
#include "LayerTypes/ShapeLayer.h"
#include "LayerTypes/SmartObjectLayer.h"
#include "LayerTypes/TextLayer.h"

#include <vector>


PSAPI_NAMESPACE_BEGIN

// Instantiate the template types for LayeredFile
template struct LayeredFile<uint8_t>;
template struct LayeredFile<uint16_t>;
template struct LayeredFile<float32_t>;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayeredFile<T>::LayeredFile(std::unique_ptr<PhotoshopFile> file)
{
	// Take ownership of document
	std::unique_ptr<PhotoshopFile> document = std::move(file);

	m_BitDepth = document->m_Header.m_Depth;
	m_ColorMode = document->m_Header.m_ColorMode;
	m_Width = document->m_Header.m_Width;
	m_Height = document->m_Header.m_Height;

	// Build the layer hierarchy
	m_Layers = LayeredFileImpl::buildLayerHierarchy<T>(std::move(document));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayeredFile<T>::LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint8_t>
{
	m_BitDepth = Enum::BitDepth::BD_8;
	m_ColorMode = colorMode;
	m_Width = width;
	m_Height = height;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayeredFile<T>::LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint16_t>
{
	m_BitDepth = Enum::BitDepth::BD_16;
	m_ColorMode = colorMode;
	m_Width = width;
	m_Height = height;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayeredFile<T>::LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, float32_t>
{
	m_BitDepth = Enum::BitDepth::BD_32;
	m_ColorMode = colorMode;
	m_Width = width;
	m_Height = height;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayeredFile<T>::LayeredFile(Enum::BitDepth bitDepth, Enum::ColorMode colorMode, uint64_t width, uint64_t height)
{
	m_BitDepth = bitDepth;
	m_ColorMode = colorMode;
	m_Width = width;
	m_Height = height;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<layerVariant<T>> LayeredFileImpl::buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file)
{
	auto& layerRecords = file->m_LayerMaskInfo.m_LayerInfo.m_LayerRecords;
	auto& channelImageData = file->m_LayerMaskInfo.m_LayerInfo.m_ChannelImageData;

	if (layerRecords.size() != channelImageData.size())
	{
		PSAPI_LOG_ERROR("LayeredFile", "LayerRecords Size does not match channelImageDataSize. File appears to be corrupted")
	}

	// Extract and iterate the layer records. We do this in reverse as Photoshop stores the layers in reverse
	// For example, imagine this layer structure:
	// 
	// Group
	//	ImageLayer
	//
	// Photoshop will actually store the layers like this
	// 
	// Layer Divider
	// ImageLayer
	// Group
	//
	// Layer divider in this case being an empty layer with a 'lsct' tagged block with Type set to 3
	auto layerRecordsIterator = layerRecords.rbegin();
	auto channelImageDataIterator = channelImageData.rbegin();
	std::vector<layerVariant<T>> root = buildLayerHierarchyRecurse<T>(layerRecords, channelImageData, layerRecordsIterator, channelImageDataIterator);

	return root;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<layerVariant<T>> LayeredFileImpl::buildLayerHierarchyRecurse(
	const std::vector<LayerRecord>& layerRecords,
	const std::vector<ChannelImageData>& channelImageData,
	std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator,
	std::vector<ChannelImageData>::reverse_iterator& channelImageDataIterator)
{
	std::vector<layerVariant<T>> root;

	// Iterate the layer records and channelImageData. These are always the same size
	while (layerRecordsIterator != layerRecords.rend() && channelImageDataIterator != channelImageData.rend())
	{
		const auto& layerRecord = *layerRecordsIterator;
		// Get the variant of channelImageDatas and extract the type we have
		const auto& channelImage = *channelImageDataIterator;

		layerVariant<T> layer = identifyLayerType<T>(layerRecord, channelImage);

		if (auto groupLayerPtr = std::get_if<GroupLayer<T>>(&layer))
		{
			// Recurse a level down
			groupLayerPtr->m_Layers = buildLayerHierarchyRecurse<T>(layerRecords, channelImageData, ++layerRecordsIterator, ++channelImageDataIterator);
			root.push_back(*groupLayerPtr);
		}
		else if (auto sectionDividerPtr = std::get_if<SectionDividerLayer<T>>(&layer))
		{
			// We have reached the end of the current nested section therefore we return the current root object we hold;
			return root;
		}
		else
		{
			root.push_back(layer);
		}
		++layerRecordsIterator;
		++channelImageDataIterator;
	}
	return root;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
layerVariant<T> LayeredFileImpl::identifyLayerType(const LayerRecord& layerRecord, const ChannelImageData& channelImageData)
{
	auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();

	// Check for GroupLayer, ArtboardLayer or SectionDividerLayer
	auto sectionDividerTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock::LayerSectionDivider>(Enum::TaggedBlockKey::lrSectionDivider);
	if (sectionDividerTaggedBlock.has_value())
	{
		if (sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::ClosedFolder
			|| sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::OpenFolder)
		{
			// This may actually house not only a group layer, but potentially also an artboard layer which we check for first
			// These are, as of yet, unsupported. Therefore we simply return an empty container
			auto artboardTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock::Generic>(Enum::TaggedBlockKey::lrArtboard);
			if (artboardTaggedBlock.has_value())
			{
				return ArtboardLayer<T>();
			}
			return GroupLayer<T>(layerRecord, channelImageData);
		}
		else if (sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::BoundingSection)
		{
			return SectionDividerLayer<T>();
		}
		// If it is Enum::SectionDivider::Any this is just any other type of layer
		// we do not need to worry about checking for correctness here as the tagged block takes care of that 
	}

	// Check for Text Layers
	auto typeToolTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock::Generic>(Enum::TaggedBlockKey::lrTypeTool);
	if (typeToolTaggedBlock.has_value())
	{
		return TextLayer<T>();
	}

	// Check for Smart Object Layers
	auto smartObjectTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock::Generic>(Enum::TaggedBlockKey::lrSmartObject);
	if (typeToolTaggedBlock.has_value())
	{
		return SmartObjectLayer<T>();
	}

	// Check if it is one of many adjustment layers
	// We do not currently implement these but it would be worth investigating
	{
#define getGenericTaggedBlock additionalLayerInfo.getTaggedBlock<TaggedBlock::Generic>

		auto blackAndWhiteTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjBlackandWhite);
		auto gradientTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjGradient);
		auto invertTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjInvert);
		auto patternFillTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjPattern);
		auto posterizeTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjPosterize);
		auto solidColorTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjSolidColor);
		auto thresholdTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjThreshold);
		auto vibranceTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjVibrance);
		auto brightnessContrastTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjBrightnessContrast);
		auto colorBalanceTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjColorBalance);
		auto colorLookupTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjColorLookup);
		auto channelMixerTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjChannelMixer);
		auto curvesTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjCurves);
		auto gradientMapTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjGradientMap);
		auto exposureTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjExposure);
		auto newHueSatTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjNewHueSat);
		auto oldHueSatTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjOldHueSat);
		auto levelsTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjLevels);
		auto photoFilterTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjPhotoFilter);
		auto selectiveColorTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::adjSelectiveColor);

		if (blackAndWhiteTaggedBlock.has_value()
			|| gradientTaggedBlock.has_value()
			|| invertTaggedBlock.has_value()
			|| patternFillTaggedBlock.has_value()
			|| posterizeTaggedBlock.has_value()
			|| solidColorTaggedBlock.has_value()
			|| thresholdTaggedBlock.has_value()
			|| vibranceTaggedBlock.has_value()
			|| brightnessContrastTaggedBlock.has_value()
			|| colorBalanceTaggedBlock.has_value()
			|| colorLookupTaggedBlock.has_value()
			|| channelMixerTaggedBlock.has_value()
			|| curvesTaggedBlock.has_value()
			|| gradientMapTaggedBlock.has_value()
			|| exposureTaggedBlock.has_value()
			|| newHueSatTaggedBlock.has_value()
			|| oldHueSatTaggedBlock.has_value()
			|| levelsTaggedBlock.has_value()
			|| photoFilterTaggedBlock.has_value()
			|| selectiveColorTaggedBlock.has_value()
			)
		{
			return AdjustmentLayer<T>();
		}
#undef getGenericTaggedBlock
	}
	
	// Now the layer could only be one of two more. A shape or pixel layer (Note files written before CS6 could fail this shape layer check here
	{
#define getGenericTaggedBlock additionalLayerInfo.getTaggedBlock<TaggedBlock::Generic>
		
		{
			auto vecOriginDataTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::vecOriginData);
			auto vecMaskSettingTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::vecMaskSettings);
			auto vecStrokeDataTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::vecStrokeData);
			auto vecStrokeContentDataTaggedBlock = getGenericTaggedBlock(Enum::TaggedBlockKey::vecStrokeContentData);

			if (vecOriginDataTaggedBlock.has_value()
				|| vecMaskSettingTaggedBlock.has_value()
				|| vecStrokeDataTaggedBlock.has_value()
				|| vecStrokeContentDataTaggedBlock.has_value()
				)
			{
				return ShapeLayer<T>();
			}
#undef getGenericTaggedBlock
		}
	}

	return ImageLayer<T>(layerRecord, channelImageData);
}


PSAPI_NAMESPACE_END