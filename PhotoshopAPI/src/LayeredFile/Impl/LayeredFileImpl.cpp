#include "LayeredFileImpl.h"

#include "Macros.h"

#include "LayeredFile/LayerTypes/AdjustmentLayer.h"
#include "LayeredFile/LayerTypes/ArtboardLayer.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "LayeredFile/LayerTypes/SectionDividerLayer.h"
#include "LayeredFile/LayerTypes/ShapeLayer.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "LayeredFile/LayerTypes/TextLayer.h"

PSAPI_NAMESPACE_BEGIN

namespace _Impl
{

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::shared_ptr<Layer<T>> identify_layer_type(LayeredFile<T>& layered_file, LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo)
	{
		if (!layerRecord.m_AdditionalLayerInfo.has_value())
		{
			return std::make_shared<ImageLayer<T>>(layerRecord, channelImageData, header);
		}
		const AdditionalLayerInfo& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();

		auto sectionDividerTaggedBlock = additionalLayerInfo.getTaggedBlock<LrSectionTaggedBlock>(Enum::TaggedBlockKey::lrSectionDivider);
		if (sectionDividerTaggedBlock.has_value())
		{
			if (sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::ClosedFolder ||
				sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::OpenFolder)
			{
				auto artboardTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrArtboard);
				if (artboardTaggedBlock.has_value())
				{
					return std::make_shared<ArtboardLayer<T>>(layerRecord, channelImageData, header);
				}
				return std::make_shared<GroupLayer<T>>(layerRecord, channelImageData, header);
			}
			else if (sectionDividerTaggedBlock.value()->m_Type == Enum::SectionDivider::BoundingSection)
			{
				return std::make_shared<SectionDividerLayer<T>>();
			}
		}

		auto typeToolTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
		if (typeToolTaggedBlock.has_value())
		{
			return std::make_shared<TextLayer<T>>(layerRecord, channelImageData, header);
		}

		auto lrPlacedTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrPlaced);
		auto lrPlacedDataTaggedBlock = additionalLayerInfo.getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrPlacedData);
		if (lrPlacedTaggedBlock.has_value() || lrPlacedDataTaggedBlock.has_value())
		{
			return std::make_shared<SmartObjectLayer<T>>(layered_file, layerRecord, channelImageData, header, globalAdditionalLayerInfo);
		}

#define CHECK_TAGGED_BLOCK(tag) additionalLayerInfo.getTaggedBlock<TaggedBlock>(tag).has_value()
		if (CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjBlackandWhite) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjGradient) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjInvert) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjPattern) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjPosterize) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjSolidColor) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjThreshold) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjVibrance) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjBrightnessContrast) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjColorBalance) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjColorLookup) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjChannelMixer) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjCurves) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjGradientMap) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjExposure) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjNewHueSat) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjOldHueSat) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjLevels) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjPhotoFilter) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::adjSelectiveColor))
		{
			return std::make_shared<AdjustmentLayer<T>>(layerRecord, channelImageData, header);
		}

		if (CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::vecOriginData) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::vecMaskSettings) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::vecStrokeData) ||
			CHECK_TAGGED_BLOCK(Enum::TaggedBlockKey::vecStrokeContentData))
		{
			return std::make_shared<ShapeLayer<T>>(layerRecord, channelImageData, header);
		}
