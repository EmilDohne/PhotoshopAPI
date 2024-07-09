#pragma once

#include "Macros.h"
#include "Enum.h"
#include "StringUtil.h"
#include "Core/Struct/TaggedBlock.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

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

#include <variant>
#include <vector>
#include <set>
#include <filesystem>
#include <memory>


PSAPI_NAMESPACE_BEGIN


/// Enumerator to specify the order of traversal for parsing
enum class LayerOrder
{
	/// Forward in this case refers to us going top to bottom. e.g. if we have the layer structure
	/// \code
	/// Group
	///	  NestedGroup
	///	  Image
	/// \endcode
	/// We would write out the layers starting with 'Group'
	forward,

	/// Reverse in this case refers to us going bottom to top. e.g. if we have the layer structure
	/// \code
	/// Group
	///	  NestedGroup
	///	  Image
	/// \endcode
	/// We would write out the layers starting with 'Image'
	reverse
};



/// Helper Structure for loading an ICC profile from memory of disk. Photoshop will then store
/// the raw bytes of the ICC profile in their ICCProfile ResourceBlock (ID 1039)
struct ICCProfile
{
	/// Initialize an empty ICCProfile
	ICCProfile() : m_Data({}) {};
	/// Initialize the ICCProfile by passing in a raw byte array of an ICC profile
	ICCProfile(std::vector<uint8_t> data) : m_Data(data) {};
	/// Initialize the ICCProfile by loading the path contents from disk
	ICCProfile(const std::filesystem::path& pathToICCFile);

	/// Return a copy of the ICC profile data
	std::vector<uint8_t> getData() const noexcept { return m_Data; };

	/// Return the absolute size of the data
	uint32_t getDataSize() const noexcept { return m_Data.size(); };

private:
	std::vector<uint8_t> m_Data;
};


namespace LayeredFileImpl
{
	/// Identify the type of layer the current layer record represents and return a layerVariant object (std::variant<ImageLayer, GroupLayer ...>)
	/// initialized with the given layer record and corresponding channel image data.
	/// This function was heavily inspired by the psd-tools library as they have the most coherent parsing of this information
	template <typename T>
	std::shared_ptr<Layer<T>> identifyLayerType(LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header)
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

