#pragma once

#include "FileHeader.h"
#include "AdditionalLayerInfo.h"
#include "Macros.h"
#include "Util/Enum.h"
#include "Util/ProgressCallback.h"
#include "Core/Struct/File.h"
#include "Core/Struct/ByteStream.h"
#include "Core/Struct/Section.h"
#include "Core/Struct/ResourceBlock.h"
#include "Core/Struct/ImageChannel.h"
#include "Core/Compression/Compression.h"

#include <vector>
#include <memory>



PSAPI_NAMESPACE_BEGIN


// Structs to hold the different types of data found in the layer records themselves
namespace LayerRecords
{

	// Holds the bit flags found in each layer record instance, there are only 5 documented ones but the other 3 appear to also hold some information.
	// However, this is mostly irrelevant for now
	struct BitFlags
	{
		bool m_isTransparencyProtected = false;
		bool m_isHidden = false;
		bool m_isBit4Useful = false;	// This bit simply tells us if the next section holds useful information and whether it should be considered
		bool m_isPixelDataIrrelevant = false;	// If m_isBit4Useful is set to false this will always also be false, no matter if the value itself would be true


		// Set the internal flag states using the provided flag uint8_t
		void setFlags(const uint8_t flags) noexcept;
		// Return the current flag states as a uint8_t with the relevant bits set
		uint8_t getFlags() const noexcept;

		BitFlags() = default;
		BitFlags(const uint8_t flags);
		BitFlags(const bool isTransparencyProtected, const bool isHidden, const bool isPixelDataIrrelevant);

	private:
		const static uint8_t m_transparencyProtectedMask = 1u << 0;
		const static uint8_t m_hiddenMask = 1u << 1;
		const static uint8_t m_unknownBit2Mask = 1u << 2;
		const static uint8_t m_bit4UsefulMask = 1u << 3;
		const static uint8_t m_pixelDataIrrelevantMask = 1u << 4;
	};


	struct ChannelInformation
	{
		Enum::ChannelIDInfo m_ChannelID;
		uint64_t m_Size;	// This appears to include the length of the compression marker

		inline bool operator==(const ChannelInformation& other) const 
		{
			return m_ChannelID == other.m_ChannelID;
		}
	};


	// A singular layer mask as represented in the LayerMaskData section found in the layer records
	struct LayerMask : public FileSection
	{
		int32_t m_Top = 0, m_Left = 0, m_Bottom = 0, m_Right = 0;
		uint8_t m_DefaultColor = 0u;		// 0 or 255

		bool m_PositionRelativeToLayer = false;	// Bit 0 of the flags
		bool m_Disabled = false;				// Bit 1 of the flags
		bool m_IsVector = false;				// Bit 3 of the flags
		bool m_HasMaskParams = false;			// Bit 4 of the flags

		bool m_unknownBit2 = false;
		bool m_unknownBit5 = false;
		bool m_unknownBit6 = false;
		bool m_unknownBit7 = false;

		bool m_HasUserMaskDensity = false;		// Bit 0 of the maskParams
		bool m_HasUserMaskFeather = false;		// Bit 1 of the maskParams
		bool m_HasVectorMaskDensity = false;	// Bit 2 of the maskParams
		bool m_HasVectorMaskFeather = false;	// Bit 3 of the maskParams

		// Only exists on one of the two masks
		std::optional<uint8_t> m_UserMaskDensity;
		std::optional<float64_t> m_UserMaskFeather;
		std::optional<uint8_t> m_VectorMaskDensity;
		std::optional<float64_t> m_VectorMaskFeather;

		// Set the boolean flags according to the data read from disk
		void setFlags(const uint8_t bitFlag);
		// Get the currently set flags as a uint8_t for writing
		uint8_t getFlags() const noexcept;
		// Set the boolean flags according to the data read from disk
		void setMaskParams(const uint8_t bitFlag);
		// Get the currently set flags as a uint8_t for writing
		uint8_t getMaskParams() const noexcept;
		// Read the mask parameters according to which mask parameter bit flags are set and return the total
		// length of all the bytes read
		uint32_t readMaskParams(File& document);
		// Write the mask parameters according to which mask parameter bit flags are set 
		// and return the amount of bytes written
		uint32_t writeMaskParams(File& document) const noexcept;

	private:
		// Masks to perform bitwise & operations with to check if certain flags exist
		// Note: we skip bit 2 here as its marked obsolete and there is only 5 total options
		const uint8_t m_PositionRelativeToLayerMask = 1u << 0;
		const uint8_t m_DisabledMask = 1u << 1;
		const uint8_t m_IsVectorMask = 1u << 3;
		const uint8_t m_HasMaskParamsMask = 1u << 4;

