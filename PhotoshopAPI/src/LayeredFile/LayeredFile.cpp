#include "LayeredFile.h"

#include "Macros.h"
#include "Layer.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "Struct/TaggedBlock.h"


#include <ranges>
#include <span>


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

	// Build the layer hierarchy
	m_Layers = LayeredFileImpl::buildLayerHierarchy<T>(std::move(document));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<layerVariant<T>> LayeredFileImpl::buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file)
{
	auto& layerRecords = file->m_LayerMaskInfo.m_LayerInfo.m_LayerRecords;

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
	std::vector<LayerRecord>::reverse_iterator layerRecordsIter = layerRecords.rbegin();
	std::vector<layerVariant<T>> root = buildLayerHierarchyRecurse<T>(layerRecords, layerRecordsIter);

	return root;
}

template <typename T>
std::vector<layerVariant<T>> LayeredFileImpl::buildLayerHierarchyRecurse(const std::vector<LayerRecord>& layerRecords, std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator)
{
	std::vector<layerVariant<T>> root;
	while (layerRecordsIterator != layerRecords.rend())
	{
		const auto& layerRecord = *layerRecordsIterator;
		auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();

		auto sectionDivider = additionalLayerInfo.getTaggedBlock<TaggedBlock::LayerSectionDivider>(Enum::TaggedBlockKey::lrSectionDivider);
		if (sectionDivider.has_value())
		{
			if (sectionDivider.value()->m_Type == Enum::SectionDivider::ClosedFolder || sectionDivider.value()->m_Type == Enum::SectionDivider::OpenFolder)
			{
				// Create a new group layer
				GroupLayer<T> groupLayer(layerRecord);

				// Recurse down a level while incrementing to go to the next layer
				groupLayer.m_Layers = buildLayerHierarchyRecurse<T>(layerRecords, ++layerRecordsIterator);

				root.push_back(groupLayer);
			}
			else if (sectionDivider.value()->m_Type == Enum::SectionDivider::BoundingSection)
			{
				// We have reached the end of the current nested section therefore we return the current root object we hold;
				return root;
			}
		}
		// Increment the counter to go to the next layer
		++layerRecordsIterator;
	}
	return root;
}

PSAPI_NAMESPACE_END