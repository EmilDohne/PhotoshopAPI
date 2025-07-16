#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Util/StringUtil.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include "LayeredFile/Impl/LayeredFileImpl.h"
#include "fwd.h"
#include "concepts.h"

#include "LinkedData/LinkedLayerData.h"

#include "LayeredFile/Util/GenerateHeader.h"
#include "LayeredFile/Util/GenerateColorModeData.h"
#include "LayeredFile/Util/GenerateImageResources.h"
#include "LayeredFile/Util/GenerateLayerMaskInfo.h"
#include "LayeredFile/Util/ClearLinkedLayers.h"

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



/// \brief Represents a layered file structure.
/// 
/// This struct defines a layered file structure, where each file contains a hierarchy
/// of layers. Layers can be grouped and organized within this structure.
/// 
/// \tparam T The data type used for pixel values in layers (e.g., uint8_t, uint16_t, float32_t).
/// 
template <typename T>
	requires concepts::bit_depth<T>
struct LayeredFile
{

	/// layers The Files' child layers
	std::vector<std::shared_ptr<Layer<T>>>& layers() noexcept { return m_Layers; }
	void layers(std::vector<std::shared_ptr<Layer<T>>> layer_vec) noexcept { m_Layers = std::move(layer_vec); }

	/// The files' ICC Profile
	/// 
	/// The ICC Profile defines the view transform on the file but does not 
	/// actually apply any color conversion. If you wish to actually convert your
	/// colors you should instead use something like `lcms2`
	ICCProfile icc_profile() const noexcept { return m_ICCProfile; }
	void icc_profile(ICCProfile profile) noexcept { m_ICCProfile = std::move(profile); }

	/// The files' dots per inch (dpi) resolution
	float& dpi() noexcept { return m_DotsPerInch; }
	float dpi() const noexcept { return m_DotsPerInch; }
	void dpi(float resolution) { m_DotsPerInch = resolution; }

	/// The files' width from 1 - 300,000
	uint64_t& width() noexcept { return m_Width; }
	uint64_t width() const noexcept { return m_Width; }
	void width(uint64_t file_width)
	{
		if (file_width < static_cast<uint64_t>(1))
		{
			PSAPI_LOG_ERROR("LayeredFile", "Unable to set height to %u as the minimum document size in photoshop is 1 for PSB", file_width);
		}
		if (file_width > static_cast<uint64_t>(300000))
		{
			PSAPI_LOG_ERROR("LayeredFile", "Unable to set width to %u as the maximum document size in photoshop is 300,000 for PSB", file_width);
		}
		m_Width = file_width;
	}

	/// The files' height from 1 - 300,000
	uint64_t& height() noexcept { return m_Height; }
	uint64_t height() const noexcept { return m_Height; }
	void height(uint64_t file_height)
	{
		if (file_height < static_cast<uint64_t>(1))
		{
			PSAPI_LOG_ERROR("LayeredFile", "Unable to set height to %u as the minimum document size in photoshop is 1 for PSB", file_height);
		}
		if (file_height > static_cast<uint64_t>(300000))
		{
			PSAPI_LOG_ERROR("LayeredFile", "Unable to set height to %u as the maximum document size in photoshop is 300,000 for PSB", file_height);
		}
		m_Height = file_height;
	}

	/// Retrieve the bounding box describing the canvas, will always have a minimum of 0, 0
	Geometry::BoundingBox<double> bbox() const noexcept 
	{ 
		return Geometry::BoundingBox<double>(
			Geometry::Point2D<double>(0, 0), 
			Geometry::Point2D<double>(static_cast<double>(width()), static_cast<double>(height())));
	}

	/// \brief The files' colormode
	/// 
	/// Currently we only fully support RGB, CMYK and Greyscale.
	Enum::ColorMode& colormode() noexcept { return m_ColorMode; }
	Enum::ColorMode colormode() const noexcept { return m_ColorMode; }
	void colormode(Enum::ColorMode color_mode) noexcept { m_ColorMode = color_mode; }

	/// \brief The files' bitdepth
	/// 
	/// As this is managed by the template type T we do not actually allow users
	/// to set this.
	Enum::BitDepth bitdepth() const noexcept { return m_BitDepth; }

	/// Primarily for internal use or advanced users. Users should usually not have to 
	/// touch this as it's handled for them by SmartObjects themselves
	/// 
	/// LinkedLayers describe a global state of linked files. Their purpose is to store
	/// the raw image data of smart objects such that any layer can have different resolution
	/// than the smart object and for deduplication.
	std::shared_ptr<LinkedLayers<T>> linked_layers() noexcept { return m_LinkedLayers; }
	const std::shared_ptr<LinkedLayers<T>> linked_layers() const noexcept { return m_LinkedLayers; }