		// Unknown bits of the flags which do sometimes get written and affect the look of the document
		// mostly for roundtripping
		const static uint8_t m_unknownBit2Mask = 1u << 5;
		const static uint8_t m_unknownBit5Mask = 1u << 5;
		const static uint8_t m_unknownBit6Mask = 1u << 6;
		const static uint8_t m_unknownBit7Mask = 1u << 7;	

		// Mask parameter bitmasks to bitwise & with
		const uint8_t m_UserMaskDensityMask = 1u << 0;
		const uint8_t m_UserMaskFeatherMask = 1u << 1;
		const uint8_t m_VectorMaskDensityMask = 1u << 2;
		const uint8_t m_VectorMaskFeatherMask = 1u << 3;
	};


	// This section can hold either no mask, one mask or two masks depending on the size of the data in it.
	// The layout is a bit confusing here as it reads the second mask in reverse order. The mask parameters
	// exist only on one of the masks rather than both as they cover both cases
	struct LayerMaskData : public FileSection
	{
		std::optional<LayerMask> m_LayerMask = std::nullopt;
		std::optional<LayerMask> m_VectorMask = std::nullopt;

		LayerMaskData() = default;

		void read(File& document);
		// Write the layer masks, currently only a single LayerMask is supported for this
		void write(File& document) const;
	};


	struct LayerBlendingRanges : public FileSection
	{
		// Blending ranges hold 2 low and 2 high values, if the marker wasnt split in photoshop 
		// the low and high values are identical
		using Data = std::vector<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>>;
		Data m_SourceRanges;
		Data m_DestinationRanges;

		// Initialize blending ranges with defaults, this works for all color modes
		LayerBlendingRanges();

		void read(File& document);
		void write(File& document) const;
	};
}


/// A layer record describes a single layer in a photoshop document and may include up to 56 channels
struct LayerRecord : public FileSection
{
	/// The name of the Layer as pascal string, has a maximum length of 255
	PascalString m_LayerName;

	/// The top edge of the layer bounding box
	int32_t m_Top{};

	/// The left edge of the layer bounding box
	int32_t m_Left{};

	/// The bottom edge of the layer bounding box
	int32_t m_Bottom{};

	/// The right edge of the layer bounding box
	int32_t m_Right{};

	/// The absolute amount of channels stored in the layer
	uint16_t m_ChannelCount{};
	/// The channel information telling us the channel ID as well as the 
	/// size of the channel for when we read the ChannelImageData
	std::vector<LayerRecords::ChannelInformation> m_ChannelInformation;
	Enum::BlendMode m_BlendMode;
	/// 0 - 255
	uint8_t m_Opacity;
	/// 0 or 1
	uint8_t m_Clipping;	
	/// Bit flags which control certain information such as visibility
	LayerRecords::BitFlags m_BitFlags;

	/// If one or both of the layer masks has some special data on it (feather or blur) it will be stored in this structure
	std::optional<LayerRecords::LayerMaskData> m_LayerMaskData = std::nullopt;

	/// The channel blending ranges for all the default channels (r, g and b in rgb color mode). Photoshop appears
	/// to always write out the maximum possible channels (5) as the section size is trivial. We match this behaviour
	LayerRecords::LayerBlendingRanges m_LayerBlendingRanges;
	/// An optional series of TaggedBlocks. This is where, e.g. SmartObjects or Adjustment layers would store their data
	std::optional<AdditionalLayerInfo> m_AdditionalLayerInfo;

	// Explicitly delete any copy operators as we cannot copy AdditionalLayerInfo
	LayerRecord(const LayerRecord&) = delete;
	LayerRecord(LayerRecord&&) = default;
	LayerRecord& operator=(const LayerRecord&) = delete;
	LayerRecord& operator=(LayerRecord&&) = delete;

	LayerRecord();
	/// Construct a layer record with literal values, useful when we know all the data beforehand, i.e. for round tripping
	LayerRecord(
		PascalString layerName,
		int32_t top,
		int32_t left,
		int32_t bottom,
		int32_t right,
		uint16_t channelCount,
		std::vector<LayerRecords::ChannelInformation> channelInfo,
		Enum::BlendMode blendMode,
		uint8_t opacity,
		uint8_t clipping,
		LayerRecords::BitFlags bitFlags,
		std::optional<LayerRecords::LayerMaskData> layerMaskData,
		LayerRecords::LayerBlendingRanges layerBlendingRanges,
		std::optional<AdditionalLayerInfo> additionalLayerInfo
	);

	/// Read and Initialize the struct from disk using the given offset
	void read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset);

	/// Write the layer record to disk, requires the Image data to be compressed already and the size to be known
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const std::vector<LayerRecords::ChannelInformation> channelInfos) const;

	/// Extract the absolute width of the layer
	uint32_t getWidth() const noexcept;

	/// Extract the absolute height of the layer
	uint32_t getHeight() const noexcept;
};


