#pragma once

#include "Macros.h"
#include "Layer.h"
#include "ImageLayer.h"
#include "SectionDividerLayer.h"
#include "ArtboardLayer.h"
#include "ShapeLayer.h"
#include "SmartObjectLayer.h"
#include "TextLayer.h"
#include "AdjustmentLayer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <variant>
#include <memory>

PSAPI_NAMESPACE_BEGIN

// Forward declare LayeredFile here
template <typename T>
struct LayeredFile;


/// \brief Represents a group of layers that may contain nested child layers.
/// 
/// \tparam T The data type for pixel values in layers (e.g., uint8_t, uint16_t, float32_t).
/// 
template <typename T>
struct GroupLayer : public Layer<T>
{
	/// Child layers contained within the group. Note that Layer<T> is polymorphic.
	std::vector<std::shared_ptr<Layer<T>>> m_Layers;

	/// Specifies whether or not the layer is collapsed or open
	bool m_isCollapsed = false;		

	/// \brief Adds a layer to the group, checking for duplicates in the process.
	/// \param layeredFile The layered file containing the group.
	/// \param layer The layer to be added.
	void addLayer(const LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer);

	/// \brief Removes a layer at the given index from the group.
	/// \param index The index of the layer to be removed.
	void removeLayer(const int index);

	/// \brief Removes the specified layer from the group.
	/// \param layer The layer to be removed.
	void removeLayer(std::shared_ptr<Layer<T>>& layer);

	/// \brief Converts the group layer to Photoshop layerRecords and imageData.
	/// \param colorMode The color mode for the conversion.
	/// \param doCopy Set to true to safely keep using the current GroupLayer instance. Advised to keep true unless parsing the whole LayeredFile
	/// \param header The file header for the conversion.
	/// \return A tuple containing layerRecords and imageData.
	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header) override;

	/// \brief Constructs a GroupLayer using layerRecord, channelImageData, and file header.
	/// \param layerRecord The layer record for the group layer.
	/// \param channelImageData The channel image data for the group layer.
	/// \param header The file header for the group layer.
	GroupLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header);

	/// \brief Constructs a GroupLayer with the given layer parameters and collapse state.
	/// \param layerParameters The parameters for the group layer.
	/// \param isCollapsed Specifies whether the group layer is initially collapsed.
	GroupLayer(const Layer<T>::Params& layerParameters, bool isCollapsed = false);


protected:
	/// \brief Generate the tagged blocks necessary for writing the layer
	std::vector<std::shared_ptr<TaggedBlock>> generateTaggedBlocks() override;
};



PSAPI_NAMESPACE_END