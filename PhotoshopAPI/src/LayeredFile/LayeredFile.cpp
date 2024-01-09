#include "LayeredFile.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "Macros.h"
#include "StringUtil.h"
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

#include "LayeredFile/Util/GenerateHeader.h"
#include "LayeredFile/Util/GenerateColorModeData.h"
#include "LayeredFile/Util/GenerateImageResources.h"
#include "LayeredFile/Util/GenerateLayerMaskInfo.h"


#include <vector>
#include <span>
#include <variant>
#include <memory>
#include <algorithm>


PSAPI_NAMESPACE_BEGIN

// Instantiate the template types for LayeredFile
template struct LayeredFile<uint8_t>;
template struct LayeredFile<uint16_t>;
template struct LayeredFile<float32_t>;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::unique_ptr<PhotoshopFile> LayeredFile<T>::toPhotoshopFile()
{
	FileHeader header = generateHeader<T>(*this);
	ColorModeData colorModeData = generateColorModeData<T>(*this);
	ImageResources imageResources = generateImageResources<T>(*this);
	LayerAndMaskInformation lrMaskInfo = generateLayerMaskInfo<T>(*this, header);

 	return std::make_unique<PhotoshopFile>(header, colorModeData, imageResources, std::move(lrMaskInfo));
}


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

	// Build the layer hierarchy, both nested and flat for easy traversal
	m_Layers = LayeredFileImpl::buildLayerHierarchy<T>(std::move(document));
	m_FlatLayers = LayeredFileImpl::generateFlatLayers<T>(m_Layers);
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
std::shared_ptr<Layer<T>> LayeredFile<T>::findLayer(std::string path) const
{
	std::vector<std::string> segments = splitString(path, '/');
	for (const auto layer : m_Layers)
	{
		// Get the layer name and recursively check the path
		if (layer->m_LayerName == segments[0])
		{
			// This is a simple path with no nested layers
			if (segments.size() == 1)
			{
				return layer;
			}
			// Pass an index of one as we already found the first layer
			return LayeredFileImpl::findLayerRecurse(layer, segments, 1);
		}
	}
	PSAPI_LOG_WARNING("LayeredFile", "Unable to find layer path %s", path.c_str())
	return nullptr;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<std::shared_ptr<Layer<T>>> LayeredFile<T>::generateFlatLayers(std::optional<std::shared_ptr<Layer<T>>> layer, const LayerOrder order) const
{
	if (order == LayerOrder::forward)
	{
		if (layer.has_value())
		{
			std::vector<std::shared_ptr<Layer<T>>> layerVec;
			layerVec.push_back(layer.value());
			return LayeredFileImpl::generateFlatLayers(layerVec);
		}
		return LayeredFileImpl::generateFlatLayers(m_Layers);
	}
	else if (order == LayerOrder::reverse)
	{
		if (layer.has_value())
		{
			std::vector<std::shared_ptr<Layer<T>>> layerVec;
			layerVec.push_back(layer.value());
			std::vector<std::shared_ptr<Layer<T>>> flatLayers = LayeredFileImpl::generateFlatLayers(layerVec);
			std::reverse(flatLayers.begin(), flatLayers.end());
			return flatLayers;
		}
		std::vector<std::shared_ptr<Layer<T>>> flatLayers = LayeredFileImpl::generateFlatLayers(m_Layers);
		std::reverse(flatLayers.begin(), flatLayers.end());
		return flatLayers;
	}
	PSAPI_LOG_ERROR("LayeredFile", "Invalid layer order specified, only accepts forward or reverse")
		return std::vector<std::shared_ptr<Layer<T>>>();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<std::shared_ptr<Layer<T>>> LayeredFileImpl::buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file)
{
	auto* layerRecords = &file->m_LayerMaskInfo.m_LayerInfo.m_LayerRecords;
	auto* channelImageData = &file->m_LayerMaskInfo.m_LayerInfo.m_ChannelImageData;

	if (layerRecords->size() != channelImageData->size())
	{
		PSAPI_LOG_ERROR("LayeredFile", "LayerRecords Size does not match channelImageDataSize. File appears to be corrupted")
	}

	// 16 and 32 bit files store their layer records in the additional layer information section. We must therefore overwrite our previous results
	if (sizeof(T) >= 2u && file->m_LayerMaskInfo.m_AdditionalLayerInfo.has_value())
	{
		const AdditionalLayerInfo& additionalLayerInfo = file->m_LayerMaskInfo.m_AdditionalLayerInfo.value();
		auto lr16TaggedBlock = additionalLayerInfo.getTaggedBlock<Lr16TaggedBlock>(Enum::TaggedBlockKey::Lr16);
		auto lr32TaggedBlock = additionalLayerInfo.getTaggedBlock<Lr32TaggedBlock>(Enum::TaggedBlockKey::Lr32);
		if (lr16TaggedBlock.has_value())
		{
			auto& block = lr16TaggedBlock.value();
			layerRecords = &block->m_Data.m_LayerRecords;
			channelImageData = &block->m_Data.m_ChannelImageData;
		}
		else if (lr32TaggedBlock.has_value())
		{
			auto& block = lr32TaggedBlock.value();
			layerRecords = &block->m_Data.m_LayerRecords;
			channelImageData = &block->m_Data.m_ChannelImageData;
		}
		else
		{
			PSAPI_LOG_ERROR("LayeredFile", "PhotoshopFile does not seem to contain a Lr16 or Lr32 Tagged block which would hold layer information")
		}
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
	auto layerRecordsIterator = layerRecords->rbegin();
	auto channelImageDataIterator = channelImageData->rbegin();
	std::vector<std::shared_ptr<Layer<T>>> root = buildLayerHierarchyRecurse<T>(*layerRecords, *channelImageData, layerRecordsIterator, channelImageDataIterator);

	return root;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<std::shared_ptr<Layer<T>>> LayeredFileImpl::buildLayerHierarchyRecurse(
	std::vector<LayerRecord>& layerRecords,
	std::vector<ChannelImageData>& channelImageData,
	std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator,
	std::vector<ChannelImageData>::reverse_iterator& channelImageDataIterator)
{
	std::vector<std::shared_ptr<Layer<T>>> root;

	// Iterate the layer records and channelImageData. These are always the same size
	while (layerRecordsIterator != layerRecords.rend() && channelImageDataIterator != channelImageData.rend())
	{
		auto& layerRecord = *layerRecordsIterator;
		// Get the variant of channelImageDatas and extract the type we have
		auto& channelImage = *channelImageDataIterator;

		std::shared_ptr<Layer<T>> layer = identifyLayerType<T>(layerRecord, channelImage);

		if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(layer))
		{
			// Recurse a level down
			groupLayerPtr->m_Layers = buildLayerHierarchyRecurse<T>(layerRecords, channelImageData, ++layerRecordsIterator, ++channelImageDataIterator);
			root.push_back(groupLayerPtr);
		}
		else if (auto sectionLayerPtr = std::dynamic_pointer_cast<SectionDividerLayer<T>>(layer))
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
std::vector<std::shared_ptr<Layer<T>>> LayeredFileImpl::generateFlatLayers(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers)
{
	std::vector<std::shared_ptr<Layer<T>>> flatLayers;
	LayeredFileImpl::generateFlatLayersRecurse(nestedLayers, flatLayers);

	return flatLayers;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void LayeredFileImpl::generateFlatLayersRecurse(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, std::vector<std::shared_ptr<Layer<T>>>& flatLayers)
{
	for (const auto& layer : nestedLayers)
	{
		// Recurse down if its a group layer
		if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(layer))
		{
			flatLayers.push_back(layer);
			LayeredFileImpl::generateFlatLayersRecurse(groupLayerPtr->m_Layers, flatLayers);
			// If the layer is a group we actually want to insert a section divider at the end of it. This makes reconstructing the layer
			// hierarchy much easier later on. We dont actually need to give this a name 
			flatLayers.push_back(std::make_shared<SectionDividerLayer<T>>(SectionDividerLayer<T>{}));
		}
		else
		{
			flatLayers.push_back(layer);
		}
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::shared_ptr<Layer<T>> LayeredFileImpl::identifyLayerType(LayerRecord& layerRecord, ChannelImageData& channelImageData)
{
	const AdditionalLayerInfo& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();

	// Check for GroupLayer, ArtboardLayer or SectionDividerLayer
	auto sectionDividerTaggedBlock = additionalLayerInfo.getTaggedBlock<LrSectionTaggedBlock>(Enum::TaggedBlockKey::lrSectionDivider);
	if (sectionDividerTaggedBlock.has_value())
	{
		if (sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::ClosedFolder
			|| sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::OpenFolder)
		{
			// This may actually house not only a group layer, but potentially also an artboard layer which we check for first
			// These are, as of yet, unsupported. Therefore we simply return an empty container
			auto artboardTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrArtboard);
			if (artboardTaggedBlock.has_value())
			{
				return std::make_shared<ArtboardLayer<T>>();
			}
			return std::make_shared<GroupLayer<T>>(layerRecord, channelImageData);
		}
		else if (sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::BoundingSection)
		{
			return std::make_shared<SectionDividerLayer<T>>();
		}
		// If it is Enum::SectionDivider::Any this is just any other type of layer
		// we do not need to worry about checking for correctness here as the tagged block takes care of that 
	}

	// Check for Text Layers
	auto typeToolTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	if (typeToolTaggedBlock.has_value())
	{
		return std::make_shared<TextLayer<T>>();
	}

	// Check for Smart Object Layers
	auto smartObjectTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrSmartObject);
	if (typeToolTaggedBlock.has_value())
	{
		return std::make_shared<SmartObjectLayer<T>>();
	}

	// Check if it is one of many adjustment layers
	// We do not currently implement these but it would be worth investigating
	{
#define getGenericTaggedBlock additionalLayerInfo.getTaggedBlock<TaggedBlock>

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
			return std::make_shared<AdjustmentLayer<T>>();
		}
#undef getGenericTaggedBlock
	}
	
	// Now the layer could only be one of two more. A shape or pixel layer (Note files written before CS6 could fail this shape layer check here
	{
#define getGenericTaggedBlock additionalLayerInfo.getTaggedBlock<TaggedBlock>
		
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
				return std::make_shared<ShapeLayer<T>>();
			}
#undef getGenericTaggedBlock
		}
	}

	return std::make_shared<ImageLayer<T>>(layerRecord, channelImageData);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::shared_ptr<Layer<T>> LayeredFileImpl::findLayerRecurse(std::shared_ptr<Layer<T>> parentLayer, std::vector<std::string> path, int index)
{
	// We must first check that the parent layer passed in is actually a group layer
	if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
	{
		for (const auto layerPtr : groupLayerPtr->m_Layers)
		{
			// Get the layer name and recursively check the path
			if (layerPtr->m_LayerName == path[index])
			{
				if (index == path.size() - 1)
				{
					// This is the last element and we return the item and propagate it up
					return layerPtr;
				}
				return LayeredFileImpl::findLayerRecurse(layerPtr, path, index+1);
			}
		}
		PSAPI_LOG_WARNING("LayeredFile", "Failed to find layer '%s' based on the path", path[index].c_str())
		return nullptr;
	}
	PSAPI_LOG_WARNING("LayeredFile", "Provided parent layer is not a grouplayer and can therefore not have children")
	return nullptr;
}




PSAPI_NAMESPACE_END