// This currently just gets skipped
struct GlobalLayerMaskInfo : public FileSection
{
	GlobalLayerMaskInfo() {};

	// Skip the contents of the Global Layer and Mask Info based on the length marker
	void read(File& document, const uint64_t offset);
	void write(File& document);
};


/// Channel Image Data for a single layer, there is at most 56 channels in a given layer
struct ChannelImageData : public FileSection
{
	ChannelImageData() = default;
	ChannelImageData(std::vector<std::unique_ptr<ImageChannel>> data) : m_ImageData(std::move(data)) 
	{
		for (const auto& item : m_ImageData)
		{
			m_ChannelCompression.push_back(item->m_Compression);
		}
	};

	/// Estimate the size the of compressed data by compressing n amount of chunks from the data and averaging the compression ratio
	/// The chunks are chosen at random and have the size of m_ChunkSize in the ImageChannels. numSamples controls how many random chunks we choose
	template <typename T>
	uint64_t estimateSize(const FileHeader& header, const uint16_t numSamples = 16u);

	/// Compress the data for the current layer and return the individual channels, invalidating the data as we go.
	/// This function must be called before writing the data for the LayerRecord as it reveals the size of the data
	/// required to write them. We fill out the lrChannelInfo and lrCompression vector as it goes.
	template <typename T>
	std::vector<std::vector<uint8_t>> compressData(const FileHeader& header, std::vector<LayerRecords::ChannelInformation>& lrChannelInfo, std::vector<Enum::Compression>& lrCompression, size_t numThreads);

	/// Read a single layer instance from a pre-allocated bytestream
	void read(ByteStream& stream, const FileHeader& header, const uint64_t offset, const LayerRecord& layerRecord);

	/// Write a single layer to disk, there is no need to write to a preallocated buffer here as we compress ahead of time
	void write(File& document, std::vector<std::vector<uint8_t>>& compressedChannelData, const std::vector<Enum::Compression>& channelCompression);

	/// Get an index to a specific channel based on the identifier
	/// returns -1 if no matching channel is found
	int getChannelIndex(Enum::ChannelID channelID) const
	{
		for (int i = 0; i < m_ImageData.size(); ++i)
		{
			if (m_ImageData[i]->m_ChannelID.id == channelID)
			{
				return i;
			}
		}
		return -1;
	}

	/// Get an index to a specific channel based on the identifier
	/// returns -1 if no matching channel is found
	int getChannelIndex(Enum::ChannelIDInfo channelIDInfo) const
	{
		for (int i = 0; i < m_ImageData.size(); ++i)
		{
			// Check if the ptr is valid as well as comparing the channelInfo struct
			auto& imgData = m_ImageData.at(i);
			if (imgData && imgData->m_ChannelID == channelIDInfo)
			{
				return i;
			}
		}
		return -1;
	}

	/// Extract a channel from the given index and take ownership of the data. After this function is called the index will point to nullptr
	/// If the channel has already been extracted we return an empty array of T and raise a warning about accessing elements that have already
	/// had their data removed
	template <typename T>
	std::vector<T> extractImageData(int index)
	{
		// Take ownership of and invalidate the current index
		std::unique_ptr<ImageChannel> imageChannelPtr = std::move(m_ImageData.at(index));
		if (imageChannelPtr == nullptr)
		{
			PSAPI_LOG_WARNING("ChannelImageData", "Channel %i no longer contains any data, was it extracted beforehand?", index);
			return std::vector<T>();
		}
		m_ImageData[index] = nullptr;
		return imageChannelPtr->extractData<T>();
	}

	/// Extract a channel from the given ChannelID and take ownership of the data. After this function is called the index will point to nullptr
	/// If the channel has already been extracted we return an empty array of T and raise a warning about accessing elements that have already
	/// had their data removed
	template <typename T>
	std::vector<T> extractImageData(Enum::ChannelID channelID)
	{
		const int index = this->getChannelIndex(channelID);
		return extractImageData<T>(index);
	}

	/// Extract a channels pointer from our channel vector and invalidate the index. If the channel is already a nullptr
	/// we just return that silently and leave it up to the caller to check for this
	std::unique_ptr<ImageChannel> extractImagePtr(Enum::ChannelIDInfo channelIDInfo)
	{
		const int index = this->getChannelIndex(channelIDInfo);
		if (index == -1)
		{
			PSAPI_LOG_WARNING("ChannelImageData", "Unable to retrieve index %i from the ChannelImageData", channelIDInfo.index);
			return nullptr;
		}
		// Take ownership of and invalidate the current index
		std::unique_ptr<ImageChannel> imageChannelPtr = std::move(m_ImageData.at(index));
		if (imageChannelPtr == nullptr)
		{
			return nullptr;
		}
		m_ImageData[index] = nullptr;
		return std::move(imageChannelPtr);
	}

