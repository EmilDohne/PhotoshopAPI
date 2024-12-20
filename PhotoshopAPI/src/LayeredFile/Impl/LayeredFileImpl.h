#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Util/StringUtil.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/Struct/ICCProfile.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include "LayeredFile/LayerTypes/Layer.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"
#include "LayeredFile/LayerTypes/AdjustmentLayer.h"
#include "LayeredFile/LayerTypes/ArtboardLayer.h"
#include "LayeredFile/LayerTypes/SectionDividerLayer.h"
#include "LayeredFile/LayerTypes/ShapeLayer.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "LayeredFile/LayerTypes/TextLayer.h"

PSAPI_NAMESPACE_BEGIN

// Forward declaration
template <typename T>
struct LayeredFile;

namespace _Impl
{

	/// Identify the type of layer the current layer record represents and return a layerVariant object (std::variant<ImageLayer, GroupLayer ...>)
	/// initialized with the given layer record and corresponding channel image data.
	/// This function was heavily inspired by the psd-tools library as they have the most coherent parsing of this information
	template <typename T>
	std::shared_ptr<Layer<T>> identify_layer_type(LayeredFile<T>& layered_file, LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo)
	{
		// Short ciruit here as we have an image layer for sure
		if (!layerRecord.m_AdditionalLayerInfo.has_value())
		{
			return std::make_shared<ImageLayer<T>>(layerRecord, channelImageData, header);
		}
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
				return std::make_shared<GroupLayer<T>>(layerRecord, channelImageData, header);
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
		auto lrPlacedTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrPlaced);
		auto lrPlacedDataTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrPlacedData);
		if (lrPlacedTaggedBlock.has_value() || lrPlacedDataTaggedBlock.has_value())
		{
			return std::make_shared<SmartObjectLayer<T>>(layered_file, layerRecord, channelImageData, header, globalAdditionalLayerInfo);
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

		return std::make_shared<ImageLayer<T>>(layerRecord, channelImageData, header);
	}


	/// Recursively build a layer hierarchy using the LayerRecords, ChannelImageData and their respective reverse iterators
	/// See comments in build_layer_hierarchy on why we iterate in reverse
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> build_layer_hierarchy_recursive(
		LayeredFile<T>& layered_file,
		std::vector<LayerRecord>& layerRecords,
		std::vector<ChannelImageData>& channelImageData,
		std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator,
		std::vector<ChannelImageData>::reverse_iterator& channelImageDataIterator,
		const FileHeader& header,
		const AdditionalLayerInfo& globalAdditionalLayerInfo
	)
	{
		std::vector<std::shared_ptr<Layer<T>>> root;

		// Iterate the layer records and channelImageData. These are always the same size
		while (layerRecordsIterator != layerRecords.rend() && channelImageDataIterator != channelImageData.rend())
		{
			auto& layerRecord = *layerRecordsIterator;
			auto& channelImage = *channelImageDataIterator;

			std::shared_ptr<Layer<T>> layer = identify_layer_type<T>(layered_file, layerRecord, channelImage, header, globalAdditionalLayerInfo);

			if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(layer))
			{
				// Recurse a level down
				groupLayerPtr->layers() = build_layer_hierarchy_recursive<T>(layered_file, layerRecords, channelImageData, ++layerRecordsIterator, ++channelImageDataIterator, header, globalAdditionalLayerInfo);
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
			try
			{
				++layerRecordsIterator;
				++channelImageDataIterator;
			}
			catch ([[maybe_unused]] const std::exception& ex)
			{
				PSAPI_LOG_ERROR("LayeredFile", "Unhandled exception when trying to decrement the layer iterator");
			}
		}
		return root;
	}


	/// Build the layer hierarchy from a PhotoshopFile object using the Layer and Mask section with its LayerRecords and ChannelImageData subsections;
	/// Returns a vector of nested layer variants which can go to any depth
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> build_layer_hierarchy(LayeredFile<T>& layered_file, std::unique_ptr<PhotoshopFile> file)
	{
		auto* layerRecords = &file->m_LayerMaskInfo.m_LayerInfo.m_LayerRecords;
		auto* channelImageData = &file->m_LayerMaskInfo.m_LayerInfo.m_ChannelImageData;

		if (layerRecords->size() != channelImageData->size())
		{
			PSAPI_LOG_ERROR("LayeredFile", "LayerRecords Size does not match channelImageDataSize. File appears to be corrupted");
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
				PSAPI_LOG_ERROR("LayeredFile", "PhotoshopFile does not seem to contain a Lr16 or Lr32 Tagged block which would hold layer information");
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

		if (file->m_LayerMaskInfo.m_AdditionalLayerInfo)
		{
			std::vector<std::shared_ptr<Layer<T>>> root = build_layer_hierarchy_recursive<T>(
				layered_file,
				*layerRecords,
				*channelImageData,
				layerRecordsIterator,
				channelImageDataIterator,
				file->m_Header,
				file->m_LayerMaskInfo.m_AdditionalLayerInfo.value()
			);
			return root;
		}
		else
		{
			AdditionalLayerInfo tmp{};
			std::vector<std::shared_ptr<Layer<T>>> root = build_layer_hierarchy_recursive<T>(
				layered_file,
				*layerRecords,
				*channelImageData,
				layerRecordsIterator,
				channelImageDataIterator,
				file->m_Header,
				tmp
			);
			return root;
		}

	}


	/// Recursively build a flat layer hierarchy
	template <typename T>
	void generate_flattened_layers_recursive(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, std::vector<std::shared_ptr<Layer<T>>>& flatLayers, bool insertSectionDivider)
	{
		for (const auto& layer : nestedLayers)
		{
			// Recurse down if its a group layer
			if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(layer))
			{
				flatLayers.push_back(layer);
				_Impl::generate_flattened_layers_recursive(groupLayerPtr->layers(), flatLayers, insertSectionDivider);
				// If the layer is a group we actually want to insert a section divider at the end of it. This makes reconstructing the layer
				// hierarchy much easier later on. We dont actually need to give this a name 
				if (insertSectionDivider)
				{
					flatLayers.push_back(std::make_shared<SectionDividerLayer<T>>());
				}
			}
			else
			{
				flatLayers.push_back(layer);
			}
		}
	}


	/// Build a flat layer hierarchy from a nested layer structure and return this vector. Layer order
	/// is not guaranteed
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> generate_flattened_layers(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, bool insertSectionDivider)
	{
		std::vector<std::shared_ptr<Layer<T>>> flatLayers;
		_Impl::generate_flattened_layers_recursive(nestedLayers, flatLayers, insertSectionDivider);

		return flatLayers;
	}


	/// Find a layer based on a separated path and a parent layer. To be called by LayeredFile::findLayer
	template <typename T>
	std::shared_ptr<Layer<T>> find_layer_recursive(std::shared_ptr<Layer<T>> parentLayer, std::vector<std::string> path, int index)
	{
		// We must first check that the parent layer passed in is actually a group layer
		if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
		{
			for (const auto& layerPtr : groupLayerPtr->layers())
			{
				// Get the layer name and recursively check the path
				if (layerPtr->name() == path[index])
				{
					if (index == path.size() - 1)
					{
						// This is the last element and we return the item and propagate it up
						return layerPtr;
					}
					return _Impl::find_layer_recursive(layerPtr, path, index + 1);
				}
			}
			PSAPI_LOG_WARNING("LayeredFile", "Failed to find layer '%s' based on the path", path[index].c_str());
			return nullptr;
		}
		PSAPI_LOG_WARNING("LayeredFile", "Provided parent layer is not a grouplayer and can therefore not have children");
		return nullptr;
	}


	template <typename T>
	bool has_alpha_recursive(std::shared_ptr<Layer<T>> parentLayer)
	{
		// Check if we can recurse down another level into a group of layers
		if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
		{
			for (const auto& layerPtr : groupLayerPtr->layers())
			{
				if (_Impl::has_alpha_recursive(layerPtr))
				{
					return true;
				}
			}
		}
		if (auto imageLayerPtr = std::dynamic_pointer_cast<ImageLayer<T>>(parentLayer))
		{
			if (imageLayerPtr->image_data().contains(Enum::ChannelIDInfo{.id = Enum::ChannelID::Alpha, .index = -1}))
			{
				return true;
			}
		}
		return false;
	}


	template <typename T>
	void set_compression_recursive(std::shared_ptr<Layer<T>> parentLayer, const Enum::Compression compCode)
	{
		// We must first check if we could recurse down another level. We dont check for masks on the 
		// group here yet as we do that further down
		if (const auto groupLayerPtr = std::dynamic_pointer_cast<const GroupLayer<T>>(parentLayer))
		{
			for (const auto& layerPtr : groupLayerPtr->layers())
			{
				layerPtr->set_compression(compCode);
				set_compression_recursive(layerPtr, compCode);
			}
		}
	}


	template <typename T>
	bool layer_in_document_recursive(const std::shared_ptr<Layer<T>> parentLayer, const std::shared_ptr<Layer<T>> layer)
	{
		// We must first check that the parent layer passed in is actually a group layer
		if (const auto groupLayerPtr = std::dynamic_pointer_cast<const GroupLayer<T>>(parentLayer))
		{
			for (const auto& layerPtr : groupLayerPtr->layers())
			{
				if (layerPtr == layer)
				{
					return true;
				}
				if (layer_in_document_recursive(layerPtr, layer))
				{
					return true;
				}
			}
		}
		return false;
	}


	/// Remove a layer from the hierarchy recursively, if a match is found we short circuit and return early
	template <typename T>
	bool remove_layer_recursive(std::shared_ptr<Layer<T>> parentLayer, std::shared_ptr<Layer<T>> layer)
	{
		// We must first check that the parent layer passed in is actually a group layer
		if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
		{
			int index = 0;
			for (const auto& layerPtr : groupLayerPtr->layers())
			{
				if (layerPtr == layer)
				{
					groupLayerPtr->remove_layer(index);
					return true;
				}
				if (remove_layer_recursive(layerPtr, layer))
				{
					return true;
				}
				++index;
			}
		}
		return false;
	}

	// Util functions
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------

	/// Read the document DPI, default to 72 if we cannot read it.
	inline float read_dpi(const PhotoshopFile* file)
	{
		const auto blockPtr = file->m_ImageResources.getResourceBlockView<ResolutionInfoBlock>(Enum::ImageResource::ResolutionInfo);
		if (blockPtr)
		{
			// We dont actually have to do any back and forth conversions here since the value is always stored as DPI and never as 
			// DPCM
			return blockPtr->m_HorizontalRes.getFloat();
		}
		return 72.0f;
	}

	/// Read the ICC profile from the PhotoshopFile, if it doesnt exist we simply initialize an
	/// empty ICC profile
	inline ICCProfile read_icc_profile(const PhotoshopFile* file)
	{
		const auto blockPtr = file->m_ImageResources.getResourceBlockView<ICCProfileBlock>(Enum::ImageResource::ICCProfile);
		if (blockPtr)
		{
			return ICCProfile{ blockPtr->m_RawICCProfile };
		}
		return ICCProfile{};
	}
}

PSAPI_NAMESPACE_END