#undef CHECK_TAGGED_BLOCK

		return std::make_shared<ImageLayer<T>>(layerRecord, channelImageData, header);
	}


	// Explicit template instantiations
	template std::shared_ptr<Layer<uint8_t>> identify_layer_type(LayeredFile<uint8_t>&, LayerRecord&, ChannelImageData&, const FileHeader&, const AdditionalLayerInfo&);
	template std::shared_ptr<Layer<uint16_t>> identify_layer_type(LayeredFile<uint16_t>&, LayerRecord&, ChannelImageData&, const FileHeader&, const AdditionalLayerInfo&);
	template std::shared_ptr<Layer<float>> identify_layer_type(LayeredFile<float>&, LayerRecord&, ChannelImageData&, const FileHeader&, const AdditionalLayerInfo&);


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
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

	// Explicit template instantiations
	template std::vector<std::shared_ptr<Layer<uint8_t>>> build_layer_hierarchy_recursive(
		LayeredFile<uint8_t>&,
		std::vector<LayerRecord>&,
		std::vector<ChannelImageData>&,
		std::vector<LayerRecord>::reverse_iterator&,
		std::vector<ChannelImageData>::reverse_iterator&,
		const FileHeader&,
		const AdditionalLayerInfo&
	);
	template std::vector<std::shared_ptr<Layer<uint16_t>>> build_layer_hierarchy_recursive(
		LayeredFile<uint16_t>&,
		std::vector<LayerRecord>&,
		std::vector<ChannelImageData>&,
		std::vector<LayerRecord>::reverse_iterator&,
		std::vector<ChannelImageData>::reverse_iterator&,
		const FileHeader&,
		const AdditionalLayerInfo&
	); 
	template std::vector<std::shared_ptr<Layer<float>>> build_layer_hierarchy_recursive(
		LayeredFile<float>&,
		std::vector<LayerRecord>&,
		std::vector<ChannelImageData>&,
		std::vector<LayerRecord>::reverse_iterator&,
		std::vector<ChannelImageData>::reverse_iterator&,
		const FileHeader&,
		const AdditionalLayerInfo&
	);


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void generate_flattened_layers_recursive(
		const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, 
		std::vector<std::shared_ptr<Layer<T>>>& flatLayers, 
		bool insertSectionDivider
	)
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

	// Explicit template instantiations
	template void generate_flattened_layers_recursive(
		const std::vector<std::shared_ptr<Layer<uint8_t>>>&,
		std::vector<std::shared_ptr<Layer<uint8_t>>>&,
		bool
	);
	template void generate_flattened_layers_recursive(
		const std::vector<std::shared_ptr<Layer<uint16_t>>>&,
		std::vector<std::shared_ptr<Layer<uint16_t>>>&,
		bool
	);
	template void generate_flattened_layers_recursive(
		const std::vector<std::shared_ptr<Layer<float>>>&,
		std::vector<std::shared_ptr<Layer<float>>>&,
		bool
	);


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
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
	// Explicit template instantiations
	template std::shared_ptr<Layer<uint8_t>> find_layer_recursive(std::shared_ptr<Layer<uint8_t>> parentLayer, std::vector<std::string> path, int index);
	template std::shared_ptr<Layer<uint16_t>> find_layer_recursive(std::shared_ptr<Layer<uint16_t>> parentLayer, std::vector<std::string> path, int index);
	template std::shared_ptr<Layer<float>> find_layer_recursive(std::shared_ptr<Layer<float>> parentLayer, std::vector<std::string> path, int index);

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
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
			constexpr auto s_alpha_idx = Enum::ChannelIDInfo(Enum::ChannelID::Alpha, -1);
			if (imageLayerPtr->get_storage().contains(s_alpha_idx))
			{
				return true;
			}
		}
		if (auto smartObjectLayerPtr = std::dynamic_pointer_cast<SmartObjectLayer<T>>(parentLayer))
		{
			// We just implicitly assume these have an alpha channel as we currently always add one
			return true;
		}
		return false;
	}
	// Explicit template instantiations
	template bool has_alpha_recursive(std::shared_ptr<Layer<uint8_t>> parentLayer);
	template bool has_alpha_recursive(std::shared_ptr<Layer<uint16_t>> parentLayer);
	template bool has_alpha_recursive(std::shared_ptr<Layer<float>> parentLayer);

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void set_compression_recursive(std::shared_ptr<Layer<T>> parentLayer, const Enum::Compression compCode)
	{
		// We must first check if we could recurse down another level. We dont check for masks on the 
		// group here yet as we do that further down
		if (const auto groupLayerPtr = std::dynamic_pointer_cast<const GroupLayer<T>>(parentLayer))
		{
			for (const auto& layerPtr : groupLayerPtr->layers())
			{
				layerPtr->set_write_compression(compCode);
				set_compression_recursive(layerPtr, compCode);
			}
		}
	}
	// Explicit template instantiations
	template void set_compression_recursive(std::shared_ptr<Layer<uint8_t>> parentLayer, const Enum::Compression compCode);
	template void set_compression_recursive(std::shared_ptr<Layer<uint16_t>> parentLayer, const Enum::Compression compCode);
	template void set_compression_recursive(std::shared_ptr<Layer<float>> parentLayer, const Enum::Compression compCode);

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
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
	// Explicit template instantiations
	template bool layer_in_document_recursive(const std::shared_ptr<Layer<uint8_t>> parentLayer, const std::shared_ptr<Layer<uint8_t>> layer);
	template bool layer_in_document_recursive(const std::shared_ptr<Layer<uint16_t>> parentLayer, const std::shared_ptr<Layer<uint16_t>> layer);
	template bool layer_in_document_recursive(const std::shared_ptr<Layer<float>> parentLayer, const std::shared_ptr<Layer<float>> layer);


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
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

	// Explicit template instantiations
	template bool remove_layer_recursive(std::shared_ptr<Layer<uint8_t>> parentLayer, std::shared_ptr<Layer<uint8_t>> layer);
	template bool remove_layer_recursive(std::shared_ptr<Layer<uint16_t>> parentLayer, std::shared_ptr<Layer<uint16_t>> layer);
	template bool remove_layer_recursive(std::shared_ptr<Layer<float>> parentLayer, std::shared_ptr<Layer<float>> layer);



	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void validate_clipping_masks(LayeredFile<T>& document)
	{
		std::function<void(const std::vector<std::shared_ptr<Layer<T>>>&)> validate_scope;
		validate_scope = [&](const std::vector<std::shared_ptr<Layer<T>>>& layer_ptrs) -> void
			{
				size_t layer_index = 0;
				for (const auto& layer : layer_ptrs)
				{
					if (auto group_layer = std::dynamic_pointer_cast<GroupLayer<T>>(layer))
					{
						if (group_layer->clipping_mask())
						{
							PSAPI_LOG_WARNING(
								"Validation", "Group Layer '%s' has a clipping mask which will be ignored by photoshop.",
								group_layer->name().c_str()
							);
						}
						validate_scope(group_layer->layers());
					}

					// Photoshop does not allow clipping masks as the last layer in the scope (i.e. in a group).
					if (layer->clipping_mask() && layer_index == layer_ptrs.size() - 1)
					{
						PSAPI_LOG_WARNING(
							"Validation", "Layer '%s' has a clipping mask which will lead to it being invisible because"
							" it is the last layer within its scope (e.g. group/root).",
							layer->name().c_str()
						);
					}

					++layer_index;
				}
			};

		validate_scope(document.layers());
	}

	// Explicit template instantiations
	template void validate_clipping_masks(LayeredFile<uint8_t>& document);
	template void validate_clipping_masks(LayeredFile<uint16_t>& document);
	template void validate_clipping_masks(LayeredFile<float>& document);

} // _Impl 

PSAPI_NAMESPACE_END