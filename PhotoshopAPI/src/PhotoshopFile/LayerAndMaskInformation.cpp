#include "LayerAndMaskInformation.h"

#include "FileHeader.h"
#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Core/FileIO/Util.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include "Profiling/Perf/Instrumentor.h"

#include "libdeflate.h"

#include <variant>
#include <algorithm>
#include <execution>
#include <limits>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::BitFlags::setFlags(const uint8_t flags) noexcept
{
	m_isTransparencyProtected = (flags & m_transparencyProtectedMask) != 0;
	m_isHidden = (flags & m_hiddenMask) != 0;
	// Bit 2 holds no relevant information
	m_isBit4Useful = (flags & m_bit4UsefulMask) != 0;
	m_isPixelDataIrrelevant = (flags & m_pixelDataIrrelevantMask) != 0 && m_isBit4Useful;
	// bit 5-7 holds no data (according to the documentation)
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint8_t LayerRecords::BitFlags::getFlags() const noexcept
{
	uint8_t result = 0u;

	if (m_isTransparencyProtected)
		result |= m_transparencyProtectedMask;
	if (m_isHidden)
		result |= m_hiddenMask;
	if (m_isBit4Useful)
		result |= m_bit4UsefulMask;
	if (m_isPixelDataIrrelevant)
		result |= m_pixelDataIrrelevantMask;

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
LayerRecords::BitFlags::BitFlags(const bool isTransparencyProtected, const bool isHidden, const bool isPixelDataIrrelevant)
{
	m_isTransparencyProtected = isTransparencyProtected;
	m_isHidden = isHidden;
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
uint64_t LayerRecords::LayerMask::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
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
void LayerRecords::LayerMask::setFlags(const uint8_t bitFlag)
{
	m_PositionRelativeToLayer = (bitFlag & m_PositionRelativeToLayerMask) != 0;
	m_Disabled = (bitFlag & m_DisabledMask) != 0;
	m_IsVector = (bitFlag & m_IsVectorMask) != 0;
	m_HasMaskParams = (bitFlag & m_HasMaskParamsMask) != 0;

	// We do need to pass this through for roundtripping
	m_unknownBit2 = (bitFlag & m_unknownBit2Mask) != 0;
	m_unknownBit5 = (bitFlag & m_unknownBit5Mask) != 0;
	m_unknownBit6 = (bitFlag & m_unknownBit6Mask) != 0;
	m_unknownBit7 = (bitFlag & m_unknownBit7Mask) != 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint8_t LayerRecords::LayerMask::getFlags() const noexcept
{
	uint8_t bitFlags = 0u;

	if (m_PositionRelativeToLayer) 
		bitFlags |= m_PositionRelativeToLayerMask;
	if (m_Disabled)
		bitFlags |= m_DisabledMask;
	if (m_IsVector)
		bitFlags |= m_IsVectorMask;
	if (m_HasMaskParams)
		bitFlags |= m_HasMaskParamsMask;
	
	if (m_unknownBit2)
		bitFlags |= m_unknownBit2Mask;
	if (m_unknownBit5)
		bitFlags |= m_unknownBit5Mask;
	if (m_unknownBit6)
		bitFlags |= m_unknownBit6Mask;
	if (m_unknownBit7)
		bitFlags |= m_unknownBit7Mask;

	return bitFlags;
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerMask::setMaskParams(const uint8_t bitFlag)
{
	m_HasUserMaskDensity = (bitFlag & m_UserMaskDensityMask) != 0;
	m_HasUserMaskFeather = (bitFlag & m_UserMaskFeatherMask) != 0;
	m_HasVectorMaskDensity = (bitFlag & m_VectorMaskDensityMask) != 0;
	m_HasVectorMaskFeather = (bitFlag & m_VectorMaskFeatherMask) != 0;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint8_t LayerRecords::LayerMask::getMaskParams() const noexcept
{
	uint8_t bitFlags = 0u;

	if (m_HasUserMaskDensity)
		bitFlags |= m_UserMaskDensityMask;
	if (m_HasUserMaskFeather)
		bitFlags |= m_UserMaskFeatherMask;
	if (m_HasVectorMaskDensity)
		bitFlags |= m_VectorMaskDensityMask;
	if (m_HasVectorMaskFeather)
		bitFlags |= m_VectorMaskFeatherMask;

	return bitFlags;
}


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
uint32_t LayerRecords::LayerMask::writeMaskParams(File& document) const noexcept
{
	uint32_t sizeWritten = 0u;
	if (m_HasUserMaskDensity)
	{
		WriteBinaryData<uint8_t>(document, m_UserMaskDensity.value());
		sizeWritten += 1u;
	}
	if (m_HasUserMaskFeather)
	{
		WriteBinaryData<float64_t>(document, m_UserMaskFeather.value());
		sizeWritten += 8u;
	}
	if (m_HasVectorMaskDensity)
	{
		WriteBinaryData<uint8_t>(document, m_VectorMaskDensity.value());
		sizeWritten += 1u;
	}
	if (m_HasVectorMaskFeather)
	{
		WriteBinaryData<float64_t>(document, m_VectorMaskFeather.value());
		sizeWritten += 8u;
	}
	return sizeWritten;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerRecords::LayerMaskData::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
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
	
	// It appears as though this section is just padded to 4-bytes regardless of
	// section lengths
	RoundUpToMultiple<uint64_t>(size, 4u);

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerMaskData::read(File& document)
{
	FileSection::size(static_cast<uint64_t>(ReadBinaryData<uint32_t>(document)) + 4u);
	int64_t toRead = FileSection::size<int64_t>() - 4u;

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

		mask.m_DefaultColor = ReadBinaryData<uint8_t>(document);
		if (mask.m_DefaultColor != 0 && mask.m_DefaultColor != 255)
		{
			PSAPI_LOG_ERROR("LayerMaskData", "Layer Mask default color can only be 0 or 255, not %u", mask.m_DefaultColor);
		}
		toRead -= 1u;


		const uint8_t bitFlags = ReadBinaryData<uint8_t>(document);
		mask.setFlags(bitFlags);
		toRead -= 1u;

		// Store this value to compare against later
		hasMaskParams = mask.m_HasMaskParams;
		if (hasMaskParams && FileSection::size() <= 28)
		{
			const uint8_t maskParams = ReadBinaryData<uint8_t>(document);
			mask.setMaskParams(maskParams);
			toRead -= 1u;
			toRead -= mask.readMaskParams(document);
		}

		mask.size(mask.calculateSize());
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

		layerMask.size(layerMask.calculateSize());
		m_LayerMask.emplace(layerMask);
	}

	if (toRead < 0 || toRead > 2)
	{
		PSAPI_LOG_WARNING("LayerMaskData", "Expected either 0 or 2 padding bytes, got %i instead", toRead);
	}

	document.skip(toRead);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerMaskData::write(File& document) const
{
	auto size = this->calculateSize();
	assert(size < std::numeric_limits<uint32_t>::max());
	uint32_t sizeWritten = 0u;

	// Section size marker
	WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(size - 4u));
	
	if (m_LayerMask && m_VectorMask)
	{
		PSAPI_LOG_WARNING("LayerMaskData", "Having two masks is currently unsupported by the PhotoshopAPI, currently only pixel masks are supported.");
	}
	else if (m_LayerMask)
	{
		const auto& lrMask = m_LayerMask.value();
		WriteBinaryData<int32_t>(document, lrMask.m_Top);
		WriteBinaryData<int32_t>(document, lrMask.m_Left);
		WriteBinaryData<int32_t>(document, lrMask.m_Bottom);
		WriteBinaryData<int32_t>(document, lrMask.m_Right);
		sizeWritten += 16u;
		WriteBinaryData<uint8_t>(document, lrMask.m_DefaultColor);
		sizeWritten += 1u;
		WriteBinaryData<uint8_t>(document, lrMask.getFlags());
		sizeWritten += 1u;
		if (lrMask.m_HasMaskParams)
		{
			WriteBinaryData<uint8_t>(document, lrMask.getMaskParams());
			sizeWritten += 1u;
			sizeWritten += lrMask.writeMaskParams(document);
		}
	}

	// Pad the section to 4 bytes
	if (size - 4u > sizeWritten)
		WritePadddingBytes(document, size - 4u - sizeWritten);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LayerRecords::LayerBlendingRanges::LayerBlendingRanges()
{
	// For some reason no matter the color mode this section is always 40 bytes long (Photoshop 23.3.2) which is 5 channels
	// Likely at some point it was decided that it was easiest to just hold the longest amount of possible combinations
	// as the size is quite trivial. Blending ranges for any non-default channels (default channels would be rgb in rgb
	// color mode or cmyk in cmyk color mode) cannot be blended and are therefore not considered.
	FileSection::size(44u);	// Include the section marker	
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
uint64_t LayerRecords::LayerBlendingRanges::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
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
	FileSection::size(static_cast<uint64_t>(ReadBinaryData<uint32_t>(document)) + 4u);
	int32_t toRead = FileSection::size<int32_t>() - 4u;

	// This appears to always be 5 different layer blending ranges. In photoshop (as of CC 23.3.2)
	// we only have control over Combined, Red, Green and Blue. My guess is that the 5th blending range is 
	// for the alpha channel
	while (toRead >= 8u)
	{
		uint8_t sourceLow1 = ReadBinaryData<uint8_t>(document);
		uint8_t sourceLow2 = ReadBinaryData<uint8_t>(document);
		uint8_t sourceHigh1 = ReadBinaryData<uint8_t>(document);
		uint8_t sourceHigh2 = ReadBinaryData<uint8_t>(document);

		m_SourceRanges.push_back(std::tuple(sourceLow1, sourceLow2, sourceHigh1, sourceHigh2));

		uint8_t destinationLow1 = ReadBinaryData<uint8_t>(document);
		uint8_t destinationLow2 = ReadBinaryData<uint8_t>(document);
		uint8_t destinationHigh1 = ReadBinaryData<uint8_t>(document);
		uint8_t destinationHigh2 = ReadBinaryData<uint8_t>(document);

		m_DestinationRanges.push_back(std::tuple(destinationLow1, destinationLow2, destinationHigh1, destinationHigh2));

		toRead -= 8u;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecords::LayerBlendingRanges::write(File& document) const
{
	// Write the size marker
	WriteBinaryData<uint32_t>(document, FileSection::size<uint32_t>() - 4u);

	if (m_SourceRanges.size() != m_DestinationRanges.size()) [[unlikely]]
	{
			PSAPI_LOG_ERROR("LayerBlendingRanges", "Source and Destination ranges must have the exact same size, source range size : %i, destination range size : %i",
				m_SourceRanges.size(), m_DestinationRanges.size());
	}

	for (int i = 0; i < m_SourceRanges.size(); ++i)
	{
		WriteBinaryData<uint8_t>(document, std::get<0>(m_SourceRanges[i]));
		WriteBinaryData<uint8_t>(document, std::get<1>(m_SourceRanges[i]));
		WriteBinaryData<uint8_t>(document, std::get<2>(m_SourceRanges[i]));
		WriteBinaryData<uint8_t>(document, std::get<3>(m_SourceRanges[i]));

		WriteBinaryData<uint8_t>(document, std::get<0>(m_DestinationRanges[i]));
		WriteBinaryData<uint8_t>(document, std::get<1>(m_DestinationRanges[i]));
		WriteBinaryData<uint8_t>(document, std::get<2>(m_DestinationRanges[i]));
		WriteBinaryData<uint8_t>(document, std::get<3>(m_DestinationRanges[i]));
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
uint64_t LayerRecord::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	if (!header)
	{
		PSAPI_LOG_ERROR("LayerRecord", "calculateSize() function requires the header to be passed");
	}

	uint64_t size = 0u;
	size += 16u;	// Enclosing rect
	size += 2u;		// Num of channels
	size += m_ChannelInformation.size() * (SwapPsdPsb<uint32_t, uint64_t>(header->m_Version) + 2u);	// Channel Information size per channel
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
void LayerRecord::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset)
{
	PSAPI_PROFILE_FUNCTION();

	FileSection::initialize(offset, 16u);
	document.setOffset(offset);

	m_Top = ReadBinaryData<int32_t>(document);
	m_Left = ReadBinaryData<int32_t>(document);
	m_Bottom = ReadBinaryData<int32_t>(document);
	m_Right = ReadBinaryData<int32_t>(document);


	m_ChannelCount = ReadBinaryData<uint16_t>(document);
	if (m_ChannelCount > 56)
	{
		PSAPI_LOG_ERROR("LayerRecord", "A Photoshop document cannot have more than 56 channels at once");
	}

	// Read the Channel Information, there is one of these for each channel in the layer record
	for (int i = 0; i < m_ChannelCount; i++)
	{
		LayerRecords::ChannelInformation channelInfo{};
		auto index = ReadBinaryData<int16_t>(document);
		channelInfo.m_ChannelID = Enum::toChannelIDInfo(index, header.m_ColorMode);

		std::variant<uint32_t, uint64_t> size = ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version);
		channelInfo.m_Size = ExtractWidestValue<uint32_t, uint64_t>(size);

		// Size of one channel information section is 6 or 10 bytes
		FileSection::size(FileSection::size() + static_cast<uint64_t>(2u) + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version));
		m_ChannelInformation.push_back(channelInfo);
	}

	// Perform a signature check but do not store it as it isnt required
	Signature signature = Signature(ReadBinaryData<uint32_t>(document));
	if (signature != Signature("8BIM"))
	{
		PSAPI_LOG_ERROR("LayerRecord", "Signature does not match '8BIM', got '%s' instead",
			uint32ToString(signature.m_Value).c_str());
	}
	FileSection::size(FileSection::size() + 4u);
	
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
	FileSection::size(FileSection::size() + 4u);


	m_Opacity = ReadBinaryData<uint8_t>(document);
	m_Clipping = ReadBinaryData<uint8_t>(document);
	m_BitFlags = LayerRecords::BitFlags(ReadBinaryData<uint8_t>(document));

	document.skip(1u);	// Filler byte;
	FileSection::size(FileSection::size() + 4u);

	// This is the length of the next fields, we need this to find the length of the additional layer info
	const uint32_t extraDataLen = ReadBinaryData<uint32_t>(document);
	FileSection::size(FileSection::size() + 4u + extraDataLen);
	int32_t toRead = extraDataLen;
	{
		LayerRecords::LayerMaskData layerMaskSection = LayerRecords::LayerMaskData{};
		layerMaskSection.read(document);
		if (layerMaskSection.size() > 4u)
		{
			m_LayerMaskData.emplace(layerMaskSection);
			toRead -= layerMaskSection.size<int32_t>();
		}
		else
		{
			toRead -= 4u;
		}
		m_LayerBlendingRanges.read(document);
		toRead -= m_LayerBlendingRanges.size<int32_t>();

		m_LayerName.read(document, 4u);
		toRead -= m_LayerName.size<int32_t>();

	}

	// A single tagged block takes at least 12 (or 16 for psb) bytes of memory. Therefore, if the remaining size is less than that we can ignore it
	if (toRead >= 12u)
	{
		AdditionalLayerInfo layerInfo = {};
		layerInfo.read(document, header, callback, document.getOffset(), toRead, 1u);
		m_AdditionalLayerInfo.emplace((std::move(layerInfo)));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerRecord::write(File& document, const FileHeader& header, ProgressCallback& callback, std::vector<LayerRecords::ChannelInformation> channelInfos) const
{
	WriteBinaryData<uint32_t>(document, m_Top);
	WriteBinaryData<uint32_t>(document, m_Left);
	WriteBinaryData<uint32_t>(document, m_Bottom);
	WriteBinaryData<uint32_t>(document, m_Right);

	if (m_ChannelCount > 56)
		PSAPI_LOG_ERROR("LayerRecord", "Maximum channel count is 56 for a given layer, got %i", m_ChannelCount);
	WriteBinaryData<uint16_t>(document, m_ChannelCount);

	if (channelInfos.size() != m_ChannelCount)
		PSAPI_LOG_ERROR("LayerRecord", "The provided channelInfo vec does not have the same amount of channels as m_ChanneCount, expected %i but got %i instead",
			m_ChannelCount, channelInfos.size());
	for (const auto& info : channelInfos)
	{
		WriteBinaryData<int16_t>(document, info.m_ChannelID.index);
		WriteBinaryDataVariadic<uint32_t, uint64_t>(document, info.m_Size, header.m_Version);
	}

	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	std::optional<std::string> blendModeStr = Enum::getBlendMode<Enum::BlendMode, std::string>(m_BlendMode);
	if (blendModeStr.has_value())
		WriteBinaryData<uint32_t>(document, Signature(blendModeStr.value()).m_Value);
	else
		PSAPI_LOG_ERROR("LayerRecord", "Could not identify a blend mode string from the given key");


	WriteBinaryData<uint8_t>(document, m_Opacity);
	if (m_Clipping > 1)
		PSAPI_LOG_ERROR("LayerRecord", "'Clipping' variable must be 0 or 1, not %u", m_Clipping);
	WriteBinaryData<uint8_t>(document, m_Clipping);

	WriteBinaryData<uint8_t>(document, m_BitFlags.getFlags());
	WriteBinaryData<uint8_t>(document, 0u);	// Filler byte

	// Write the extra data here which the official docs refer to as 5 sections but is in reality 4 (LayerMaskData, LayerBlendingRanges, LayerName, AdditionalLayerInfo)
	{
		// Keep in mind that these individual sections will already be padded to their respective size so we dont need to worry about padding
		size_t extraDataSize = 0u;
		{
			if (m_LayerMaskData.has_value())
			{
				extraDataSize += m_LayerMaskData.value().calculateSize();
			}
			else
			{
				extraDataSize += 4u;	// Explicit size marker
			}
			extraDataSize += m_LayerBlendingRanges.calculateSize();
			extraDataSize += m_LayerName.calculateSize();
			if (m_AdditionalLayerInfo)
			{
				extraDataSize += m_AdditionalLayerInfo.value().calculateSize();
			}
		}
		assert(extraDataSize < std::numeric_limits<uint32_t>::max());
		WriteBinaryData<uint32_t>(document, RoundUpToMultiple<uint32_t>(static_cast<uint32_t>(extraDataSize), 2u));

		// We must explicitly write an empty section size if this is not present
		if (m_LayerMaskData)
		{
			m_LayerMaskData.value().write(document);
		}
		else
		{
			WriteBinaryData<uint32_t>(document, 0u);
		}
		m_LayerBlendingRanges.write(document);
		m_LayerName.write(document);

		if (m_AdditionalLayerInfo.has_value())
		{
			m_AdditionalLayerInfo.value().write(document, header, callback);
		}

		// The additional data is aligned to 2 bytes
		WritePadddingBytes(document, RoundUpToMultiple(static_cast<uint32_t>(extraDataSize), 2u) - extraDataSize);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint32_t LayerRecord::getWidth() const noexcept
{
	return static_cast<uint32_t>(m_Right - m_Left);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint32_t LayerRecord::getHeight() const noexcept
{
	return static_cast<uint32_t>(m_Bottom - m_Top);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t ChannelImageData::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	uint64_t size = 0u;

	PSAPI_LOG_WARNING("ChannelImageData", "Unable to compute size of channelImageData due to the size only being known at export time, please refrain from using this function");

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
uint64_t ChannelImageData::estimateSize(const FileHeader& header, const uint16_t numSamples)
{
	uint64_t estimatedSize = 0u;

	for (const auto& channel : m_ImageData)
	{
		auto imageChannelPtr = channel.get();
		if (!imageChannelPtr)
		{
			PSAPI_LOG_WARNING("ChannelImageData", "Unable to read data from channel '%i'", channel->m_ChannelID.id);
			continue;
		}

		if (imageChannelPtr->m_Compression == Enum::Compression::Raw)
		{
			// We can just get the actual byte size making the estimate entirely accurate
			estimatedSize += imageChannelPtr->m_OrigByteSize;
			continue;
		}

		// Extract a number of sample regions from the image that are chosen at random,
		// we will now compress them according to the channels compression codec and add the size to 
		// the total size multiplying by the number of chunks divided by our number of samples
		auto channelData = imageChannelPtr->getRandomChunks<T>(header, numSamples);
		for (const auto& sample : channelData)
		{
			if (imageChannelPtr->m_Compression == Enum::Compression::Rle)
			{
				// We want to just compress as a single row to avoid any issues regarding the rows being cut off etc.
				auto tmp = CompressData(sample, Enum::Compression::Rle, header, sample.size(), 1u);
				// Subtract 2/4 bytes for the scanline size stored at the end of the data section
				estimatedSize += (tmp.size() * sizeof(T) - SwapPsdPsb<uint16_t, uint32_t>(header.m_Version)) * (imageChannelPtr->getNumChunks() / numSamples);
			}
			else if (imageChannelPtr->m_Compression == Enum::Compression::Zip)
			{
				// We want to just compress as a single row to avoid any issues regarding the rows being cut off etc.
				auto tmp = CompressData(sample, Enum::Compression::Zip, header, sample.size(), 1u);
				// Subtract 5 bytes to remove any header information 
				estimatedSize += (tmp.size() * sizeof(T) - 5u) * (imageChannelPtr->getNumChunks() / numSamples);
			}
			else if (imageChannelPtr->m_Compression == Enum::Compression::ZipPrediction)
			{
				// We want to just compress as a single row to avoid any issues regarding the rows being cut off etc.
				auto tmp = CompressData(sample, Enum::Compression::Zip, header, sample.size(), 1u);
				// Subtract 5 bytes to remove any header information 
				estimatedSize += (tmp.size() * sizeof(T) - 5u) * (imageChannelPtr->getNumChunks() / numSamples);
			}
		}
	}

	return estimatedSize;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<std::vector<uint8_t>> ChannelImageData::compressData(const FileHeader& header, std::vector<LayerRecords::ChannelInformation>& lrChannelInfo, std::vector<Enum::Compression>& lrCompression, size_t numThreads)
{
	PSAPI_PROFILE_FUNCTION();

	if (lrChannelInfo.size() != 0 || lrCompression.size() != 0) [[unlikely]]
	{
		PSAPI_LOG_ERROR("ChannelImage", "lrChannelInfo and lrCompression vectors must both be empty as allocation occurs in compressData()");
	}

	std::vector<std::vector<uint8_t>> compressedData;
	compressedData.reserve(m_ImageData.size());

	// We create a scratch buffer here which we use to store data for compression since we need a sufficiently large buffer to compress into but
	// then at the end want to shrink to the desired size. So here we create one that can accomodate any level of compression and then finally copy
	// the buffers out after decompression in order to not allocate the buffer at each step of the way
	std::vector<uint8_t> buffer;
	libdeflate_compressor* compressor = libdeflate_alloc_compressor(ZIP_COMPRESSION_LVL);
	{
		PSAPI_PROFILE_SCOPE("Allocate compression buffer");
		size_t maxWidth = 0;
		size_t maxHeight = 0;
		for (const auto& channel : m_ImageData)
		{
			size_t width = channel->getWidth();
			size_t height = channel->getHeight();
			if (width > maxWidth)
				maxWidth = width;
			if (height > maxHeight)
				maxHeight = height;
		}

		// We filter RLE early here if its not present since RLE at worst has a compression overhead of ~33% while ZIP has
		// <1% so we dont want to spend time allocating a buffer we wont need
		bool hasRLE = false;
		for (const auto& channel : m_ImageData)
		{
			if (channel->m_Compression == Enum::Compression::Rle)
				hasRLE = true;
		}
		// Compute the maximum necessary size to fit all of our compression needs and fill the buffer with that information
		if (hasRLE)
		{
			size_t maxRLESize = RLE_Impl::MaxCompressedSize<T>(header, maxHeight, maxWidth);
			size_t maxZIPSize = libdeflate_zlib_compress_bound(compressor, static_cast<uint64_t>(maxWidth) * maxHeight * sizeof(T));
			size_t maxCompressedSize = std::max(maxRLESize, maxZIPSize);
			buffer = std::vector<uint8_t>(maxCompressedSize);
		}
		else
		{
			size_t maxZIPSize = libdeflate_zlib_compress_bound(compressor, static_cast<uint64_t>(maxWidth) * maxHeight * sizeof(T));
			buffer = std::vector<uint8_t>(maxZIPSize);
		}
	}

	// Allocate a buffer we can use as scratch for the channel extraction that way we dont have to regenerate a buffer for each iteration.
	// If we ever change back for doing the compression per-channel in parallel we have to get rid of this again
	size_t maxSize = 0;
	for (const auto& channel : m_ImageData)
	{
		size_t size = static_cast<size_t>(channel->getWidth()) * channel->getHeight();
		if (maxSize < size)
		{
			maxSize = size;
		}
	}
	std::vector<T> channelDataBuffer;
	{
		PSAPI_PROFILE_SCOPE("Allocate channel buffer");
		channelDataBuffer = std::vector<T>(maxSize);
	}

	for (int i = 0; i < m_ImageData.size(); ++i)
	{
		// Take ownership of and invalidate the current channel index
		std::unique_ptr<ImageChannel> imageChannelPtr = std::move(m_ImageData[i]);
		if (imageChannelPtr == nullptr) [[unlikely]]
		{
			PSAPI_LOG_WARNING("ChannelImageData", "Channel %i no longer contains any data, was it extracted beforehand?", i);
			return std::vector<std::vector<uint8_t>>();
		}
		m_ImageData[i] = nullptr;

		const auto& width = imageChannelPtr->getWidth();
		const auto& height = imageChannelPtr->getHeight();
		auto& compressionMode = imageChannelPtr->m_Compression;
		const auto& channelIdx = imageChannelPtr->m_ChannelID;

		// In 32-bit mode Photoshop insists on the data being prediction encoded even if the compression mode is set to zip
		// to probably get better compression. We warn the user of this and switch to ZipPrediction
		if constexpr (std::is_same_v<T, float32_t>)
		{
			if (compressionMode == Enum::Compression::Zip)
			{
				PSAPI_LOG("ChannelImageData", "Photoshop insists on ZipPrediction encoded data rather than Zip for 32-bit, switching to ZipPrediction");
				compressionMode = Enum::Compression::ZipPrediction;
			}
		}

		// Construct a span from our buffer that is exactly sized to make the CompressData calls behave correctly. The wh
		std::span<T> channelDataSpan = std::span<T>(channelDataBuffer.begin(), channelDataBuffer.begin() + static_cast<size_t>(width) * height);

		// Compress the image data into a binary array and store it in our compressedData vec
		imageChannelPtr->getData<T>(channelDataSpan, numThreads);
		compressedData.push_back(CompressData(channelDataSpan, buffer, compressor, compressionMode, header, width, height));

		// Store our additional data. The size of the channel must include the 2 bytes for the compression marker
		LayerRecords::ChannelInformation channelInfo{.m_ChannelID = channelIdx, .m_Size = compressedData[i].size() + 2u };
		lrChannelInfo.push_back(channelInfo);
		lrCompression.push_back(compressionMode);
	}

	libdeflate_free_compressor(compressor);

	return compressedData;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ChannelImageData::read(ByteStream& stream, const FileHeader& header, const uint64_t offset, const LayerRecord& layerRecord)
{
	PSAPI_PROFILE_FUNCTION();

	FileSection::initialize(offset, 0u);

	// Store the offsets into each of the channels, note that these are ByteStream offsets, not file offsets!
	std::vector<uint64_t> channelOffsets;
	uint64_t countingOffset = 0;
	for (const auto& channel : layerRecord.m_ChannelInformation)
	{
		m_ChannelOffsetsAndSizes.push_back(std::tuple<uint64_t, uint64_t>(offset + countingOffset, channel.m_Size));
		channelOffsets.push_back(countingOffset);
		countingOffset += channel.m_Size;
	}

	// Allocate a binary vector for the maximum extents such that we can then reuse the buffer in the loops rather than reallocating memory
	ChannelCoordinates extent = generateChannelCoordinates(ChannelExtents(layerRecord.m_Top, layerRecord.m_Left, layerRecord.m_Bottom, layerRecord.m_Right), header);
	uint32_t maxWidth = extent.width;
	uint32_t maxHeight = extent.height;
	if (layerRecord.m_LayerMaskData)
	{
		if (layerRecord.m_LayerMaskData.value().m_LayerMask)
		{
			const LayerRecords::LayerMask mask = layerRecord.m_LayerMaskData.value().m_LayerMask.value();
			// Generate our coordinates from the mask extents instead
			ChannelCoordinates lrMask = generateChannelCoordinates(ChannelExtents(mask.m_Top, mask.m_Left, mask.m_Bottom, mask.m_Right), header);
			if (static_cast<uint32_t>(lrMask.width) > maxWidth)
			{
				maxWidth = static_cast<uint32_t>(lrMask.width);
			}
			if (static_cast<uint32_t>(lrMask.height) > maxHeight)
			{
				maxHeight = static_cast<uint32_t>(lrMask.height);
			}
		}
	}
	std::vector<uint8_t> buffer;
	if (header.m_Depth == Enum::BitDepth::BD_8)
	{
		buffer = std::vector<uint8_t>(maxWidth * maxHeight * sizeof(uint8_t));
	}
	else if (header.m_Depth == Enum::BitDepth::BD_16)
	{
		buffer = std::vector<uint8_t>(maxWidth * maxHeight * sizeof(uint16_t));
	}
	else if (header.m_Depth == Enum::BitDepth::BD_32)
	{
		buffer = std::vector<uint8_t>(maxWidth * maxHeight * sizeof(float32_t));
	}


	// Preallocate the ImageData vector as we need valid indices for the for each loop
	m_ImageData.resize(layerRecord.m_ChannelInformation.size());
	m_ChannelCompression.resize(layerRecord.m_ChannelInformation.size());

	// Iterate the channels and decompress after which we generate the image channels.
	// uses the 'buffer' as an intermediate memory area
	for (const auto& channel : layerRecord.m_ChannelInformation)
	{
		const size_t index = &channel - &layerRecord.m_ChannelInformation[0];
		const uint64_t channelOffset = channelOffsets[index];

		// Generate our coordinates from the layer extents
		ChannelCoordinates coordinates = generateChannelCoordinates(ChannelExtents(layerRecord.m_Top, layerRecord.m_Left, layerRecord.m_Bottom, layerRecord.m_Right), header);

		// If the channel is a mask the extents are actually stored in the layermaskdata
		if (channel.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask || channel.m_ChannelID.id == Enum::ChannelID::RealUserSuppliedLayerMask)
		{
			if (layerRecord.m_LayerMaskData.has_value() && layerRecord.m_LayerMaskData->m_LayerMask.has_value())
			{
				const LayerRecords::LayerMask mask = layerRecord.m_LayerMaskData.value().m_LayerMask.value();
				// Generate our coordinates from the mask extents instead
				coordinates = generateChannelCoordinates(ChannelExtents(mask.m_Top, mask.m_Left, mask.m_Bottom, mask.m_Right), header);
			}
		}
		// Get the compression of the channel. We must read it this way as the offset has to be correct before parsing
		Enum::Compression channelCompression = Enum::Compression::ZipPrediction;
		{
			uint16_t compressionNum = 0;
			auto compressionNumSpan = Util::toWritableBytes(compressionNum);
			stream.read(compressionNumSpan, channelOffset);
			compressionNum = endianDecodeBE<uint16_t>(compressionNumSpan.data());
			channelCompression = Enum::compressionMap.at(compressionNum);
		}
		m_ChannelCompression[index] = channelCompression;
		FileSection::size(FileSection::size() + channel.m_Size);

		if (header.m_Depth == Enum::BitDepth::BD_8)
		{
			std::span<uint8_t> bufferSpan(buffer.data(), coordinates.width * coordinates.height);
			DecompressData<uint8_t>(stream, bufferSpan, channelOffset + 2u, channelCompression, header, coordinates.width, coordinates.height, channel.m_Size - 2u);
			auto channelPtr = std::make_unique<ImageChannel>(
				channelCompression,
				bufferSpan,
				channel.m_ChannelID,
				coordinates.width,
				coordinates.height,
				coordinates.centerX,
				coordinates.centerY);
			m_ImageData[index] = std::move(channelPtr);
		}
		else if (header.m_Depth == Enum::BitDepth::BD_16)
		{
			std::span<uint16_t> bufferSpan(reinterpret_cast<uint16_t*>(buffer.data()), coordinates.width * coordinates.height);
			DecompressData<uint16_t>(stream, bufferSpan, channelOffset + 2u, channelCompression, header, coordinates.width, coordinates.height, channel.m_Size - 2u);
			auto channelPtr = std::make_unique<ImageChannel>(
				channelCompression,
				bufferSpan,
				channel.m_ChannelID,
				coordinates.width,
				coordinates.height,
				coordinates.centerX,
				coordinates.centerY);
			m_ImageData[index] = std::move(channelPtr);
		}
		if (header.m_Depth == Enum::BitDepth::BD_32)
		{
			std::span<float32_t> bufferSpan(reinterpret_cast<float32_t*>(buffer.data()), coordinates.width * coordinates.height);
			DecompressData<float32_t>(stream, bufferSpan, channelOffset + 2u, channelCompression, header, coordinates.width, coordinates.height, channel.m_Size - 2u);
			auto channelPtr = std::make_unique<ImageChannel>(
				channelCompression,
				bufferSpan,
				channel.m_ChannelID,
				coordinates.width,
				coordinates.height,
				coordinates.centerX,
				coordinates.centerY);
			m_ImageData[index] = std::move(channelPtr);
		}
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ChannelImageData::write(File& document, std::vector<std::vector<uint8_t>>& compressedChannelData, const std::vector<Enum::Compression>& channelCompression)
{
	m_ChannelOffsetsAndSizes = {};
	for (int i = 0; i < compressedChannelData.size(); ++i)
	{
		m_ChannelCompression.push_back(channelCompression[i]);
		m_ChannelOffsetsAndSizes.push_back(std::tuple<uint64_t, uint64_t>(document.getOffset(), compressedChannelData[i].size() + 2u));

		std::optional<uint16_t> compressionCode = Enum::getCompression<Enum::Compression, uint16_t>(channelCompression[i]);
		if (compressionCode.has_value()) [[likely]]
			WriteBinaryData<uint16_t>(document, compressionCode.value());
		else
			PSAPI_LOG_ERROR("LayerInfo", "Could not find a match for the given compression codec");
		WriteBinaryArray<uint8_t>(document, std::move(compressedChannelData[i]));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerInfo::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	uint64_t size = 0u;

	PSAPI_LOG_WARNING("LayerInfo", "Unable to compute size of LayerInfo due to the size only being known upon compressing of the image channels, please refrain from using this function");

	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerInfo::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const bool isFromAdditionalLayerInfo, std::optional<uint64_t> sectionSize)
{
	PSAPI_PROFILE_FUNCTION();

	FileSection::initialize(offset, 0u);
	document.setOffset(offset);

	if (!isFromAdditionalLayerInfo)
	{
		// Read the layer info length marker which is 4 bytes in psd and 8 bytes in psb mode 
		// (note, this section is padded to 4 bytes which means we might have some padding bytes at the end)
		std::variant<uint32_t, uint64_t> size = ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version);
		// We add the size of the length marker as it isnt included in the size 
		FileSection::size(ExtractWidestValue<uint32_t, uint64_t>(size) + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version));
		if (ExtractWidestValue<uint32_t, uint64_t>(size) == 0u)
		{
			return;
		}
	}
	else if (isFromAdditionalLayerInfo && sectionSize)
	{
		// The reason for this specialization is that in 16 and 32 bit mode photoshop writes the layer info section
		// in a tagged block "Lr16" or "Lr32" which already has a size variable.
		FileSection::size(sectionSize.value());
	}
	else
	{
		PSAPI_LOG_ERROR("LayerInfo", "LayerInfo() expects an explicit section size if the call is from the additional layer information section");
	}

	// If this value is negative the first alpha channel of the layer records would hold the merged image result (Image Data Section) alpha channel
	// which we do not care about
	uint16_t layerCount = static_cast<uint16_t>(std::abs(ReadBinaryData<int16_t>(document)));
	// While it may seem counterintuitive to set the callbacks max here, due to the way the data is either stored
	// on the LayerInfo or in the AdditionalLayerInfo this value will only be set once 
	callback.setMax(layerCount);

	m_LayerRecords.reserve(layerCount);
	m_ChannelImageData.reserve(layerCount);

	// Extract layer records
	for (int i = 0; i < layerCount; i++)
	{
		LayerRecord layerRecord = {};
		layerRecord.read(document, header, callback, document.getOffset());
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

	// Read the Channel Image Instances
	std::vector<ChannelImageData> localResults(m_LayerRecords.size());
	std::for_each(std::execution::par, m_LayerRecords.begin(), m_LayerRecords.end(), [&](const LayerRecord& layerRecord)
	{
		callback.setTask("Reading Layer: " + std::string(layerRecord.m_LayerName.getString()));
		size_t index = &layerRecord - &m_LayerRecords[0];

		uint64_t tmpOffset = channelImageDataOffsets[index];
		uint64_t tmpSize = channelImageDataSizes[index];

		// Read the binary data. Note that this is done in one step to avoid the offset being set differently before 
		// reading the data. We also do this within the loop to avoid allocating all the memory at once
		ByteStream stream(document, tmpOffset, tmpSize);

		// Create the ChannelImageData by parsing the given buffer
		auto result = ChannelImageData();
		result.read(stream, header, tmpOffset, layerRecord);

		// As each index is unique we do not need to worry about locking here
		localResults[index] = std::move(result);
		// Increment the callback
		callback.setTask("Read Layer: " + std::string(layerRecord.m_LayerName.getString()));
		callback.increment();
	});
	// Combine results after the loop
	m_ChannelImageData.insert(m_ChannelImageData.end(), std::make_move_iterator(localResults.begin()), std::make_move_iterator(localResults.end()));

	// Set the offset to where it is supposed to be as we cannot guarantee the location of the marker after jumping back and forth in image sections
	document.setOffset(imageDataOffset);

	const uint64_t expectedOffset = FileSection::offset() + FileSection::size();
	if (document.getOffset() != expectedOffset)
	{
		int64_t toSkip = static_cast<int64_t>(expectedOffset) - static_cast<int64_t>(document.getOffset());
		// Check that the skipped bytes are within the amount needed to pad a LayerInfo section
		if (toSkip < -4 || toSkip > 4)
		{
			PSAPI_LOG_ERROR("LayerInfo", "Tried skipping bytes larger than the padding of the section: %i", toSkip);
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
		if (layer.m_LayerName.getString() == layerName)
		{
			return count;
		}
		++count;
	}
	return -1;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerInfo::write(File& document, const FileHeader& header, ProgressCallback& callback)
{
	PSAPI_PROFILE_FUNCTION();
	// The writing of this section is a bit confusing as we must first compress all of our image data, then write the 
	// section size and Layer Records with the size markers that we found. After this we finally write the compressed data to disk
	// it is imperative that the layer order is consistent between the LayerRecords and the ChannelImageData as that is how photoshop
	// maps these two together

	// If we are in 16- or 32-bit mode we just write an empty section marker and continue. We must additionally check
	// that the layer size is 0 as this function gets called from both the 'Lr16' and 'Lr32'
	// tagged block as well as the layer info section itself
	if (m_LayerRecords.size() == 0u && (header.m_Depth == Enum::BitDepth::BD_16 || header.m_Depth == Enum::BitDepth::BD_32))
	{
		WriteBinaryDataVariadic<uint32_t, uint64_t>(document, 0u, header.m_Version);
		return;
	}
	if (m_LayerRecords.size() == 0u) [[unlikely]]
	{
		PSAPI_LOG_ERROR("LayerInfo", "Invalid Document encountered. Photoshop files must contain at least one layer");
	}
	if (m_LayerRecords.size() != m_ChannelImageData.size()) [[unlikely]]
	{
		PSAPI_LOG_ERROR("LayerInfo", "The number of layer records and channel image data instances mismatch, got %i lrRecords and %i channelImgData", m_LayerRecords.size(), m_ChannelImageData.size());
	}

	// We set the max to be two times the layer size here to indicate one step for compressing the data and another step for
	// writing the data, the final step is added for the ImageData section
	callback.setMax(m_LayerRecords.size() * 2 + 1);
	
	// The nesting here indicates Layers/Channels/ImgData. We reserve the top level as we access these members in parallel
	std::vector<std::vector<std::vector<uint8_t>>> compressedData(m_ChannelImageData.size());
	std::vector<std::vector<LayerRecords::ChannelInformation>> channelInfos(m_ChannelImageData.size());
	std::vector<std::vector<Enum::Compression>> channelCompression(m_ChannelImageData.size());

	// Write an empty section size, we come back later and fill this out once written
	uint64_t sizeMarkerOffset = document.getOffset();
	WriteBinaryDataVariadic<uint32_t, uint64_t>(document, 0u, header.m_Version);
	// The layer count could be written as a negative value to indicate that the first alpha channel in the file is the merged image data alpha
	// but we do not bother with that at this point
	WriteBinaryData(document, static_cast<int16_t>(m_LayerRecords.size()));

	// Loop over the individual layers and compress them while also storing the channel information
	std::for_each(std::execution::par, m_ChannelImageData.begin(), m_ChannelImageData.end(),
		[&](ChannelImageData& channel)
		{
			// Get a unique index for each of the layers to compress them in random order
			const size_t index = &channel - &m_ChannelImageData[0];
			callback.setTask("Compressing Layer: " + std::string(m_LayerRecords[index].m_LayerName.getString()));
			std::vector<LayerRecords::ChannelInformation> lrChannelInfo;
			std::vector<Enum::Compression> lrCompression;

			// If we have some additional threads to spare we pass them into compression as some internal functions can make use of this
			size_t threadCount = std::thread::hardware_concurrency() / m_ChannelImageData.size();
			threadCount = threadCount < 1 ? 1 : threadCount;

			if (header.m_Depth == Enum::BitDepth::BD_8)
			{
				compressedData[index] = channel.compressData<uint8_t>(header, lrChannelInfo, lrCompression, threadCount);
			}
			else if (header.m_Depth == Enum::BitDepth::BD_16)
			{
				compressedData[index] = channel.compressData<uint16_t>(header, lrChannelInfo, lrCompression, threadCount);
			}
			else if (header.m_Depth == Enum::BitDepth::BD_32)
			{
				compressedData[index] = channel.compressData<float32_t>(header, lrChannelInfo, lrCompression, threadCount);
			}
			else
			{
				PSAPI_LOG_ERROR("LayerInfo", "Unsupported BitDepth encountered, currently only 8-, 16- and 32-bit files are supported");
			}
			channelInfos[index] = lrChannelInfo;
			channelCompression[index] = lrCompression;
			callback.setTask("Compressed Layer: " + std::string(m_LayerRecords[index].m_LayerName.getString()));
			callback.increment();
		});

	// Write the layer records
	for (int i = 0; i < m_LayerRecords.size(); ++i)
	{
		m_LayerRecords[i].write(document, header, callback, channelInfos[i]);
	}

	// Write the ChannelImageData to disk, 
	for (int i = 0; i < compressedData.size(); ++i)
	{
		callback.setTask("Writing Layer: " + std::string(m_LayerRecords[i].m_LayerName.getString()));
		m_ChannelImageData[i].write(document, compressedData[i], channelCompression[i]);
		callback.increment();
	}

	// Count how many bytes we already wrote, go back to the size marker and write that information
	uint64_t endOffset = document.getOffset();
	uint64_t sectionSize = endOffset - sizeMarkerOffset;
	document.setOffset(sizeMarkerOffset);
	uint64_t sectionSizeRounded = RoundUpToMultiple<uint64_t>(sectionSize, 4u);
	// Subtract the section size marker from the total length as it isnt counted
	WriteBinaryDataVariadic<uint32_t, uint64_t>(document, sectionSizeRounded - SwapPsdPsb<uint32_t, uint64_t>(header.m_Version), header.m_Version);
	// Set the offset back to the end to leave the document in a valid state
	document.setOffset(endOffset);
	WritePadddingBytes(document, sectionSizeRounded - sectionSize);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void GlobalLayerMaskInfo::read(File& document, const uint64_t offset)
{
	document.setOffset(offset);
	// As this section is undocumented, we currently just skip it.
	FileSection::initialize(offset, static_cast<uint64_t>(ReadBinaryData<uint32_t>(document)) + 4u);
	document.skip(FileSection::size() - 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void GlobalLayerMaskInfo::write(File& document)
{
	// Write an empty section
	WriteBinaryData<uint32_t>(document, 0u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LayerAndMaskInformation::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	uint64_t size = 0u;

	PSAPI_LOG_WARNING("LayerAndMaskInformation", "Unable to compute size of LayerAndMaskInformation due to the size only being known upon compressing of the image channels, please refrain from using this function");

	return size;
}


// Extract the layer and mask information section
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerAndMaskInformation::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset)
{
	PSAPI_PROFILE_FUNCTION();

	FileSection::initialize(offset, 0u);
	document.setOffset(offset);

	// Read the layer mask info length marker which is 4 bytes in psd and 8 bytes in psb mode
	std::variant<uint32_t, uint64_t> size = ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version);
	FileSection::size(ExtractWidestValue<uint32_t, uint64_t>(size));

	// Parse Layer Info Section
	{
		m_LayerInfo.read(document, header, callback, document.getOffset());
		// Check the theoretical document offset against what was read by the layer info section. These should be identical
		if (document.getOffset() != (FileSection::offset() + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version)) + m_LayerInfo.size())
		{
			PSAPI_LOG_ERROR("LayerAndMaskInformation", "Layer Info read an incorrect amount of bytes from the document, expected an offset of %" PRIu64 ", but got %" PRIu64 " instead.",
				FileSection::offset() + m_LayerInfo.size() + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version),
				document.getOffset());
		}
	}
	// Parse Global Layer Mask Info
	{
		m_GlobalLayerMaskInfo.read(document, document.getOffset());
	}

	int64_t toRead = FileSection::size() - m_LayerInfo.size() - m_GlobalLayerMaskInfo.size();
	// If there is still data left to read, this is the additional layer information which is also present at the end of each layer record
	if (toRead >= 12u)
	{
		// Tagged blocks at the end of the layer and mask information seem to be padded to 4-bytes
		AdditionalLayerInfo layerInfo = {};
		layerInfo.read(document, header, callback, document.getOffset(), toRead, 4u);
		m_AdditionalLayerInfo.emplace((std::move(layerInfo)));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LayerAndMaskInformation::write(File& document, const FileHeader& header, ProgressCallback& callback)
{
	PSAPI_PROFILE_FUNCTION();
	// For the layer and mask information section, getting the size is a little bit awkward as we only know the size upon 
	// writing the layer info and additional layer information sections. Therefore we will write an empty size marker,
	// then write the contents after which we manually calculate the section size and replace the value
	uint64_t sizeMarkerOffset = document.getOffset();
	WriteBinaryDataVariadic<uint32_t, uint64_t>(document, 0u, header.m_Version);

	m_LayerInfo.write(document, header, callback);
	m_GlobalLayerMaskInfo.write(document);
	if (m_AdditionalLayerInfo.has_value())
		m_AdditionalLayerInfo.value().write(document, header, callback, 4u);

	
	// The section size does not include the size marker so we must subtract that
	uint64_t endOffset = document.getOffset();
	uint64_t sectionSize = endOffset - sizeMarkerOffset - SwapPsdPsb<uint32_t, uint64_t>(header.m_Version);
	document.setOffset(sizeMarkerOffset);
	uint64_t sectionSizeRounded = RoundUpToMultiple<uint64_t>(sectionSize, 4u);
	WriteBinaryDataVariadic<uint32_t, uint64_t>(document, sectionSizeRounded, header.m_Version);
	// Set the offset back to the end to leave the document in a valid state
	document.setOffset(endOffset);
	WritePadddingBytes(document, sectionSizeRounded - sectionSize);
}


PSAPI_NAMESPACE_END