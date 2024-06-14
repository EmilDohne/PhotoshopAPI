#pragma once

#include "Macros.h"
#include "Enum.h"
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
	LayeredFile(std::unique_ptr<PhotoshopFile> file);

	/// \ingroup Constructors
	/// \brief Constructs an empty LayeredFile object.
	///
	/// Creates a LayeredFile with the specified color mode, width, and height.
	///
	/// \param colorMode The color mode of the file.
	/// \param width The width of the file in pixels.
	/// \param height The height of the file in pixels.
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint8_t>;
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint16_t>;
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, float32_t>;

	/// \brief Finds a layer based on the given path.
	///
	/// The path should be separated by forward slashes, e.g., "Group1/GroupNested/ImageLayer".
	/// Returns a reference to the specific layer if found; otherwise, returns nullptr and issues a warning.
	///
	/// \param path The path to the layer.
	/// \return A shared pointer to the found layer or nullptr.
	std::shared_ptr<Layer<T>> findLayer(std::string path) const;

	/// 
	/// \brief Inserts a layer into the root of the layered file.
	///
	/// If you wish to add a layer to a group, use GroupLayer::addLayer() on a group node retrieved by \ref findLayer().
	///
	/// \param layer The layer to be added.
	void addLayer(std::shared_ptr<Layer<T>> layer);

	/// \brief Moves a layer from its current parent to a new parent node.
	///
	/// If no parentLayer is provided, moves the layer to the root. If the parentLayer is found to be under
	/// the layer it will issue a warning and stop the insertion. I.e. if moving "/Group" to "/Group/GroupNested/"
	/// that would be an illegal move operation as well as moving a layer to itself
	///
	/// \param layer The layer to be moved.
	/// \param parentLayer The new parent layer (if not provided, moves to the root).
	void moveLayer(std::shared_ptr<Layer<T>> layer, std::shared_ptr<Layer<T>> parentLayer = nullptr);

	/// \brief Moves a layer from its current parent to a new parent node.
	///
	/// If no parentLayer is provided, moves the layer to the root. If the parentLayer is found to be under
	/// the layer it will issue a warning and stop the insertion. I.e. if moving "/Group" to "/Group/GroupNested/"
	/// that would be an illegal move operation as well as moving a layer to itself
	///
	/// \param layer The layer to be moved.
	/// \param parentLayer The new parent layer (if not provided, moves to the root).
	void moveLayer(const std::string layer, const std::string parentLayer = "");

	/// \brief Recursively removes a layer from the layer structure.
	///
	/// Iterates the layer structure until the given node is found and then removes it from the tree.
	///
	/// \param layer The layer to be removed.
	void removeLayer(std::shared_ptr<Layer<T>> layer);

	/// \brief Recursively removes a layer from the layer structure.
	///
	/// Iterates the layer structure until the given node is found and then removes it from the tree.
	///
	/// \param layer The layer to be removed.
	void removeLayer(const std::string layer);
	
	/// \brief change the compression codec across all layers and channels
	///
	/// Iterates the layer structure and changes the compression codec for write on all layers.
	/// This is especially useful for e.g. 8-bit files which from Photoshop write with RLE compression
	/// but ZipCompression gives us better ratios
	/// 
	/// \param compCode the compression codec to apply
	void setCompression(const Enum::Compression compCode);

	/// \brief Generates a flat layer stack from either the current root or a given layer.
	///
	/// Use this function to get the most up-to-date flat layer stack based on the given order.
	///
	/// \param layer Optional layer to start the generation from (default is root). If you provide e.g. a group this will only build the below layer tree
	/// \param order The order in which layers should be stacked.
	/// \return The flat layer tree with automatic \ref SectionDividerLayer inserted to mark section ends
	std::vector<std::shared_ptr<Layer<T>>> generateFlatLayers(std::optional<std::shared_ptr<Layer<T>>> layer, const LayerOrder order) const;

	/// \brief Gets the total number of channels in the document.
	///
	/// Excludes mask channels unless ignoreMaskChannels is set to false. Same goes 
	/// for ignoreAlphaChannel
	///
	/// \param ignoreMaskChannels Flag to exclude mask channels from the count.
	/// \param ignoreMaskChannel Flag to exclude the transparency alpha channel from the count.
	/// \return The total number of channels in the document.
	uint16_t getNumChannels(bool ignoreMaskChannels = true, bool ignoreAlphaChannel = true);

	/// \brief Checks if a layer already exists in the nested structure.
	///
	/// \param layer The layer to check for existence.
	/// \return True if the layer exists, false otherwise.
	bool isLayerInDocument(const std::shared_ptr<Layer<T>> layer) const;

	/// \brief read and create a LayeredFile from disk
	///
	/// Simplify the creation of a LayeredFile by abstracting away the step of 
	/// PhotoshopFile -> LayeredFile doing the work internally without exposing the 
	/// PhotoshopFile instance to the user
	/// 
	/// \param filePath the path on disk of the file to be read
	/// \param callback the callback which reports back the current progress and task to the user
	static LayeredFile<T> read(const std::filesystem::path& filePath, ProgressCallback& callback);

	/// \brief read and create a LayeredFile from disk
	///
	/// Simplify the creation of a LayeredFile by abstracting away the step of 
	/// PhotoshopFile -> LayeredFile doing the work internally without exposing the 
	/// PhotoshopFile instance to the user
	/// 
	/// \param filePath the path on disk of the file to be read
	static LayeredFile<T> read(const std::filesystem::path& filePath);

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
	static void write(LayeredFile<T>&& layeredFile, const std::filesystem::path& filePath, ProgressCallback& callback, const bool forceOvewrite = true);

	/// \brief write the LayeredFile instance to disk, consumes and invalidates the instance
	/// 
	/// Simplify the writing of a LayeredFile by abstracting away the step of 
	/// LayeredFile -> PhotoshopFile doing the work internally without exposing the 
	/// PhotoshopFile instance to the user
	/// 
	/// \param layeredFile The LayeredFile to consume, invalidates it
	/// \param filePath The path on disk of the file to be written
	/// \param forceOvewrite Whether to forcefully overwrite the file or fail if the file already exists
	static void write(LayeredFile<T>&& layeredFile, const std::filesystem::path& filePath, const bool forceOvewrite = true);