	LayeredFile() = default;

	/// \brief Constructs a LayeredFile instance from a Photoshop file.
	///
	/// Takes ownership of the provided Photoshop file, transferring from a flat layer hierarchy
	/// to a layered file using the lrSectionDivider taggedBlock to identify layer breaks.
	///
	/// \param file The PhotoshopFile to transfer
	/// \param file_path The path to the photoshop file
	LayeredFile(std::unique_ptr<PhotoshopFile> file, std::filesystem::path file_path)
	{
		// Take ownership of document
		std::unique_ptr<PhotoshopFile> document = std::move(file);

		m_BitDepth = document->m_Header.m_Depth;
		m_ColorMode = document->m_Header.m_ColorMode;
		m_Width = document->m_Header.m_Width;
		m_Height = document->m_Header.m_Height;

		// Extract the ICC Profile if it exists on the document, otherwise it will simply be empty
		m_ICCProfile = _Impl::read_icc_profile(document.get());
		// Extract the DPI from the document, default to 72
		m_DotsPerInch = _Impl::read_dpi(document.get());
		if (document->m_LayerMaskInfo.m_AdditionalLayerInfo)
		{
			m_LinkedLayers = std::make_shared<LinkedLayers<T>>(document->m_LayerMaskInfo.m_AdditionalLayerInfo.value(), file_path);
		}

		m_Layers = _Impl::template build_layer_hierarchy<T>(*this, std::move(document));
		if (m_Layers.size() == 0)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Read an invalid PhotoshopFile as it does not contain any layers. Is the only layer in the scene locked? This is not supported by the PhotoshopAPI");
		}
	}

	/// \brief Constructs an empty LayeredFile object.
	///
	/// Creates a LayeredFile with the specified color mode, width, and height.
	///
	/// \param colorMode The color mode of the file.
	/// \param width The width of the file in pixels.
	/// \param height The height of the file in pixels.
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint8_t>
	{
		if (width < 1 || width > 300000)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Invalid width for Photoshop file provided, must be in the range of 1-300,000 pixels. Got: %" PRIu64 " pixels", width);
		}
		if (height < 1 || height > 300000)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Invalid height for Photoshop file provided, must be in the range of 1-300,000 pixels. Got: %" PRIu64 " pixels", width);
		}

		m_BitDepth = Enum::BitDepth::BD_8;
		m_ColorMode = colorMode;
		m_Width = width;
		m_Height = height;
	}
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint16_t>
	{
		if (width < 1 || width > 300000)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Invalid width for Photoshop file provided, must be in the range of 1-300,000 pixels. Got: %" PRIu64 " pixels", width);
		}
		if (height < 1 || height > 300000)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Invalid height for Photoshop file provided, must be in the range of 1-300,000 pixels. Got: %" PRIu64 " pixels", width);
		}

		m_BitDepth = Enum::BitDepth::BD_16;
		m_ColorMode = colorMode;
		m_Width = width;
		m_Height = height;
	}
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, float32_t>
	{
		if (width < 1 || width > 300000)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Invalid width for Photoshop file provided, must be in the range of 1-300,000 pixels. Got: %" PRIu64 " pixels", width);
		}
		if (height < 1 || height > 300000)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Invalid height for Photoshop file provided, must be in the range of 1-300,000 pixels. Got: %" PRIu64 " pixels", width);
		}

		if (colorMode == Enum::ColorMode::CMYK)
		{
			PSAPI_LOG_ERROR("LayeredFile", "Invalid bitdepth of 32 specified for CMYK colormode. Only 16- and 32-bit are supported");
		}

		m_BitDepth = Enum::BitDepth::BD_32;
		m_ColorMode = colorMode;
		m_Width = width;
		m_Height = height;
	}

	/// \brief Finds a layer based on the given path.
	/// The path should be separated by forward slashes, e.g., "Group1/GroupNested/ImageLayer".
	/// Returns a reference to the specific layer if found; otherwise, returns nullptr and issues a warning.
	///
	/// \param path The path to the layer.
	/// \return A shared pointer to the found layer or nullptr.
	std::shared_ptr<Layer<T>> find_layer(std::string path) const
	{
		PSAPI_PROFILE_FUNCTION();
		std::vector<std::string> segments = splitString(path, '/');
		for (const auto& layer : m_Layers)
		{
			// Get the layer name and recursively check the path
			if (layer->name() == segments[0])
			{
				// This is a simple path with no nested layers
				if (segments.size() == 1)
				{
					return layer;
				}
				// Pass an index of one as we already found the first layer
				return _Impl::find_layer_recursive(layer, segments, 1);
			}
		}
		PSAPI_LOG_WARNING("LayeredFile", "Unable to find layer path %s", path.c_str());
		return nullptr;
	}

	/// \brief Inserts a layer into the root of the layered file.
	///
	/// If you wish to add a layer to a group, use GroupLayer::addLayer() on a group node retrieved by \ref find_layer().
	///
	/// \param layer The layer to be added.
	void add_layer(std::shared_ptr<Layer<T>> layer)
	{
		if (is_layer_in_file(layer))
		{
			PSAPI_LOG_WARNING("LayeredFile", "Cannot insert a layer into the document twice, please use a unique layer. Skipping layer '%s'", layer->name().c_str());
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
	void move_layer(std::shared_ptr<Layer<T>> layer, std::shared_ptr<Layer<T>> parentLayer = nullptr)
	{
		PSAPI_PROFILE_FUNCTION();
		// We must first check that we are not trying to move a layer higher in the hierarchy to lower in the hierarchy 
		// as that would be undefined behaviour. E.g. if we want to move /Group/ to /Group/NestedGroup that wouldnt work
		// since the down stream nodes are dependant on the upstream nodes
		if (parentLayer && is_moving_to_invalid_hierarchy(layer, parentLayer))
		{
			PSAPI_LOG_WARNING("LayeredFile", "Cannot move layer '%s' under '%s' as that would represent an illegal move operation",
				layer->name().c_str(), parentLayer->name().c_str());
			return;
		}


		// First we must remove the layer from the hierarchy and then reappend it in a different place
		remove_layer(layer);

		// Insert the layer back, either under the provided parent layer or under the scene root
		if (parentLayer)
		{
			if (auto groupLayerPtr = std::dynamic_pointer_cast<GroupLayer<T>>(parentLayer))
			{
				groupLayerPtr->add_layer(*this, layer);
			}
			else
			{
				PSAPI_LOG_WARNING("LayeredFile", "Parent layer '%s' provided is not a group layer, can only move layers under groups", 
					parentLayer->name().c_str());
				return;
			}
		}
		else
		{
			add_layer(layer);
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
	void move_layer(const std::string layer, const std::string parentLayer = "")
	{
		PSAPI_PROFILE_FUNCTION();
		if (parentLayer == "")
		{
			auto layerPtr = find_layer(layer);
			if (!layerPtr) [[unlikely]]
			{
				PSAPI_LOG_ERROR("LayeredFile", "Could not find the layer %s for move_layer()", layer.c_str());
			}
			move_layer(layerPtr);
		}
		else
		{
			auto layerPtr = find_layer(layer);
			auto parentLayerPtr = find_layer(parentLayer);
			if (!layerPtr) [[unlikely]]
			{
				PSAPI_LOG_ERROR("LayeredFile", "Could not find the layer %s for move_layer()", layer.c_str());
			}
			if (!parentLayerPtr) [[unlikely]]
			{
				PSAPI_LOG_ERROR("LayeredFile", "Could not find the parentlayer %s for move_layer()", parentLayer.c_str());
			}
			move_layer(layerPtr, parentLayerPtr);
		}
	}

	/// \brief Recursively removes a layer from the layer structure.
	///
	/// Iterates the layer structure until the given node is found and then removes it from the tree.
	///
	/// \param layer The layer to be removed.
	void remove_layer(std::shared_ptr<Layer<T>> layer)
	{
		PSAPI_PROFILE_FUNCTION();
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
			if (_Impl::remove_layer_recursive(sceneLayer, layer))
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
	void remove_layer(const std::string layer)
	{
		PSAPI_PROFILE_FUNCTION();
		auto layerPtr = find_layer(layer);
		if (!layerPtr) [[unlikely]]
		{
			PSAPI_LOG_ERROR("LayeredFile", "Could not find the layer %s for remove_layer()", layer.c_str());
		}
		remove_layer(layerPtr);
	}
	
	/// \brief change the compression codec across all layers and channels
	///
	/// Iterates the layer structure and changes the compression codec for write on all layers.
	/// This is especially useful for e.g. 8-bit files which from Photoshop write with RLE compression
	/// but ZipCompression gives us better ratios
	/// 
	/// \param compCode the compression codec to apply
	void set_compression(const Enum::Compression compCode)
	{
		for (const auto& documentLayer : m_Layers)
		{
			documentLayer->set_write_compression(compCode);
			_Impl::set_compression_recursive(documentLayer, compCode);
		}
	}

	/// Generate a flat layer stack from either the current root or (if supplied) from the given layer.
	/// It should be preferred to use \ref flat_layers() (no arguments) instead of this function.
	///
	/// \param layer Optional layer to start the generation from (default is root). If you provide e.g. a group this will only build the below layer tree
	/// \param order The order in which layers should be stacked.
	/// \return The flat layer tree with automatic \ref SectionDividerLayer inserted to mark section ends
	std::vector<std::shared_ptr<Layer<T>>> flat_layers(std::optional<std::shared_ptr<Layer<T>>> layer, const LayerOrder order) const
	{
		if (order == LayerOrder::forward)
		{
			if (layer.has_value())
			{
				std::vector<std::shared_ptr<Layer<T>>> layerVec;
				layerVec.push_back(layer.value());
				return _Impl::generate_flattened_layers(layerVec, true);
			}
			return _Impl::generate_flattened_layers(m_Layers, true);
		}
		else if (order == LayerOrder::reverse)
		{
			if (layer.has_value())
			{
				std::vector<std::shared_ptr<Layer<T>>> layerVec;
				layerVec.push_back(layer.value());
				std::vector<std::shared_ptr<Layer<T>>> flatLayers = _Impl::generate_flattened_layers(layerVec, true);
				std::reverse(flatLayers.begin(), flatLayers.end());
				return flatLayers;
			}
			std::vector<std::shared_ptr<Layer<T>>> flatLayers = _Impl::generate_flattened_layers(m_Layers, true);
			std::reverse(flatLayers.begin(), flatLayers.end());
			return flatLayers;
		}
		PSAPI_LOG_ERROR("LayeredFile", "Invalid layer order specified, only accepts forward or reverse");
		return std::vector<std::shared_ptr<Layer<T>>>();
	}


	/// Get a view over the flattened layer stack, helpful for iterating and applying properties to all 
	/// layers such as visibility overrides etc.
	/// 
	/// After any layer modification actions this list may no longer be up-to-date so it would have to be re-generated.
	/// It is highly discouraged to use this flattened layer vector for any layer hierarchy modifications
	/// 
	/// \return The layer hierarchy as a flattened vector that can be iterated over.
	std::vector<std::shared_ptr<Layer<T>>> flat_layers()
	{
		return generate_flattened_layers_impl(LayerOrder::forward);
	}

	/// \brief Gets the total number of channels in the document.
	///
	/// \return The total number of channels in the document.
	uint16_t num_channels()
	{
		bool hasAlpha = false;
		for (auto& layer : m_Layers)
		{
			hasAlpha &= _Impl::has_alpha_recursive(layer);
		}

		uint16_t numChannels = hasAlpha ? 1u : 0u;
		if (m_ColorMode == Enum::ColorMode::RGB ||
			m_ColorMode == Enum::ColorMode::Lab)
		{
			numChannels += 3u;
		}
		else if (m_ColorMode == Enum::ColorMode::CMYK)
		{
			numChannels += 4u;
		}
		else if (
			m_ColorMode == Enum::ColorMode::Bitmap ||
			m_ColorMode == Enum::ColorMode::Indexed ||
			m_ColorMode == Enum::ColorMode::Grayscale ||
			m_ColorMode == Enum::ColorMode::Duotone ||
			m_ColorMode == Enum::ColorMode::Multichannel
			)
		{
			numChannels += 1u;
		}
		return numChannels;
	}

	/// \brief Checks if a layer already exists in the nested structure.
	///
	/// \param layer The layer to check for existence.
	/// \return True if the layer exists, false otherwise.
	bool is_layer_in_file(const std::shared_ptr<Layer<T>> layer) const
	{
		PSAPI_PROFILE_FUNCTION();
		for (const auto& documentLayer : m_Layers)
		{
			if (documentLayer == layer)
			{
				return true;
			}
			if (_Impl::layer_in_document_recursive(documentLayer, layer))
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

		if constexpr (std::is_same_v<T, bpp8_t>)
		{
			if (psDocumentPtr->m_Header.m_Depth != Enum::BitDepth::BD_8)
			{
				PSAPI_LOG_ERROR("LayeredFile", "Tried to read a %d-bit file with a 8-bit LayeredFile instantiation",
					Enum::bitDepthToUint(psDocumentPtr->m_Header.m_Depth));
			}
		}
		else if constexpr (std::is_same_v<T, bpp16_t>)
		{
			if (psDocumentPtr->m_Header.m_Depth != Enum::BitDepth::BD_16)
			{
				PSAPI_LOG_ERROR("LayeredFile", "Tried to read a %d-bit file with a 16-bit LayeredFile instantiation",
					Enum::bitDepthToUint(psDocumentPtr->m_Header.m_Depth));
			}
		}
		else if constexpr (std::is_same_v<T, bpp32_t>)
		{
			if (psDocumentPtr->m_Header.m_Depth != Enum::BitDepth::BD_32)
			{
				PSAPI_LOG_ERROR("LayeredFile", "Tried to read a %d-bit file with a 32-bit LayeredFile instantiation",
					Enum::bitDepthToUint(psDocumentPtr->m_Header.m_Depth));
			}
		}
		return LayeredFile<T>({ std::move(psDocumentPtr), filePath });
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
		_Impl::validate_file(layeredFile);

		File::FileParams params = {};
		params.doRead = false;
		params.forceOverwrite = forceOvewrite;

		if (layeredFile.m_ICCProfile.data_size() == 0 && layeredFile.m_ColorMode == Enum::ColorMode::CMYK)
		{
			PSAPI_LOG_WARNING("LayeredFile",
				"Writing out a CMYK file without an embedded ICC Profile. The output image data will likely look very wrong");
		}

		auto outputFile = File(filePath, params);
		auto psdOutDocumentPtr = layered_to_photoshop(std::move(layeredFile), filePath);
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
	uint64_t m_Width = 1u;

	/// The height of the file in pixels. Can be up to 30,000 for PSD and up to 300,000 for PSB
	uint64_t m_Height = 1u;

	/// Linked layers are external files associated with the layered file. In the context of 
	/// e.g. SmartObjects these will hold the raw file bytes so that multiple smart objects
	/// can access the same layers without data duplication
	std::shared_ptr<LinkedLayers<T>> m_LinkedLayers = std::make_shared<LinkedLayers<T>>();


	std::vector<std::shared_ptr<Layer<T>>> generate_flattened_layers_impl(const LayerOrder order)
	{
		if (order == LayerOrder::forward)
		{
			return _Impl::generate_flattened_layers(m_Layers, false);
		}
		else if (order == LayerOrder::reverse)
		{
			std::vector<std::shared_ptr<Layer<T>>> flatLayers = _Impl::generate_flattened_layers(m_Layers, false);
			std::reverse(flatLayers.begin(), flatLayers.end());
			return flatLayers;
		}
		PSAPI_LOG_ERROR("LayeredFile", "Invalid layer order specified, only accepts forward or reverse");
		return std::vector<std::shared_ptr<Layer<T>>>();
	}

	/// \brief Checks if moving the child layer to the provided parent layer is valid.
	///
	/// Checks for illegal moves, such as moving a layer to itself or above its current parent.
	///
	/// \param layer The child layer to be moved.
	/// \param parentLayer The new parent layer.
	/// \return True if the move is valid, false otherwise.
	bool is_moving_to_invalid_hierarchy(const std::shared_ptr<Layer<T>> layer, const std::shared_ptr<Layer<T>> parentLayer)
	{
		// Check if the layer would be moving to one of its descendants which is illegal. Therefore the argument order is reversed
		bool isDescendantOf = _Impl::layer_in_document_recursive(parentLayer, layer);
		// We additionally check if the layer is the same as the parent layer as that would also not be allowed
		return isDescendantOf || layer == parentLayer;
	}
};


/// \brief Finds a layer based on the given path and casts it to the given type.
///
/// This function matches LayeredFile<T>::find_layer() but instead of returning a generic layer basetype
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
std::shared_ptr<LayerType<T>> find_layer_as(const std::string path, const LayeredFile<T>& layeredFile)
{
	auto basePtr = layeredFile.find_layer(path);
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
std::unique_ptr<PhotoshopFile> layered_to_photoshop(LayeredFile<T>&& layered_file, std::filesystem::path file_path)
{
	PSAPI_PROFILE_FUNCTION();

	// Remove any unused linked layers that do not have a reference to anything anymore
	clear_unused_linked_layers<T>(layered_file);

	FileHeader header = generate_header<T>(layered_file);
	ColorModeData colorModeData = generate_colormodedata<T>(layered_file);
	ImageResources imageResources = generate_imageresources<T>(layered_file);
	LayerAndMaskInformation lrMaskInfo = generate_layermaskinfo<T>(layered_file, file_path);
	ImageData imageData = ImageData(layered_file.num_channels());

	return std::make_unique<PhotoshopFile>(header, colorModeData, std::move(imageResources), std::move(lrMaskInfo), imageData);
}


PSAPI_NAMESPACE_END