#pragma once

#include "FileHeader.h"
#include "AdditionalLayerInfo.h"
#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/ByteStream.h"
#include "Struct/Section.h"
#include "Struct/ResourceBlock.h"
#include "Struct/ImageChannel.h"
#include "Compression/Compression.h"

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
		bool m_isVisible = false;
		bool m_isBit4Useful = false;	// This bit simply tells us if the next section holds useful information and whether it should be considered
		bool m_isPixelDataIrrelevant = false;	// If m_isBit4Useful is set to false this will always also be false, no matter if the value itself would be true

		// Set the internal flag states using the provided flag uint8_t
		void setFlags(const uint8_t flags);
		// Return the current flag states as a uint8_t with the relevant bits set
		uint8_t getFlags() const;

		BitFlags() = default;
		BitFlags(const uint8_t flags);
		BitFlags(const bool isTransparencyProtected, const bool isVisible, const bool isPixelDataIrrelevant);

	private:
		const static uint8_t m_transparencyProtectedMask = 1u << 0;
		const static uint8_t m_visibleMask = 1u << 1;
		const static uint8_t m_bit4UsefulMask = 1u << 3;
		const static uint8_t m_pixelDataIrrelevantMask = 1u << 4;
	};


	struct ChannelInformation
	{
		Enum::ChannelIDInfo m_ChannelID;
		uint64_t m_Size;	// This appears to include the length of the compression marker
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

		bool m_HasUserMaskDensity = false;		// Bit 0 of the maskParams
		bool m_HasUserMaskFeather = false;		// Bit 1 of the maskParams
		bool m_HasVectorMaskDensity = false;	// Bit 2 of the maskParams
		bool m_HasVectorMaskFeather = false;	// Bit 3 of the maskParams

		// Only exists on one of the two masks
		std::optional<uint8_t> m_UserMaskDensity;
		std::optional<float64_t> m_UserMaskFeather;
		std::optional<uint8_t> m_VectorMaskDensity;
		std::optional<float64_t> m_VectorMaskFeather;

		uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

		void setFlags(const uint32_t bitFlag);
		void setMaskParams(const uint32_t bitFlag);
		uint32_t readMaskParams(File& document);

	private:
		// Masks to perform bitwise & operations with to check if certain flags exist
		// Note: we skip bit 2 here as its marked obsolete and there is only 5 total options
		const uint8_t m_PositionRelativeToLayerMask = 1u << 0;
		const uint8_t m_DisabledMask = 1u << 1;
		const uint8_t m_IsVectorMask = 1u << 3;
		const uint8_t m_HasMaskParamsMask = 1u << 4;

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
		std::optional<LayerMask> m_LayerMask;
		std::optional<LayerMask> m_VectorMask;

		LayerMaskData() = default;

		uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

		void read(File& document);
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

		uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

		void read(File& document);
	};
}


// A layer record describes a single layer in a photoshop document and may include up to 56 channels
struct LayerRecord : public FileSection
{
	PascalString m_LayerName;

	int32_t m_Top, m_Left, m_Bottom, m_Right;
	uint16_t m_ChannelCount;
	std::vector<LayerRecords::ChannelInformation> m_ChannelInformation;
	Enum::BlendMode m_BlendMode;
	uint8_t m_Opacity;	// 0 - 255
	uint8_t m_Clipping;	// 0 or 1
	LayerRecords::BitFlags m_BitFlags;

	std::optional<LayerRecords::LayerMaskData> m_LayerMaskData;
	LayerRecords::LayerBlendingRanges m_LayerBlendingRanges;
	std::optional<AdditionalLayerInfo> m_AdditionalLayerInfo;

	// Explicitly delete any copy operators as we cannot copy AdditionalLayerInfo
	LayerRecord(const LayerRecord&) = delete;
	LayerRecord(LayerRecord&&) = default;
	LayerRecord& operator=(const LayerRecord&) = delete;
	LayerRecord& operator=(LayerRecord&&) = default;

	LayerRecord();
	// Construct a layer record with literal values, useful when we know all the data beforehand, i.e. for round tripping
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

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	void read(File& document, const FileHeader& header, const uint64_t offset);
};


// This currently just gets skipped
struct GlobalLayerMaskInfo : public FileSection
{
	GlobalLayerMaskInfo() {};

	// We dont store anythin here so just an empty size marker will do
	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override { return 4u; };

	// Skip the contents of the Global Layer and Mask Info based on the length marker
	void read(File& document, const uint64_t offset);
};


// Channel Image Data for a single layer, there is at most 56 channels in a given layer
struct ChannelImageData : public FileSection
{

	// We hold the image data for all of the channels in this vector.
	// The image data gets compressed using blosc2 on creation allowing for a very small
	// memory footprint
	std::vector<std::unique_ptr<BaseImageChannel>> m_ImageData;

	ChannelImageData() = default;
	ChannelImageData(std::vector<std::unique_ptr<BaseImageChannel>> data) : m_ImageData(std::move(data)) {};

	// This function will raise a warning as we do not know the size of the compressed image data at this stage yet, only once we actually write this information 
	// becomes available. To get an estimate of the size use the estimateSize() function instead
	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	// Estimate the size the of compressed data by compressing n amount of chunks from the data and averaging the compression ratio
	// The chunks are chosen at random and have the size of m_ChunkSize in the ImageChannels. numSamples controls how many random chunks we choose
	template <typename T>
	uint64_t estimateSize(const FileHeader header, const uint16_t numSamples = 16u);