		return std::make_shared<ImageLayer<T>>(layerRecord, channelImageData, header);
	}



	/// Recursively build a layer hierarchy using the LayerRecords, ChannelImageData and their respective reverse iterators
	/// See comments in buildLayerHierarchy on why we iterate in reverse
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> buildLayerHierarchyRecurse(
		std::vector<LayerRecord>& layerRecords,
		std::vector<ChannelImageData>& channelImageData,
		std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator,
		std::vector<ChannelImageData>::reverse_iterator& channelImageDataIterator,
		const FileHeader& header
	)
	{
		std::vector<std::shared_ptr<Layer<T>>> root;

		// Iterate the layer records and channelImageData. These are always the same size
		while (layerRecordsIterator != layerRecords.rend() && channelImageDataIterator != channelImageData.rend())
		{
			auto& layerRecord = *layerRecordsIterator;
			auto& channelImage = *channelImageDataIterator;

			std::shared_ptr<Layer<T>> layer = identifyLayerType<T>(layerRecord, channelImage, header);

			if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(layer))
			{
				// Recurse a level down
				groupLayerPtr->m_Layers = buildLayerHierarchyRecurse<T>(layerRecords, channelImageData, ++layerRecordsIterator, ++channelImageDataIterator, header);
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
			catch (const std::exception& ex)
			{
				PSAPI_UNUSED(ex);
				PSAPI_LOG_ERROR("LayeredFile", "Unhandled exception when trying to decrement the layer iterator");
			}
		}
		return root;
	}


	/// Build the layer hierarchy from a PhotoshopFile object using the Layer and Mask section with its LayerRecords and ChannelImageData subsections;
	/// Returns a vector of nested layer variants which can go to any depth
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file)
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
		std::vector<std::shared_ptr<Layer<T>>> root = buildLayerHierarchyRecurse<T>(*layerRecords, *channelImageData, layerRecordsIterator, channelImageDataIterator, file->m_Header);

		return root;
	}

	/// Recursively build a flat layer hierarchy
	template <typename T>
	void generateFlatLayersRecurse(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, std::vector<std::shared_ptr<Layer<T>>>& flatLayers)
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
				flatLayers.push_back(std::make_shared<SectionDividerLayer<T>>());
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
	std::vector<std::shared_ptr<Layer<T>>> generateFlatLayers(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers)
	{
		std::vector<std::shared_ptr<Layer<T>>> flatLayers;
		LayeredFileImpl::generateFlatLayersRecurse(nestedLayers, flatLayers);

		return flatLayers;
	}
	

	/// Find a layer based on a separated path and a parent layer. To be called by LayeredFile::findLayer
	template <typename T>
	std::shared_ptr<Layer<T>> findLayerRecurse(std::shared_ptr<Layer<T>> parentLayer, std::vector<std::string> path, int index)
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
			PSAPI_LOG_WARNING("LayeredFile", "Failed to find layer '%s' based on the path", path[index].c_str());
			return nullptr;
		}
		PSAPI_LOG_WARNING("LayeredFile", "Provided parent layer is not a grouplayer and can therefore not have children");
		return nullptr;
	}

	template <typename T>
	void getNumChannelsRecurse(std::shared_ptr<Layer<T>> parentLayer, std::set<int16_t>& channelIndices)
	{
		// We must first check if we could recurse down another level. We dont check for masks on the 
		// group here yet as we do that further down
		if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
		{
			for (const auto layerPtr : groupLayerPtr->m_Layers)
			{
				LayeredFileImpl::getNumChannelsRecurse(layerPtr, channelIndices);
			}
		}

		// Check for a pixel mask
		if (parentLayer->m_LayerMask.has_value())
		{
			channelIndices.insert(-2);
		}

		// Deal with Image channels
		if (auto imageLayerPtr = std::dynamic_pointer_cast<ImageLayer<T>>(parentLayer))
		{
			for (const auto& pair : imageLayerPtr->m_ImageData)
			{
				channelIndices.insert(pair.first.index);
			}
		}
	}

	template <typename T>
	void setCompressionRecurse(std::shared_ptr<Layer<T>> parentLayer, const Enum::Compression compCode)
	{
		// We must first check if we could recurse down another level. We dont check for masks on the 
		// group here yet as we do that further down
		if (const auto groupLayerPtr = std::dynamic_pointer_cast<const GroupLayer<T>>(parentLayer))
		{
			for (const auto& layerPtr : groupLayerPtr->m_Layers)
			{
				layerPtr->setCompression(compCode);
				setCompressionRecurse(layerPtr, compCode);
			}
		}
	}

	template <typename T>
	bool isLayerInDocumentRecurse(const std::shared_ptr<Layer<T>> parentLayer, const std::shared_ptr<Layer<T>> layer)
	{
		// We must first check that the parent layer passed in is actually a group layer
		if (const auto groupLayerPtr = std::dynamic_pointer_cast<const GroupLayer<T>>(parentLayer))
		{
			for (const auto& layerPtr : groupLayerPtr->m_Layers)
			{
				if (layerPtr == layer)
				{
					return true;
				}
				if (isLayerInDocumentRecurse(layerPtr, layer))
				{
					return true;
				}
			}
		}
		return false;
	}

	/// Remove a layer from the hierarchy recursively, if a match is found we short circuit and return early
	template <typename T>
	bool removeLayerRecurse(std::shared_ptr<Layer<T>> parentLayer, std::shared_ptr<Layer<T>> layer)
	{
		// We must first check that the parent layer passed in is actually a group layer
		if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
		{
			int index = 0;
			for (const auto& layerPtr : groupLayerPtr->m_Layers)
			{
				if (layerPtr == layer)
				{
					groupLayerPtr->removeLayer(index);
					return true;
				}
				if (removeLayerRecurse(layerPtr, layer))
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

	/// Read the ICC profile from the PhotoshopFile, if it doesnt exist we simply initialize an
	/// empty ICC profile
	ICCProfile readICCProfile(const PhotoshopFile* file);

	/// Read the document DPI, default to 72 if we cannot read it.
	float readDPI(const PhotoshopFile* file);
}


/// \brief Represents a layered file structure.
/// 
/// This struct defines a layered file structure, where each file contains a hierarchy
/// of layers. Layers can be grouped and organized within this structure.
/// 
/// \tparam T The data type used for pixel values in layers (e.g., uint8_t, uint16_t, float32_t).
/// 
template <typename T>
struct LayeredFile
{
	/// The root layers in the file, they may contain multiple levels of sub-layers
	std::vector<std::shared_ptr<Layer<T>>> m_Layers;
	
	/// The ICC Profile associated with the file, this may be empty in which case there will be no colour
	/// profile associated with the file
	ICCProfile m_ICCProfile;

	/// The DPI of the document, this will only change the display unit and wont resize any data
	float m_DotsPerInch = 72.0f;

	/// The bit depth of the file
	Enum::BitDepth m_BitDepth = Enum::BitDepth::BD_8;

	/// The color mode of the file. Currently supports RGB only.
	Enum::ColorMode m_ColorMode = Enum::ColorMode::RGB;	

	/// The width of the file in pixels. Can be up to 30,000 for PSD and up to 300,000 for PSB
	uint64_t m_Width = 0u;

	/// The height of the file in pixels. Can be up to 30,000 for PSD and up to 300,000 for PSB
	uint64_t m_Height = 0u;

	LayeredFile() = default;

	/// \ingroup Constructors
	/// \brief Constructs a LayeredFile instance from a Photoshop file.
	///
	/// Takes ownership of the provided Photoshop file, transferring from a flat layer hierarchy
	/// to a layered file using the lrSectionDivider taggedBlock to identify layer breaks.
	///
	/// \param file The PhotoshopFile to transfer
	LayeredFile(std::unique_ptr<PhotoshopFile> file)
	{
		// Take ownership of document
		std::unique_ptr<PhotoshopFile> document = std::move(file);

		m_BitDepth = document->m_Header.m_Depth;
		m_ColorMode = document->m_Header.m_ColorMode;
		m_Width = document->m_Header.m_Width;
		m_Height = document->m_Header.m_Height;

		// Extract the ICC Profile if it exists on the document, otherwise it will simply be empty
		m_ICCProfile = LayeredFileImpl::readICCProfile(document.get());
		// Extract the DPI from the document, default to 72
		m_DotsPerInch = LayeredFileImpl::readDPI(document.get());

		m_Layers = LayeredFileImpl::buildLayerHierarchy<T>(std::move(document));
		if (m_Layers.size() == 0)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Read an invalid PhotoshopFile as it does not contain any layers. Is the only layer in the scene locked? This is not supported by the PhotoshopAPI");
		}
	}

	/// \ingroup Constructors
	/// \brief Constructs an empty LayeredFile object.
	///
	/// Creates a LayeredFile with the specified color mode, width, and height.
	///
	/// \param colorMode The color mode of the file.
	/// \param width The width of the file in pixels.
	/// \param height The height of the file in pixels.
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint8_t>
	{
		m_BitDepth = Enum::BitDepth::BD_8;
		m_ColorMode = colorMode;
		m_Width = width;
		m_Height = height;
	}
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint16_t>
	{
		m_BitDepth = Enum::BitDepth::BD_16;
		m_ColorMode = colorMode;
		m_Width = width;
		m_Height = height;
	}
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, float32_t>
	{
		m_BitDepth = Enum::BitDepth::BD_32;
		m_ColorMode = colorMode;
		m_Width = width;
		m_Height = height;
	}

	/// \brief Finds a layer based on the given path.
	///
	/// The path should be separated by forward slashes, e.g., "Group1/GroupNested/ImageLayer".
	/// Returns a reference to the specific layer if found; otherwise, returns nullptr and issues a warning.
	///
	/// \param path The path to the layer.
	/// \return A shared pointer to the found layer or nullptr.
	std::shared_ptr<Layer<T>> findLayer(std::string path) const
	{
		PROFILE_FUNCTION();
		std::vector<std::string> segments = splitString(path, '/');
		for (const auto& layer : m_Layers)
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
		PSAPI_LOG_WARNING("LayeredFile", "Unable to find layer path %s", path.c_str());
		return nullptr;
	}

	/// 
	/// \brief Inserts a layer into the root of the layered file.
	///
	/// If you wish to add a layer to a group, use GroupLayer::addLayer() on a group node retrieved by \ref findLayer().
	///
	/// \param layer The layer to be added.
	void addLayer(std::shared_ptr<Layer<T>> layer)
	{
		if (isLayerInDocument(layer))
		{
			PSAPI_LOG_WARNING("LayeredFile", "Cannot insert a layer into the document twice, please use a unique layer. Skipping layer '%s'", layer->m_LayerName.c_str());
			return;
		}
		m_Layers.push_back(layer);
	}

	/// \brief Moves a layer from its current parent to a new parent node.
	///
	/// If no parentLayer is provided, moves the layer to the root. If the parentLayer is found to be under
	/// the layer it will issue a warning and stop the insertion. I.e. if moving "/Group" to "/Group/GroupNested/"
	/// that would be an illegal move operation as well as moving a layer to itself
	///
	/// \param layer The layer to be moved.
	/// \param parentLayer The new parent layer (if not provided, moves to the root).
	void moveLayer(std::shared_ptr<Layer<T>> layer, std::shared_ptr<Layer<T>> parentLayer = nullptr)
	{
		PROFILE_FUNCTION();
		// We must first check that we are not trying to move a layer higher in the hierarchy to lower in the hierarchy 
		// as that would be undefined behaviour. E.g. if we want to move /Group/ to /Group/NestedGroup that wouldnt work
		// since the down stream nodes are dependant on the upstream nodes
		if (parentLayer && isMovingToInvalidHierarchy(layer, parentLayer))
		{
			PSAPI_LOG_WARNING("LayeredFile", "Cannot move layer '%s' under '%s' as that would represent an illegal move operation",
				layer->m_LayerName.c_str(), parentLayer->m_LayerName.c_str());
			return;
		}


		// First we must remove the layer from the hierarchy and then reappend it in a different place
		removeLayer(layer);

		// Insert the layer back, either under the provided parent layer or under the scene root
		if (parentLayer)
		{
			if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
			{
				groupLayerPtr->addLayer(*this, layer);
			}
			else
			{
				PSAPI_LOG_WARNING("LayeredFile", "Parent layer '%s' provided is not a group layer, can only move layers under groups", 
					parentLayer->m_LayerName.c_str());
				return;
			}
		}
		else
		{
			addLayer(layer);
		}
	}

	/// \brief Moves a layer from its current parent to a new parent node.
	///
	/// If no parentLayer is provided, moves the layer to the root. If the parentLayer is found to be under
	/// the layer it will issue a warning and stop the insertion. I.e. if moving "/Group" to "/Group/GroupNested/"
	/// that would be an illegal move operation as well as moving a layer to itself
	///
	/// \param layer The layer to be moved.
	/// \param parentLayer The new parent layer (if not provided, moves to the root).
	void moveLayer(const std::string layer, const std::string parentLayer = "")
	{
		PROFILE_FUNCTION();
		if (parentLayer == "")
		{
			auto layerPtr = findLayer(layer);
			if (!layerPtr) [[unlikely]]
			{
				PSAPI_LOG_ERROR("LayeredFile", "Could not find the layer %s for moveLayer()", layer.c_str());
			}
			moveLayer(layerPtr);
		}
		else
		{
			auto layerPtr = findLayer(layer);
			auto parentLayerPtr = findLayer(parentLayer);
			if (!layerPtr) [[unlikely]]
			{
				PSAPI_LOG_ERROR("LayeredFile", "Could not find the layer %s for moveLayer()", layer.c_str());
			}
			if (!parentLayerPtr) [[unlikely]]
			{
				PSAPI_LOG_ERROR("LayeredFile", "Could not find the parentlayer %s for moveLayer()", parentLayer.c_str());
			}
			moveLayer(layerPtr, parentLayerPtr);
		}
	}

	/// \brief Recursively removes a layer from the layer structure.
	///
	/// Iterates the layer structure until the given node is found and then removes it from the tree.
	///
	/// \param layer The layer to be removed.
	void removeLayer(std::shared_ptr<Layer<T>> layer)
	{
		PROFILE_FUNCTION();
		int index = 0;
		for (auto& sceneLayer : m_Layers)
		{
			// Check if the layers directly in the scene root is the layer we are looking for and remove the layer if that is the case 
			if (sceneLayer == layer)
			{
				m_Layers.erase(m_Layers.begin() + index);
				return;
			}

			// Recurse down and short circuit if we find a match
			if (LayeredFileImpl::removeLayerRecurse(sceneLayer, layer))
			{
				return;
			}
			++index;
		}
	}

	/// \brief Recursively removes a layer from the layer structure.
	///
	/// Iterates the layer structure until the given node is found and then removes it from the tree.
	///
	/// \param layer The layer to be removed.
	void removeLayer(const std::string layer)
	{
		PROFILE_FUNCTION();
		auto layerPtr = findLayer(layer);
		if (!layerPtr) [[unlikely]]
		{
			PSAPI_LOG_ERROR("LayeredFile", "Could not find the layer %s for removeLayer()", layer.c_str());
		}
		removeLayer(layerPtr);
	}
	
	/// \brief change the compression codec across all layers and channels
	///
	/// Iterates the layer structure and changes the compression codec for write on all layers.
	/// This is especially useful for e.g. 8-bit files which from Photoshop write with RLE compression
	/// but ZipCompression gives us better ratios
	/// 
	/// \param compCode the compression codec to apply
	void setCompression(const Enum::Compression compCode)
	{
		for (const auto& documentLayer : m_Layers)
		{
			documentLayer->setCompression(compCode);
			LayeredFileImpl::setCompressionRecurse(documentLayer, compCode);
		}
	}

	/// Generate a flat layer stack from either the current root or (if supplied) from the given layer.
	/// Use this function if you wish to get the most up to date flat layer stack that is in the given
	/// \brief Generates a flat layer stack from either the current root or a given layer.
	///
	/// Use this function to get the most up-to-date flat layer stack based on the given order.
	///
	/// \param layer Optional layer to start the generation from (default is root).
	/// \param order The order in which layers should be stacked.
	/// \return The flat layer tree with automatic \ref SectionDividerLayer inserted to mark section ends
	std::vector<std::shared_ptr<Layer<T>>> generateFlatLayers(std::optional<std::shared_ptr<Layer<T>>> layer, const LayerOrder order) const
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
		PSAPI_LOG_ERROR("LayeredFile", "Invalid layer order specified, only accepts forward or reverse");
		return std::vector<std::shared_ptr<Layer<T>>>();
	}

	/// \brief Gets the total number of channels in the document.
	///
	/// Excludes mask channels unless ignoreMaskChannels is set to false. Same goes 
	/// for ignoreAlphaChannel
	///
	/// \param ignoreMaskChannels Flag to exclude mask channels from the count.
	/// \param ignoreMaskChannel Flag to exclude the transparency alpha channel from the count.
	/// \return The total number of channels in the document.
	uint16_t getNumChannels(bool ignoreMaskChannels = true, bool ignoreAlphaChannel = true)
	{
		std::set<int16_t> channelIndices = {};
		for (const auto& layer : m_Layers)
		{
			LayeredFileImpl::getNumChannelsRecurse(layer, channelIndices);
		}

		uint16_t numChannels = channelIndices.size();
		if (ignoreMaskChannels)
		{
			// Photoshop doesnt consider mask channels for the total amount of channels
			if (channelIndices.contains(-2))
				numChannels -= 1u;
			if (channelIndices.contains(-3))
				numChannels -= 1u;
		}
		if (ignoreAlphaChannel)
		{
			// Photoshop doesnt store the alpha channels in the merged image data section so we must not count it
			if (channelIndices.contains(-1))
				numChannels -= 1u;
		}
		return numChannels;
	}

	/// \brief Checks if a layer already exists in the nested structure.
	///
	/// \param layer The layer to check for existence.
	/// \return True if the layer exists, false otherwise.
	bool isLayerInDocument(const std::shared_ptr<Layer<T>> layer) const
	{
		PROFILE_FUNCTION();
		for (const auto& documentLayer : m_Layers)
		{
			if (documentLayer == layer)
			{
				return true;
			}
			if (LayeredFileImpl::isLayerInDocumentRecurse(documentLayer, layer))
			{
				return true;
			}
		}
		return false;
	}

	/// \brief read and create a LayeredFile from disk
	///
	/// Simplify the creation of a LayeredFile by abstracting away the step of 
	/// PhotoshopFile -> LayeredFile doing the work internally without exposing the 
	/// PhotoshopFile instance to the user
	/// 
	/// \param filePath the path on disk of the file to be read
	/// \param callback the callback which reports back the current progress and task to the user
	static LayeredFile<T> read(const std::filesystem::path& filePath, ProgressCallback& callback)
	{
		auto inputFile = File(filePath);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		psDocumentPtr->read(inputFile, callback);
		LayeredFile<T> layeredFile = { std::move(psDocumentPtr) };
		return layeredFile;
	}

	/// \brief read and create a LayeredFile from disk
	///
	/// Simplify the creation of a LayeredFile by abstracting away the step of 
	/// PhotoshopFile -> LayeredFile doing the work internally without exposing the 
	/// PhotoshopFile instance to the user
	/// 
	/// \param filePath the path on disk of the file to be read
	static LayeredFile<T> read(const std::filesystem::path& filePath)
	{
		ProgressCallback callback{};
		return LayeredFile<T>::read(filePath, callback);
	}

	/// \brief write the LayeredFile instance to disk, consumes and invalidates the instance
	/// 
	/// Simplify the writing of a LayeredFile by abstracting away the step of 
	/// LayeredFile -> PhotoshopFile doing the work internally without exposing the 
	/// PhotoshopFile instance to the user
	/// 
	/// \param layeredFile The LayeredFile to consume, invalidates it
	/// \param filePath The path on disk of the file to be written
	/// \param callback the callback which reports back the current progress and task to the user
	/// \param forceOvewrite Whether to forcefully overwrite the file or fail if the file already exists
	static void write(LayeredFile<T>&& layeredFile, const std::filesystem::path& filePath, ProgressCallback& callback, const bool forceOvewrite = true)
	{
		File::FileParams params = {};
		params.doRead = false;
		params.forceOverwrite = forceOvewrite;
		auto outputFile = File(filePath, params);
		auto psdOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));
		psdOutDocumentPtr->write(outputFile, callback);
	}

	/// \brief write the LayeredFile instance to disk, consumes and invalidates the instance
	/// 
	/// Simplify the writing of a LayeredFile by abstracting away the step of 
	/// LayeredFile -> PhotoshopFile doing the work internally without exposing the 
	/// PhotoshopFile instance to the user
	/// 
	/// \param layeredFile The LayeredFile to consume, invalidates it
	/// \param filePath The path on disk of the file to be written
	/// \param forceOvewrite Whether to forcefully overwrite the file or fail if the file already exists
	static void write(LayeredFile<T>&& layeredFile, const std::filesystem::path& filePath, const bool forceOvewrite = true)
	{
		ProgressCallback callback{};
		LayeredFile<T>::write(std::move(layeredFile), filePath, callback, forceOvewrite);
	}

