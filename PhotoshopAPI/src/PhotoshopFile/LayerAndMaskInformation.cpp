#include "LayerAndMaskInformation.h"

#include "../Macros.h"
#include "../Util/Read.h"
#include "../Util/StringUtil.h"
#include "../Util/Struct/TaggedBlock.h"
#include "FileHeader.h"


#include <variant>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN


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
LayerRecords::LayerMaskData::LayerMaskData(File& document)
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
LayerRecords::LayerBlendingRanges::LayerBlendingRanges(File& document)
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
LayerRecord::LayerRecord(File& document, const FileHeader& header, const uint64_t offset)
{
	m_Offset = offset;
	document.setOffset(offset);

	m_Top = ReadBinaryData<uint32_t>(document);
	m_Left = ReadBinaryData<uint32_t>(document);
	m_Bottom = ReadBinaryData<uint32_t>(document);
	m_Right = ReadBinaryData<uint32_t>(document);

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
		channelInfo.m_ChannelID = Enum::intToChannelID(ReadBinaryData<uint16_t>(document));

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
	m_BitFlags = ReadBinaryData<uint8_t>(document);

	document.skip(1u);	// Filler byte;
	m_Size += 4u;

	// This is the length of the next fields, we need this to find the length of the additional layer info
	const uint32_t extraDataLen = ReadBinaryData<uint32_t>(document);
	m_Size += 4u + static_cast<uint64_t>(extraDataLen);
	int32_t toRead = extraDataLen;
	{
		LayerRecords::LayerMaskData layerMaskSection = LayerRecords::LayerMaskData(document);
		if (layerMaskSection.m_Size > 4u)
		{
			m_LayerMaskData.emplace(layerMaskSection);
			toRead -= layerMaskSection.m_Size;
		}
		else
		{
			toRead -= 4u;
		}

		m_LayerBlendingRanges = LayerRecords::LayerBlendingRanges(document);
		toRead -= m_LayerBlendingRanges.m_Size;

		m_LayerName = PascalString(document, 4u);
		toRead -= m_LayerName.m_Size;

	}

	// A single tagged block takes at least 12 (or 16) bytes of memory. Therefore, if the remaining size is less than that we can ignore it
	if (toRead >= 12u)
	{
		m_AdditionalLayerInfo.emplace(AdditionalLayerInfo(document, header, document.getOffset(), toRead, 1u));
	}
}


ChannelImageData::ChannelImageData(File& document, const FileHeader& header, const uint64_t offset, const std::vector<LayerRecords::ChannelInformation>& channelInfos)
{
	// TODO add this back in on parsing of image data
	PSAPI_UNUSED(header)

	m_Offset = offset;
	document.setOffset(offset);
	m_Size = 0;

	for (const auto& channel : channelInfos)
	{
		m_Compression[channel.m_ChannelID] = Enum::compressionMap.at(ReadBinaryData<uint16_t>(document));
		m_Data[channel.m_ChannelID] = std::vector<uint8_t>{};
		m_Size += channel.m_Size;	// The size holds the value of the compression marker
		document.skip(channel.m_Size - 2u);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
AdditionalLayerInfo::AdditionalLayerInfo(File& document, const FileHeader& header, const uint64_t offset, const uint64_t maxLength, const uint16_t padding)
{
	m_Offset = offset;
	document.setOffset(offset);
	m_Size = 0u;

	int64_t toRead = maxLength;
	while (toRead >= 12u)
	{
		std::unique_ptr<TaggedBlock::Base> taggedBlock = readTaggedBlock(document, header, padding);
		toRead -= taggedBlock->getTotalSize();
		m_Size += taggedBlock->getTotalSize();
		m_TaggedBlocks.push_back(std::move(taggedBlock));
	}
	if (toRead >= 0)
	{
		m_Size += toRead;
		document.skip(toRead);
		return;
	}

	if (toRead <= 0)
	{
		PSAPI_LOG_WARNING("AdditionalLayerInfo", "Read too much data for the additional layer info, was allowed %" PRIu64 " but read %" PRIu64 " instead",
			maxLength, maxLength - toRead);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LayerInfo::LayerInfo(File& document, const FileHeader& header, const uint64_t offset, const bool isFromAdditionalLayerInfo, std::optional<uint64_t> sectionSize)
{
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
		LayerRecord layerRecord = LayerRecord(document, header, document.getOffset());
		m_LayerRecords.push_back(std::move(layerRecord));
	}

	// Extract Channel Image Data
	for (int i = 0; i < layerCount; i++)
	{
		ChannelImageData channelImageData = ChannelImageData(document, header, document.getOffset(), m_LayerRecords[i].m_ChannelInformation);
		m_ChannelImageData.push_back(std::move(channelImageData));
	}

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
GlobalLayerMaskInfo::GlobalLayerMaskInfo(File& document, const uint64_t offset)
{
	m_Offset = offset;
	document.setOffset(offset);

	// As this section is undocumented, we currently just skip it.
	// TODO explore if this is relevant
	m_Size = static_cast<uint64_t>(ReadBinaryData<uint32_t>(document)) + 4u;
	document.skip(m_Size - 4u);
}


// Extract the layer and mask information section
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool LayerAndMaskInformation::read(File& document, const FileHeader& header, const uint64_t offset)
{
	m_Offset = offset;
	document.setOffset(offset);

	// Read the layer mask info length marker which is 4 bytes in psd and 8 bytes in psb mode
	// This value is
	std::variant<uint32_t, uint64_t> size = ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version);
	m_Size = ExtractWidestValue<uint32_t, uint64_t>(size);

	// Parse Layer Info Section
	{
		m_LayerInfo = LayerInfo(document, header, document.getOffset());
		// Check the theoretical document offset against what was read by the layer info section. These should be identical
		if (document.getOffset() != (m_Offset + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version)) + m_LayerInfo.m_Size)
		{
			PSAPI_LOG_ERROR("LayerAndMaskInformation", "Layer Info read an incorrect amount of bytes from the document, expected an offset of %" PRIu64 ", but got %" PRIu64 " instead.",
				m_Offset + m_LayerInfo.m_Size + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version),
				document.getOffset())
			return false;
		}
	}
	// Parse Global Layer Mask Info
	{
		m_GlobalLayerMaskInfo = GlobalLayerMaskInfo(document, document.getOffset());
	}

	int64_t toRead = m_Size - m_LayerInfo.m_Size - m_GlobalLayerMaskInfo.m_Size;
	// If there is still data left to read, this is the additional layer information which is also present at the end of each layer record
	if (toRead >= 12u)
	{
		// Tagged blocks at the end of the layer and mask information seem to be padded to 4-bytes
		m_AdditionalLayerInfo.emplace(AdditionalLayerInfo(document, header, document.getOffset(), toRead, 4u));
	}

	return true;
}


PSAPI_NAMESPACE_END