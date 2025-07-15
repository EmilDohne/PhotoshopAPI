#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Util/StringUtil.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/TaggedBlocks/LrSectionTaggedBlock.h"
#include "Core/TaggedBlocks/Lr16TaggedBlock.h"
#include "Core/TaggedBlocks/Lr32TaggedBlock.h"
#include "Core/Struct/ICCProfile.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include "LayeredFile/fwd.h"


PSAPI_NAMESPACE_BEGIN

namespace _Impl
{

	/// Identify the type of layer the current layer record represents and return a layerVariant object (std::variant<ImageLayer, GroupLayer ...>)
	/// initialized with the given layer record and corresponding channel image data.
	/// This function was heavily inspired by the psd-tools library as they have the most coherent parsing of this information
	template <typename T>
	std::shared_ptr<Layer<T>> identify_layer_type(
		LayeredFile<T>& layered_file,
		LayerRecord& layerRecord,
		ChannelImageData& channelImageData,
		const FileHeader& header,
		const AdditionalLayerInfo& globalAdditionalLayerInfo);


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
	);


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
	void generate_flattened_layers_recursive(
		const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, 
		std::vector<std::shared_ptr<Layer<T>>>& flatLayers, 
		bool insertSectionDivider
	);


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
	std::shared_ptr<Layer<T>> find_layer_recursive(std::shared_ptr<Layer<T>> parentLayer, std::vector<std::string> path, int index);


	template <typename T>
	bool has_alpha_recursive(std::shared_ptr<Layer<T>> parentLayer);


	template <typename T>
	void set_compression_recursive(std::shared_ptr<Layer<T>> parentLayer, const Enum::Compression compCode);


	template <typename T>
	bool layer_in_document_recursive(const std::shared_ptr<Layer<T>> parentLayer, const std::shared_ptr<Layer<T>> layer);


	/// Remove a layer from the hierarchy recursively, if a match is found we short circuit and return early
	template <typename T>
	bool remove_layer_recursive(std::shared_ptr<Layer<T>> parentLayer, std::shared_ptr<Layer<T>> layer);


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

	// Validate clipping masks, this checks that layers with clipping masks have a layer below them.
	template <typename T>
	void validate_clipping_masks(LayeredFile<T>& document);

	// Validate the file before writing to disk.
	template <typename T>
	void validate_file(LayeredFile<T>& document)
	{
		validate_clipping_masks(document);
	}
}

PSAPI_NAMESPACE_END