private:

	/// \brief Checks if moving the child layer to the provided parent layer is valid.
	///
	/// Checks for illegal moves, such as moving a layer to itself or above its current parent.
	///
	/// \param layer The child layer to be moved.
	/// \param parentLayer The new parent layer.
	/// \return True if the move is valid, false otherwise.
	bool isMovingToInvalidHierarchy(const std::shared_ptr<Layer<T>> layer, const std::shared_ptr<Layer<T>> parentLayer)
	{
		// Check if the layer would be moving to one of its descendants which is illegal. Therefore the argument order is reversed
		bool isDescendantOf = LayeredFileImpl::isLayerInDocumentRecurse(parentLayer, layer);
		// We additionally check if the layer is the same as the parent layer as that would also not be allowed
		return isDescendantOf || layer == parentLayer;
	}
};


/// \brief Finds a layer based on the given path and casts it to the given type.
///
/// This function matches LayeredFile<T>::findLayer() but instead of returning a generic layer basetype
/// we return the requested type (if the cast is valid), this is especially useful if the layer type is
/// known ahead of time and is the preferred way of accessing a layer.
/// 
/// Example call:
/// \code{.cpp}
/// LayeredFile<bpp8_t> layeredFile{};	// We assume this is already populated
/// auto imageLayerPtr = findLayerAs<bpp8_t, ImageLayer>("Path/To/ImageLayer", layeredFile);
/// \endcode
/// 
/// The path should be separated by forward slashes, e.g., "Group1/GroupNested/ImageLayer".
/// Returns a reference to the specific layer if found; otherwise, returns nullptr and issues a warning.
/// If we cannot upcast to the specified ptr a warning is raised and nullptr is returned.
///
/// \param path The path to the layer.
/// \param layeredFile the file to search from
/// \return A shared pointer to the found layer or nullptr.
template<typename T, template<typename X> class LayerType>
std::shared_ptr<LayerType<T>> findLayerAs(const std::string path, const LayeredFile<T>& layeredFile)
{
	auto basePtr = layeredFile.findLayer(path);
	auto downcastedPtr = std::dynamic_pointer_cast<LayerType<T>>(basePtr);
	if (downcastedPtr)
	{
		return downcastedPtr;
	}
	PSAPI_LOG_WARNING("LayeredFile", "Unable to cast Layer pointer to requested type, aborting");
	return nullptr;
}


/// \brief Converts a layeredFile into a PhotoshopFile, taking ownership of and invalidating any data
/// 
/// \note This will not fill any specific TaggedBlocks or ResourceBlocks beyond what is required
/// to create the layer structure.
template <typename T>
std::unique_ptr<PhotoshopFile> LayeredToPhotoshopFile(LayeredFile<T>&& layeredFile)
{
	PROFILE_FUNCTION();
	FileHeader header = generateHeader<T>(layeredFile);
	ColorModeData colorModeData = generateColorModeData<T>(layeredFile);
	ImageResources imageResources = generateImageResources<T>(layeredFile);
	LayerAndMaskInformation lrMaskInfo = generateLayerMaskInfo<T>(layeredFile, header);
	ImageData imageData = ImageData(layeredFile.getNumChannels(true, true));	// Ignore any mask or alpha channels

	return std::make_unique<PhotoshopFile>(header, colorModeData, std::move(imageResources), std::move(lrMaskInfo), imageData);
}


PSAPI_NAMESPACE_END