	/// Get the offsets and sizes for each of the channels, the order being the same as m_ImageData. Therefore indices
	/// gotten through e.g. getChannelIndex() are valid here as well. The offsets include the compression marker (2 bytes)
	/// so the actual data starts at offset + 2
	std::vector<std::tuple<uint64_t, uint64_t>> getChannelOffsetsAndSizes() const noexcept { return m_ChannelOffsetsAndSizes; };

	/// Get the compression of a channel by logical index acquired by e.g. getChannelIndex
	inline Enum::Compression getChannelCompression(int index) const noexcept {	return m_ChannelCompression.at(index); };
private:
	/// Store the offset and size of each of the compressed channels. The offset starts at the channel compression marker
	std::vector<std::tuple<uint64_t, uint64_t>> m_ChannelOffsetsAndSizes;

	/// Store the compression marker for all the channels
	std::vector<Enum::Compression> m_ChannelCompression;

	/// We hold the image data for all of the channels in this vector.
	/// The image data gets compressed using blosc2 on creation allowing for a very small
	/// memory footprint
	std::vector<std::unique_ptr<ImageChannel>> m_ImageData;
};


/// \brief The LayerInfo section holds the layer structure as well as the image data for the layers
/// 
/// Internally these are stored as two vectors of LayerRecord as well as ChannelImageData with the 
/// same size as the order is the exact same. E.g. a Layer Record at index 5 would correlate to the 
/// Image Data at index 5. 
/// 
/// If the file is 16- or 32-bit this section exists twice, once in its regular
/// spot in the LayerAndMaskInformation section and again in a 'Lr16' or 'Lr32' TaggedBlock. If this
/// is the case the section in the LayerAndMaskInformation must be empty!
struct LayerInfo : public FileSection
{
	std::vector<LayerRecord> m_LayerRecords;
	std::vector<ChannelImageData> m_ChannelImageData;

	LayerInfo() = default;
	LayerInfo(std::vector<LayerRecord> layerRecords, std::vector<ChannelImageData> imageData) : m_LayerRecords(std::move(layerRecords)), m_ChannelImageData(std::move(imageData)) {};

	/// Read and Initialize the struct from disk using the given offset
	///
	/// \param isFromAdditionalLayerInfo If true the section is parsed without a size marker as it is already stored on the tagged block
	/// \param sectionSize This parameter must be present when isFromAdditionalLayerInfo = true
	void read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const bool isFromAdditionalLayerInfo = false, std::optional<uint64_t> sectionSize = std::nullopt);
	/// Write the layer info section to file with the given padding
	void write(File& document, const FileHeader& header, ProgressCallback& callback);

	/// Find the index to a layer based on a layer name that is given
	/// 
	/// If no layer with the name is found, return -1. In the case of multiple name matches the last in the photoshop document
	/// is returned (due to photoshop storing layers in reverse). 
	/// This can also be used to get an index into the ChannelImageData vector as the indices are identical
	int getLayerIndex(const std::string& layerName);
};


/// \brief The LayerAndMaskInformation section stores the layer structure as well as any image data associated with it 
///
/// The section is split up into two fixed components and one optional section. It always contains a LayerInfo section
/// as well as a GlobalLayerMaskInfo section and optionally can contain an AdditionalLayerInfo section which e.g. in 
/// 16- and 32-bit mode stores the LayerInfo struct as a Tagged Block. 
/// 
/// This appears to be more of a band aid fix when 
/// they introduced 16-bit colours in 1992 and again when 32-bit colours were introduced in 2005 although it is not
/// quite clear why this was done rather than directly integrating it into the LayerInfo section as the information
/// and structure is identical.
struct LayerAndMaskInformation : public FileSection
{
	/// This struct holds the documents layers and channel image data
	LayerInfo m_LayerInfo;
	/// This section is undocumented and gets skipped
	GlobalLayerMaskInfo m_GlobalLayerMaskInfo;
	/// If present, this holds a list of tagged blocks which add some document specific layer data
	std::optional<AdditionalLayerInfo> m_AdditionalLayerInfo;

	LayerAndMaskInformation() = default;
	LayerAndMaskInformation(LayerInfo& layerInfo, GlobalLayerMaskInfo globalLayerMaskInfo, std::optional<AdditionalLayerInfo> additionalLayerInfo) :
		m_LayerInfo(std::move(layerInfo)), m_GlobalLayerMaskInfo(globalLayerMaskInfo), m_AdditionalLayerInfo(std::move(additionalLayerInfo)) {};

	/// Read and Initialize the struct from disk using the given offset
	void read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset);

	/// Write the section to disk in a Photoshop compliant way
	void write(File& document, const FileHeader& header, ProgressCallback& callback);
};


PSAPI_NAMESPACE_END