	// Read a single channel image data instance from a pre-allocated bytestream
	void read(ByteStream& stream, const FileHeader& header, const uint64_t offset, const LayerRecord& layerRecord);

	// Get an index to a specific channel based on the identifier
	// returns -1 if no matching channel is found
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

	// Get an index to a specific channel based on the identifier
	// returns -1 if no matching channel is found
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

	// Extract a channel from the given index and take ownership of the data. After this function is called the index will point to nullptr
	// If the channel has already been extracted we return an empty array of T and raise a warning about accessing elements that have already
	// had their data removed
	template <typename T>
	std::vector<T> extractImageData(int index)
	{
		// Take ownership of and invalidate the current index
		std::unique_ptr<BaseImageChannel> imageChannelPtr = std::move(m_ImageData.at(index));
		if (imageChannelPtr == nullptr)
		{
			PSAPI_LOG_WARNING("ChannelImageData", "Channel %i no longer contains any data, was it extracted beforehand?", index)
			auto emptyVec = std::vector<T>();
			return emptyVec;
		}
		m_ImageData[index] = nullptr;

		if (auto imageChannel = dynamic_cast<ImageChannel<T>*>(imageChannelPtr.get()))
		{
			return std::move(imageChannel->getData());
		}
		else
		{
			PSAPI_LOG_ERROR("ChannelImageData", "Unable to extract image data for channel at index %i", index)
			auto emptyVec = std::vector<T>();
			return emptyVec;
		}
	}

	// Extract a channel from the given ChannelID and take ownership of the data. After this function is called the index will point to nullptr
	// If the channel has already been extracted we return an empty array of T and raise a warning about accessing elements that have already
	// had their data removed
	template <typename T>
	std::vector<T> extractImageData(Enum::ChannelID channelID)
	{
		const int index = this->getChannelIndex(channelID);

		// Take ownership of and invalidate the current index
		std::unique_ptr<BaseImageChannel> imageChannelPtr = std::move(m_ImageData.at(index));
		if (imageChannelPtr == nullptr)
		{
			PSAPI_LOG_WARNING("ChannelImageData", "Channel %i no longer contains any data, was it extracted beforehand?", index);
			return std::vector<T>();
		}
		m_ImageData[index] = nullptr;

		if (auto imageChannel = dynamic_cast<ImageChannel<T>*>(imageChannelPtr))
		{
			return std::move(imageChannel->getData());
		}
		else
		{
			PSAPI_LOG_ERROR("ChannelImageData", "Unable to extract image data for channel at index %i", index)
			return std::vector<T>();			
		}
	}


	// Extract a channels pointer from our channel vector and invalidate the index. If the channel is already a nullptr
	// we just return that silently and leave it up to the caller to check for this
	std::unique_ptr<BaseImageChannel> extractImagePtr(Enum::ChannelIDInfo channelIDInfo)
	{
		const int index = this->getChannelIndex(channelIDInfo);
		// Take ownership of and invalidate the current index
		std::unique_ptr<BaseImageChannel> imageChannelPtr = std::move(m_ImageData.at(index));
		if (imageChannelPtr == nullptr)
		{
			return nullptr;
		}
		m_ImageData[index] = nullptr;
		return std::move(imageChannelPtr);
	}
};


struct LayerInfo : public FileSection
{
	// These two are guaranteed to be in the same order based on Photoshop specification
	std::vector<LayerRecord> m_LayerRecords;
	std::vector<ChannelImageData> m_ChannelImageData;

	LayerInfo() = default;
	LayerInfo(std::vector<LayerRecord> layerRecords, std::vector<ChannelImageData> imageData) : m_LayerRecords(std::move(layerRecords)), m_ChannelImageData(std::move(imageData)) {};

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	// Read the layer info section
	void read(File& document, const FileHeader& header, const uint64_t offset, const bool isFromAdditionalLayerInfo = false, std::optional<uint64_t> sectionSize = std::nullopt);

	// Find the index to a layer based on a layer name that is given
	// if no layer with the name is found, return -1. In the case of multiple name matches the last in the photoshop document
	// is returned. This can also be used to get an index into the ChannelImageData vector
	int getLayerIndex(const std::string& layerName);
};


struct LayerAndMaskInformation : public FileSection
{

	LayerInfo m_LayerInfo;
	GlobalLayerMaskInfo m_GlobalLayerMaskInfo;
	std::optional<AdditionalLayerInfo> m_AdditionalLayerInfo;

	LayerAndMaskInformation() = default;
	LayerAndMaskInformation(LayerInfo& layerInfo, GlobalLayerMaskInfo globalLayerMaskInfo, std::optional<AdditionalLayerInfo> additionalLayerInfo) :
		m_LayerInfo(std::move(layerInfo)), m_GlobalLayerMaskInfo(globalLayerMaskInfo), m_AdditionalLayerInfo(std::move(additionalLayerInfo)) {};

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	void read(File& document, const FileHeader& header, const uint64_t offset);
};


PSAPI_NAMESPACE_END