private:

	/// \brief Checks if moving the child layer to the provided parent layer is valid.
	///
	/// Checks for illegal moves, such as moving a layer to itself or above its current parent.
	///
	/// \param layer The child layer to be moved.
	/// \param parentLayer The new parent layer.
	/// \return True if the move is valid, false otherwise.
	bool isMovingToInvalidHierarchy(const std::shared_ptr<Layer<T>> layer, const std::shared_ptr<Layer<T>> parentLayer);
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
std::unique_ptr<PhotoshopFile> LayeredToPhotoshopFile(LayeredFile<T>&& layeredFile);


namespace LayeredFileImpl
{
	/// Build the layer hierarchy from a PhotoshopFile object using the Layer and Mask section with its LayerRecords and ChannelImageData subsections;
	/// Returns a vector of nested layer variants which can go to any depth
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file);
	/// Recursively build a layer hierarchy using the LayerRecords, ChannelImageData and their respective reverse iterators
	/// See comments in buildLayerHierarchy on why we iterate in reverse
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> buildLayerHierarchyRecurse(
		std::vector<LayerRecord>& layerRecords,
		std::vector<ChannelImageData>& channelImageData,
		std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator,
		std::vector<ChannelImageData>::reverse_iterator& channelImageDataIterator,
		const FileHeader& header
	);

	/// Identify the type of layer the current layer record represents and return a layerVariant object (std::variant<ImageLayer, GroupLayer ...>)
	/// initialized with the given layer record and corresponding channel image data.
	/// This function was heavily inspired by the psd-tools library as they have the most coherent parsing of this information
	template <typename T>
	std::shared_ptr<Layer<T>> generateLayerFromPhotoshopData(LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header);

	/// Build a flat layer hierarchy from a nested layer structure and return this vector. Layer order
	/// is not guaranteed
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> generateFlatLayers(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers);
	
	/// Recursively build a flat layer hierarchy
	template <typename T>
	void generateFlatLayersRecurse(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, std::vector<std::shared_ptr<Layer<T>>>& flatLayers);
	
	/// Find a layer based on a separated path and a parent layer. To be called by LayeredFile::findLayer
	template <typename T>
	std::shared_ptr<Layer<T>> findLayerRecurse(std::shared_ptr<Layer<T>> parentLayer, std::vector<std::string> path, int index);

	template <typename T>
	void getNumChannelsRecurse(std::shared_ptr<Layer<T>> parentLayer, std::set<int16_t>& channelIndices);

	template <typename T>
	void setCompressionRecurse(std::shared_ptr<Layer<T>> parentLayer, const Enum::Compression compCode);

	template <typename T>
	bool isLayerInDocumentRecurse(const std::shared_ptr<Layer<T>> parentLayer, const std::shared_ptr<Layer<T>> layer);

	/// Remove a layer from the hierarchy recursively, if a match is found we short circuit and return early
	template <typename T>
	bool removeLayerRecurse(std::shared_ptr<Layer<T>> parentLayer, std::shared_ptr<Layer<T>> layer);

	// Util functions
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------

	/// Read the ICC profile from the PhotoshopFile, if it doesnt exist we simply initialize an
	/// empty ICC profile
	ICCProfile readICCProfile(const PhotoshopFile* file);

	/// Read the document DPI, default to 72 if we cannot read it.
	float readDPI(const PhotoshopFile* file);
}


PSAPI_NAMESPACE_END