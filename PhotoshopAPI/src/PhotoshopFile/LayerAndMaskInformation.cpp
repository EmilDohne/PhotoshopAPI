#include "LayerAndMaskInformation.h"

#include "FileHeader.h"
#include "Macros.h"
#include "FileIO/Read.h"
#include "FileIO/Util.h"
#include "StringUtil.h"
#include "Profiling/Perf/Instrumentor.h"

#include <variant>
#include <algorithm>
#include <execution>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::BitFlags::setFlags(const uint8_t flags)
{
	m_isTransparencyProtected = (flags & m_transparencyProtectedMask) != 0;
	m_isVisible = (flags & m_visibleMask) != 0;
	// Bit 2 holds no relevant information
	m_isBit4Useful = (flags & m_bit4UsefulMask) != 0;
	m_isPixelDataIrrelevant = (flags & m_pixelDataIrrelevantMask) != 0 && m_isBit4Useful;
	// bit 5-7 holds no data (according to the documentation)
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint8_t LayerRecords::BitFlags::getFlags() const
{
	uint8_t result = 0u;
	if (m_isTransparencyProtected) result = result & 1u << 0;
	if (m_isVisible) result = result & 1u << 1;
	if (m_isBit4Useful) result = result & 1u << 3;
	if (m_isPixelDataIrrelevant) result = result & 1u << 4;

	return result;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LayerRecords::BitFlags::BitFlags(const uint8_t flags)
{
	this->setFlags(flags);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LayerRecords::BitFlags::BitFlags(const bool isTransparencyProtected, const bool isVisible, const bool isPixelDataIrrelevant)
{
	m_isTransparencyProtected = isTransparencyProtected;
	m_isVisible = isVisible;
	// TODO this approach of simplifying is probably fine but we need to test if it actually defaults to false or not
	if (isPixelDataIrrelevant)
	{
		m_isBit4Useful = true;
		m_isPixelDataIrrelevant = true;
	}
	else
	{
		m_isBit4Useful = false;
		m_isPixelDataIrrelevant = false;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerRecords::LayerMask::calculateSize(std::optional<FileHeader> header) const
{
	uint64_t size = 0u;
	size += 16u;	// Enclosing rectangle
	size += 1u;		// Default color
	size += 1u;		// Flags
	if (m_HasMaskParams)
	{
		size += 1u;	// Mask parameter bit flags
		if (m_HasUserMaskDensity) size += 1u;
		if (m_HasUserMaskFeather) size += 4u;
		if (m_HasVectorMaskDensity) size += 1u;
		if (m_HasVectorMaskFeather) size += 4u;
	}

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerMask::setFlags(const uint32_t bitFlag)
{
	m_PositionRelativeToLayer = (bitFlag & m_PositionRelativeToLayerMask) != 0;
	m_Disabled = (bitFlag & m_DisabledMask) != 0;
	m_IsVector = (bitFlag & m_IsVectorMask) != 0;
	m_HasMaskParams = (bitFlag & m_HasMaskParamsMask) != 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerMask::setMaskParams(const uint32_t bitFlag)
{
	m_HasUserMaskDensity = (bitFlag & m_UserMaskDensityMask) != 0;
	m_HasUserMaskFeather = (bitFlag & m_UserMaskFeatherMask) != 0;
	m_HasVectorMaskDensity = (bitFlag & m_VectorMaskDensityMask) != 0;
	m_HasVectorMaskFeather = (bitFlag & m_VectorMaskFeatherMask) != 0;
}


// Read the mask parameters according to which mask parameter bit flags are set and return the total
// length of all the bytes read
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint32_t LayerRecords::LayerMask::readMaskParams(File& document)
{
	uint32_t bytesRead = 0u;
	if (m_HasUserMaskDensity)
	{
		m_UserMaskDensity.emplace(ReadBinaryData<uint8_t>(document));
		bytesRead += 1u;
	}
	if (m_HasUserMaskFeather)
	{
		m_UserMaskFeather.emplace(ReadBinaryData<float64_t>(document));
		bytesRead += 8u;
	}
	if (m_HasVectorMaskDensity)
	{
		m_VectorMaskDensity.emplace(ReadBinaryData<uint8_t>(document));
		bytesRead += 1u;
	}
	if (m_HasVectorMaskFeather)
	{
		m_VectorMaskFeather.emplace(ReadBinaryData<float64_t>(document));
		bytesRead += 8u;
	}

	return bytesRead;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerRecords::LayerMaskData::calculateSize(std::optional<FileHeader> header) const
{
	uint64_t size = 0u;
	size += 4u;	// Size marker

	// Since we already take care of making sure only one of these has mask parameters
	// during initialization/read we dont actually need to perform any checks here
	if (m_VectorMask.has_value())
	{
		size += m_VectorMask.value().calculateSize();
	}
	if (m_LayerMask.has_value())
	{
		size += m_LayerMask.value().calculateSize();
	}
	
	// xor as we only have padding bytes if exactly one of these exists
	if (m_LayerMask.has_value() != m_VectorMask.has_value())
	{ 
		size += 2u;
	}

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerMaskData::read(File& document)
{
	m_Size = ReadBinaryData<uint32_t>(document) + 4u;
	int64_t toRead = static_cast<int64_t>(m_Size) - 4u;

	// Empty section;
	if (toRead == 0)
	{
		return;
	}

	// Use this to make sure we only read one set of mask parameters
	bool hasMaskParams = false;

	// Read the first layer mask, we dont yet know which one it is
	{
		LayerMask mask = LayerMask();

		mask.m_Top = ReadBinaryData<int32_t>(document);
		mask.m_Left = ReadBinaryData<int32_t>(document);
		mask.m_Bottom = ReadBinaryData<int32_t>(document);
		mask.m_Right = ReadBinaryData<int32_t>(document);
		toRead -= 16u;

		const uint8_t defaultColor = ReadBinaryData<uint8_t>(document);
		toRead -= 1u;
		if (defaultColor != 0 && defaultColor != 255)
		{
			PSAPI_LOG_ERROR("LayerMaskData", "Layer Mask default color can only be 0 or 255, not %u", defaultColor);
		}

		const uint8_t bitFlags = ReadBinaryData<uint8_t>(document);
		mask.setFlags(bitFlags);
		toRead -= 1u;

		// Store this value to compare against later
		hasMaskParams = mask.m_HasMaskParams;
		if (hasMaskParams && m_Size <= 28)
		{
			const uint8_t maskParams = ReadBinaryData<uint8_t>(document);
			mask.setMaskParams(maskParams);
			toRead -= 1u;
			toRead -= mask.readMaskParams(document);
		}

		mask.m_Size = mask.calculateSize();
		// Depending on the flags this is either a vector or layer mask
		if ((bitFlags & 1u << 3) != 0u)
		{
			m_VectorMask.emplace(mask);
		}
		else
		{
			m_LayerMask.emplace(mask);
		}
	}

	// Check if there is still enough space left to read another section
	// If there is 2 masks the vector mask will always be first, therefore we push 
	// back into the real user mask instead (pixel mask)
	if (toRead >= 18u)
	{
		LayerMask layerMask = LayerMask();

		const uint8_t bitFlags = ReadBinaryData<uint8_t>(document);
		layerMask.setFlags(bitFlags);
		toRead -= 1u;

		layerMask.m_DefaultColor = ReadBinaryData<uint8_t>(document);
		if (layerMask.m_DefaultColor != 0 && layerMask.m_DefaultColor != 255)
		{
			PSAPI_LOG_ERROR("LayerMaskData", "Layer Mask default color can only be 0 or 255, not %u", layerMask.m_DefaultColor);
		}
		toRead -= 1u;

		layerMask.m_Top = ReadBinaryData<int32_t>(document);
		layerMask.m_Left = ReadBinaryData<int32_t>(document);
		layerMask.m_Bottom = ReadBinaryData<int32_t>(document);
		layerMask.m_Right = ReadBinaryData<int32_t>(document);
		toRead -= 16u;

		if (hasMaskParams || layerMask.m_HasMaskParams)
		{
			const uint8_t maskParams = ReadBinaryData<uint8_t>(document);
			layerMask.setMaskParams(maskParams);
			toRead -= 1u;
			toRead -= layerMask.readMaskParams(document);
		}

		layerMask.m_Size = layerMask.calculateSize();
		m_LayerMask.emplace(layerMask);
	}

	if (toRead < 0 || toRead > 2)
	{
		PSAPI_LOG_WARNING("LayerMaskData", "Expected either 0 or 2 padding bytes, got %i instead", toRead)
	}

	document.skip(toRead);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LayerRecords::LayerBlendingRanges::LayerBlendingRanges()
{
	// For some reason no matter the color mode this section is always 40 bytes long (Photoshop 23.3.2). 5 channels
	// Likely at some point it was decided that it was easiest to just hold the longest amount of possible combinations
	// as the size is quite trivial. Blending ranges for any non-default channels (default channels would be rgb in rgb
	// color mode or cmyk in cmyk color mode) cannot be blended and are therefore not considered.
	m_Size = 40u;	
	Data sourceRanges{};
	Data destinationRanges{};
	for (int i = 0; i < 5u; ++i)
	{
		// We just initialize defaults for no blending to take place
		std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> range = std::make_tuple<uint8_t, uint8_t, uint8_t, uint8_t>(0u, 0u, 255u, 255u);
		sourceRanges.push_back(range);
		destinationRanges.push_back(range);
	}
	m_SourceRanges = sourceRanges;
	m_DestinationRanges = destinationRanges;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerRecords::LayerBlendingRanges::calculateSize(std::optional<FileHeader> header) const
{
	uint64_t size = 0u;

	// The size should pretty much always evaluate to 44 but we do the calculations either way here in case that changes
	size += 4u;	// Size marker
	size += m_SourceRanges.size() * 4u;
	size += m_DestinationRanges.size() * 4u;

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerBlendingRanges::read(File& document)
{
	m_Size = ReadBinaryData<uint32_t>(document) + 4u;
	int32_t toRead = m_Size - 4u;

	// This appears to always be 5 different layer blending ranges. In photoshop (as of CC 23.3.2)
	// we only have control over Combined, Red, Green and Blue. My guess is that the 5th blending range is 
	// for the alpha channel
	while (toRead >= 8u)
	{
		uint8_t sourceLow1 = ReadBinaryData<uint8_t>(document);
		uint8_t sourceLow2 = ReadBinaryData<uint8_t>(document);
		uint8_t sourceHigh1 = ReadBinaryData<uint8_t>(document);
		uint8_t sourceHigh2 = ReadBinaryData<uint8_t>(document);

		m_SourceRanges.emplace_back(std::tuple(sourceLow1, sourceLow2, sourceHigh1, sourceHigh2));

		uint8_t destinationLow1 = ReadBinaryData<uint8_t>(document);
		uint8_t destinationLow2 = ReadBinaryData<uint8_t>(document);
		uint8_t destinationHigh1 = ReadBinaryData<uint8_t>(document);
		uint8_t destinationHigh2 = ReadBinaryData<uint8_t>(document);

		m_DestinationRanges.emplace_back(std::tuple(destinationLow1, destinationLow2, destinationHigh1, destinationHigh2));

		toRead -= 8u;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LayerRecord::LayerRecord()
{
	m_LayerName = PascalString("", 4u);
	m_Top = 0;
	m_Left = 0;
	m_Bottom = 0;
	m_Right = 0;
	m_ChannelCount = 0u;
	m_BlendMode = Enum::BlendMode::Normal;
	m_Opacity = 255u;
	m_Clipping = 1u;
	m_BitFlags = LayerRecords::BitFlags(false, true, false);
	m_LayerMaskData = std::nullopt;
	m_AdditionalLayerInfo = std::nullopt;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LayerRecord::LayerRecord(
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
	std::optional<AdditionalLayerInfo> additionalLayerInfo)
{
	m_LayerName = layerName;
	m_Top = top;
	m_Left = left;
	m_Bottom = bottom;
	m_Right = right;
	m_ChannelCount = channelCount;
	m_ChannelInformation = std::move(channelInfo);
	m_BlendMode = blendMode;
	m_Opacity = opacity;
	m_Clipping = clipping;
	m_BitFlags = bitFlags;
	if (layerMaskData.has_value())
		m_LayerMaskData.emplace(layerMaskData.value());
	m_LayerBlendingRanges = layerBlendingRanges;
	if (additionalLayerInfo.has_value())
		m_AdditionalLayerInfo.emplace(std::move(additionalLayerInfo.value()));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerRecord::calculateSize(std::optional<FileHeader> header) const
{
	uint64_t size = 0u;
	size += 16u;	// Enclosing rect
	size += 2u;		// Num of channels
	size += m_ChannelInformation.size() * 6u;	// Channel Information size per channel
	size += 4u;		// Blend mode signature
	size += 4u;		// Blend mode 
	size += 1u;		// Opacity
	size += 1u;		// Clipping
	size += 1u;		// Flags
	size += 1u;		// Filler byte
	size += 4u;		// Length of extra data 
	if (m_LayerMaskData.has_value())
		size += m_LayerMaskData.value().calculateSize();
	size += m_LayerBlendingRanges.calculateSize();
	size += m_LayerName.calculateSize();
	if (m_AdditionalLayerInfo.has_value())
		size += m_AdditionalLayerInfo.value().calculateSize();

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecord::read(File& document, const FileHeader& header, const uint64_t offset)
{
	PROFILE_FUNCTION();

	m_Offset = offset;
	document.setOffset(offset);

	m_Top = ReadBinaryData<int32_t>(document);
	m_Left = ReadBinaryData<int32_t>(document);
	m_Bottom = ReadBinaryData<int32_t>(document);
	m_Right = ReadBinaryData<int32_t>(document);

	m_Size = 16u;


	m_ChannelCount = ReadBinaryData<uint16_t>(document);
	if (m_ChannelCount > 56)
	{
		PSAPI_LOG_ERROR("LayerRecord", "A Photoshop document cannot have more than 56 channels at once")
	}
	m_ChannelInformation.reserve(m_ChannelCount);

	// Read the Channel Information, there is one of these for each channel in the layer record
	for (int i = 0; i < m_ChannelCount; i++)
	{
		LayerRecords::ChannelInformation channelInfo{};
		switch (header.m_ColorMode)
		{
		case Enum::ColorMode::RGB:
			channelInfo.m_ChannelID = Enum::rgbIntToChannelID(ReadBinaryData<uint16_t>(document));
			break;
		case Enum::ColorMode::CMYK:
			channelInfo.m_ChannelID = Enum::cmykIntToChannelID(ReadBinaryData<uint16_t>(document));
			break;
		case Enum::ColorMode::Grayscale:
			channelInfo.m_ChannelID = Enum::grayscaleIntToChannelID(ReadBinaryData<uint16_t>(document));
			break;
		default:
			int16_t index = ReadBinaryData<uint16_t>(document);
			PSAPI_LOG_WARNING("LayerRecord", "Currently unsupported ColorMode encountered, storing ChannelID::Custom")
				channelInfo.m_ChannelID = { Enum::ChannelID::Custom, index };
			break;
		}

		std::variant<uint32_t, uint64_t> size = ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version);
		channelInfo.m_Size = ExtractWidestValue<uint32_t, uint64_t>(size);
		m_ChannelInformation.emplace_back(channelInfo);

		// Size of one channel information section is 6 or 10 bytes
		m_Size += static_cast<uint64_t>(2u) + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version);
	}

	// Perform a signature check but do not store it as it isnt required
	Signature signature = Signature(ReadBinaryData<uint32_t>(document));
	if (signature != Signature("8BIM"))
	{
		PSAPI_LOG_ERROR("LayerRecord", "Signature does not match '8BIM', got '%s' instead",
			uint32ToString(signature.m_Value).c_str())
	}
	m_Size += 4u;
	
	std::string blendModeStr = uint32ToString(ReadBinaryData<uint32_t>(document));
	std::optional<Enum::BlendMode> blendMode = Enum::getBlendMode<std::string, Enum::BlendMode>(blendModeStr);
	if (blendMode.has_value())
	{
		m_BlendMode = blendMode.value();
	}
	else
	{
		m_BlendMode = Enum::BlendMode::Normal;
		PSAPI_LOG_ERROR("LayerRecord", "Got invalid blend mode: %s", blendModeStr.c_str());
	}
	m_Size += 4u;


	m_Opacity = ReadBinaryData<uint8_t>(document);
	m_Clipping = ReadBinaryData<uint8_t>(document);
	m_BitFlags = LayerRecords::BitFlags(ReadBinaryData<uint8_t>(document));

	document.skip(1u);	// Filler byte;
	m_Size += 4u;

	// This is the length of the next fields, we need this to find the length of the additional layer info
	const uint32_t extraDataLen = ReadBinaryData<uint32_t>(document);
	m_Size += 4u + static_cast<uint64_t>(extraDataLen);
	int32_t toRead = extraDataLen;
	{
		LayerRecords::LayerMaskData layerMaskSection = LayerRecords::LayerMaskData{};
		layerMaskSection.read(document);
		if (layerMaskSection.m_Size > 4u)
		{
			m_LayerMaskData.emplace(layerMaskSection);
			toRead -= layerMaskSection.m_Size;
		}
		else
		{
			toRead -= 4u;
		}
		m_LayerBlendingRanges.read(document);
		toRead -= m_LayerBlendingRanges.m_Size;

		m_LayerName.read(document, 4u);
		toRead -= m_LayerName.m_Size;

	}

	// A single tagged block takes at least 12 (or 16 for psb) bytes of memory. Therefore, if the remaining size is less than that we can ignore it
	if (toRead >= 12u)
	{
		AdditionalLayerInfo layerInfo = {};
		layerInfo.read(document, header, document.getOffset(), toRead, 1u);
		m_AdditionalLayerInfo.emplace((std::move(layerInfo)));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t ChannelImageData::calculateSize(std::optional<FileHeader> header) const
{
	uint64_t size = 0u;

	for (const auto& imgData : m_ImageData)
	{
		// TODO this implementation will need some more thinking as we dont know the size of the data until compressing it
	}

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ChannelImageData::read(ByteStream& stream, const FileHeader& header, const uint64_t offset, const LayerRecord& layerRecord)
{
	PROFILE_FUNCTION();

	m_Offset = offset;
	m_Size = 0;

	// Store the offsets into each of the channels, note that these are ByteStream offsets, not file offsets!
	std::vector<uint64_t> channelOffsets;
	uint64_t countingOffset = 0;
	for (const auto& channel : layerRecord.m_ChannelInformation)
	{
		channelOffsets.push_back(countingOffset);
		countingOffset += channel.m_Size;
	}

	// Preallocate the ImageData vector as we need valid indices for the for each loop
	m_ImageData.resize(layerRecord.m_ChannelInformation.size());

	// Iterate the channels in parallel
	std::for_each(std::execution::par, layerRecord.m_ChannelInformation.begin(), layerRecord.m_ChannelInformation.end(),
		[&](const LayerRecords::ChannelInformation& channel)
		{
			uint32_t index = &channel - &layerRecord.m_ChannelInformation[0];
			uint64_t channelOffset = channelOffsets[index];

			int32_t width = layerRecord.m_Right - layerRecord.m_Left;
			int32_t height = layerRecord.m_Bottom - layerRecord.m_Top;
			int32_t centerX = layerRecord.m_Left + width / 2;
			int32_t centerY = layerRecord.m_Top + height / 2;

			// If the channel is a mask the extents are actually stored in the layermaskdata
			if (channel.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask || channel.m_ChannelID.id == Enum::ChannelID::RealUserSuppliedLayerMask)
			{
				if (layerRecord.m_LayerMaskData.has_value() && layerRecord.m_LayerMaskData->m_LayerMask.has_value())
				{
					const LayerRecords::LayerMask mask = layerRecord.m_LayerMaskData.value().m_LayerMask.value();
					width = mask.m_Right - mask.m_Left;
					height = mask.m_Bottom - mask.m_Top;
					centerX = mask.m_Left + width / 2;
					centerY = mask.m_Top + height / 2;
				}
			}
			// Get the compression of the channel. We must read it this way as the offset has to be correct before parsing
			uint16_t compressionNum = 0;
			stream.setOffsetAndRead(reinterpret_cast<char*>(&compressionNum), channelOffset, sizeof(uint16_t));
			compressionNum = endianDecodeBE<uint16_t>(reinterpret_cast<const uint8_t*>(&compressionNum));
			Enum::Compression channelCompression = Enum::compressionMap.at(compressionNum);
			this->m_Size += channel.m_Size;

			if (header.m_Depth == Enum::BitDepth::BD_8)
			{
				std::vector<uint8_t> decompressedData = DecompressData<uint8_t>(stream, channelOffset + 2u, channelCompression, header, width, height, channel.m_Size - 2u);
				std::unique_ptr<ImageChannel<uint8_t>> channelPtr = std::make_unique<ImageChannel<uint8_t>>(channelCompression, decompressedData, channel.m_ChannelID, width, height, centerX, centerY);
				m_ImageData[index] = std::move(channelPtr);
			}
			else if (header.m_Depth == Enum::BitDepth::BD_16)
			{
				std::vector<uint16_t> decompressedData = DecompressData<uint16_t>(stream, channelOffset + 2u, channelCompression, header, width, height, channel.m_Size - 2u);
				std::unique_ptr<ImageChannel<uint16_t>> channelPtr = std::make_unique<ImageChannel<uint16_t>>(channelCompression, decompressedData, channel.m_ChannelID, width, height, centerX, centerY);
				m_ImageData[index] = std::move(channelPtr);
			}
			if (header.m_Depth == Enum::BitDepth::BD_32)
			{
				std::vector<float32_t> decompressedData = DecompressData<float32_t>(stream, channelOffset + 2u, channelCompression, header, width, height, channel.m_Size - 2u);
				std::unique_ptr<ImageChannel<float32_t>> channelPtr = std::make_unique<ImageChannel<float32_t>>(channelCompression, decompressedData, channel.m_ChannelID, width, height, centerX, centerY);
				m_ImageData[index] = std::move(channelPtr);
			}
		});
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerInfo::calculateSize(std::optional<FileHeader> header) const
{
	if (!header.has_value())
		PSAPI_LOG_ERROR("LayerInfo", "FileHeader is required to calculate the size of the LayerInfo section")

	uint64_t size = 0u;
	size += SwapPsdPsb<uint32_t, uint64_t>(header.value().m_Version);	// Size marker
	size += 2u;	// Layer count

	for (const auto& lr : m_LayerRecords)
	{
		size += lr.calculateSize();
	}
	for (const auto& lr : m_ChannelImageData)
	{
		size += lr.calculateSize();
	}
	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerInfo::read(File& document, const FileHeader& header, const uint64_t offset, const bool isFromAdditionalLayerInfo, std::optional<uint64_t> sectionSize)
{
	PROFILE_FUNCTION();

	m_Offset = offset;
	document.setOffset(offset);

	if (!isFromAdditionalLayerInfo)
	{
		// Read the layer info length marker which is 4 bytes in psd and 8 bytes in psb mode 
		// (note, this section is padded to 4 bytes which means we might have some padding bytes at the end)
		std::variant<uint32_t, uint64_t> size = ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version);
		m_Size = ExtractWidestValue<uint32_t, uint64_t>(size) + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version);
	}
	else if (isFromAdditionalLayerInfo && sectionSize.has_value())
	{
		// The reason for this specialization is that in 16 and 32 bit mode photoshop writes the layer info section
		// in a tagged block "Lr16" or "Lr32" which already has a size variable.
		m_Size = sectionSize.value();
	}
	else
	{
		PSAPI_LOG_ERROR("LayerInfo", "LayerInfo() expects an explicit section size if the call is from the additional layer information section");
	}

	// If this value is negative the first alpha channel of the layer records holds the merged image result (Image Data Section) alpha channel
	// TODO this isnt yet implemented
	uint16_t layerCount = static_cast<uint16_t>(std::abs(ReadBinaryData<int16_t>(document)));
	m_LayerRecords.reserve(layerCount);
	m_ChannelImageData.reserve(layerCount);

	// Extract layer records
	for (int i = 0; i < layerCount; i++)
	{
		LayerRecord layerRecord = {};
		layerRecord.read(document, header, document.getOffset());
		m_LayerRecords.push_back(std::move(layerRecord));
	}

	// Read the offsets and sizes of the channelImageData section ahead of time to later parallelize
	std::vector<uint64_t> channelImageDataOffsets;
	std::vector<uint64_t> channelImageDataSizes;
	uint64_t imageDataOffset = document.getOffset();
	for (const auto& layerRecord : m_LayerRecords)
	{
		// Push back the offsets first as we want the start of the section, not the end
		channelImageDataOffsets.push_back(imageDataOffset);

		uint64_t imageDataSize = 0u;
		for (const auto& channel : layerRecord.m_ChannelInformation)
		{
			imageDataOffset += channel.m_Size;
			imageDataSize += channel.m_Size;
		}
		channelImageDataSizes.push_back(imageDataSize);
	}

	std::vector<ChannelImageData> localResults(m_LayerRecords.size());
	// Extract Channel Image Data in parallel. Note that we perform this insertion in 
	std::for_each(m_LayerRecords.begin(), m_LayerRecords.end(), [&](const auto& layerRecord)
	{
		int index = &layerRecord - &m_LayerRecords[0];

		uint64_t tmpOffset = channelImageDataOffsets[index];
		uint64_t tmpSize = channelImageDataSizes[index];

		// Read the binary data. Note that this is done in one step to avoid the offset being set differently before 
		// reading the data. We also do this within the loop to avoid allocating all the memory at once
		ByteStream stream(document, tmpOffset, tmpSize);

		// Create the ChannelImageData by parsing the given buffer
		auto result = ChannelImageData();
		result.read(stream, header, tmpOffset, layerRecord);

		// As each index is unique we do not need to worry about locking the mutex here
		localResults[index] = std::move(result);
	});
	// Combine results after the loop
	m_ChannelImageData.insert(m_ChannelImageData.end(), std::make_move_iterator(localResults.begin()), std::make_move_iterator(localResults.end()));

	// Set the offset to where it is supposed to be as we cannot guarantee the location of the marker after jumping back and forth in image sections
	document.setOffset(imageDataOffset);

	const uint64_t expectedOffset = m_Offset + m_Size;
	if (document.getOffset() != expectedOffset)
	{
		int64_t toSkip = static_cast<int64_t>(expectedOffset) - static_cast<int64_t>(document.getOffset());
		// Check that the skipped bytes are within the amount needed to pad a LayerInfo section
		if (toSkip < -4 || toSkip > 4)
		{
			PSAPI_LOG_ERROR("LayerInfo", "Tried skipping bytes larger than the padding of the section: %i", toSkip)
		}
		document.setOffset(document.getOffset() + toSkip);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
int LayerInfo::getLayerIndex(const std::string& layerName)
{
	int count = 0;
	for (const LayerRecord& layer : m_LayerRecords)
	{
		if (layer.m_LayerName.m_String == layerName)
		{
			return count;
		}
		++count;
	}
	return -1;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void GlobalLayerMaskInfo::read(File& document, const uint64_t offset)
{
	m_Offset = offset;
	document.setOffset(offset);

	// As this section is undocumented, we currently just skip it.
	// TODO explore if this is relevant
	m_Size = static_cast<uint64_t>(ReadBinaryData<uint32_t>(document)) + 4u;
	document.skip(m_Size - 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerAndMaskInformation::calculateSize(std::optional<FileHeader> header) const
{
	if (!header.has_value())
		PSAPI_LOG_ERROR("LayerAndMaskInformation", "FileHeader is required to calculate the size of the LayerAndMaskInformation section")
	uint64_t size = 0u;
	size += SwapPsdPsb<uint32_t, uint64_t>(header.value().m_Version);	// Size marker
	
	size += m_LayerInfo.calculateSize(header);
	size += m_GlobalLayerMaskInfo.calculateSize();

	if (m_AdditionalLayerInfo.has_value())
		size += m_AdditionalLayerInfo.value().calculateSize();

	return size;
}


// Extract the layer and mask information section
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerAndMaskInformation::read(File& document, const FileHeader& header, const uint64_t offset)
{
	PROFILE_FUNCTION();

	m_Offset = offset;
	document.setOffset(offset);

	// Read the layer mask info length marker which is 4 bytes in psd and 8 bytes in psb mode
	// This value is
	std::variant<uint32_t, uint64_t> size = ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version);
	m_Size = ExtractWidestValue<uint32_t, uint64_t>(size);

	// Parse Layer Info Section
	{
		m_LayerInfo.read(document, header, document.getOffset());
		// Check the theoretical document offset against what was read by the layer info section. These should be identical
		if (document.getOffset() != (m_Offset + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version)) + m_LayerInfo.m_Size)
		{
			PSAPI_LOG_ERROR("LayerAndMaskInformation", "Layer Info read an incorrect amount of bytes from the document, expected an offset of %" PRIu64 ", but got %" PRIu64 " instead.",
				m_Offset + m_LayerInfo.m_Size + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version),
				document.getOffset())
		}
	}
	// Parse Global Layer Mask Info
	{
		m_GlobalLayerMaskInfo.read(document, document.getOffset());
	}

	int64_t toRead = m_Size - m_LayerInfo.m_Size - m_GlobalLayerMaskInfo.m_Size;
	// If there is still data left to read, this is the additional layer information which is also present at the end of each layer record
	if (toRead >= 12u)
	{
		// Tagged blocks at the end of the layer and mask information seem to be padded to 4-bytes
		AdditionalLayerInfo layerInfo = {};
		layerInfo.read(document, header, document.getOffset(), toRead, 4u);
		m_AdditionalLayerInfo.emplace((std::move(layerInfo)));
	}
}


PSAPI_NAMESPACE_END