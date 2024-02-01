#include "ResourceBlock.h"

#include "FileIO/Read.h"
#include "FileIO/Write.h"
#include "FileIO/Util.h"
#include "Enum.h"
#include "Logger.h"
#include "Profiling/Perf/Instrumentor.h"

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t ResourceBlock::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	uint64_t size = 0u;
	size += 4u;	// Signature
	size += 2u; // ID of the resource
	size += m_Name.calculateSize();
	size += 4u;	// Size marker of data to follow
	size += m_DataSize;	// Data size, already padded to 2u
	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ResolutionInfoBlock::ResolutionInfoBlock()
{
	m_UniqueId = Enum::ImageResource::ResolutionInfo;
	m_Name = {};
	m_DataSize = 16u;	// 8-bytes for each horizontal and vertical
	m_Size = ResourceBlock::calculateSize();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ResolutionInfoBlock::ResolutionInfoBlock(float resolution, Enum::ResolutionUnit resolutionUnit /*= Enum::ResolutionUnit::PixelsPerInch*/, Enum::DisplayUnit displayUnit /*= Enum::DisplayUnit::Cm*/)
{
	m_UniqueId = Enum::ImageResource::ResolutionInfo;
	m_Name = {};
	m_DataSize = 16u;	// 8-bytes for each horizontal and vertical
	m_Size = ResourceBlock::calculateSize();
	
	m_HorizontalRes = { resolution };
	m_HorizontalResUnit = resolutionUnit;
	m_WidthUnit = displayUnit;

	m_VerticalRes = { resolution };
	m_VerticalResUnit = resolutionUnit;
	m_HeightUnit = displayUnit;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ResolutionInfoBlock::read(File& document, const uint64_t offset)
{
	PROFILE_FUNCTION();
	m_Offset = offset;
	m_UniqueId = Enum::ImageResource::ResolutionInfo;
	m_Name.read(document, 2u);
	m_DataSize = RoundUpToMultiple(ReadBinaryData<uint32_t>(document), 2u);
	m_Size = static_cast<uint64_t>(4u) + 2u + m_Name.m_Size + 4u + m_DataSize;

	if (m_DataSize != 16u) [[unlikely]]
	{
		PSAPI_LOG_ERROR("ResolutionInfoBlock", "Data size must be 16, not %u", m_DataSize);
	}
	
	// Read the ResolutionInfo struct
	m_HorizontalRes = { ReadBinaryData<uint16_t>(document), ReadBinaryData<uint16_t>(document) };
	m_HorizontalResUnit = Enum::resolutionUnitMap.at(ReadBinaryData<uint16_t>(document));
	m_WidthUnit = Enum::displayUnitMap.at(ReadBinaryData<uint16_t>(document));

	m_VerticalRes = { ReadBinaryData<uint16_t>(document), ReadBinaryData<uint16_t>(document) };
	m_VerticalResUnit = Enum::resolutionUnitMap.at(ReadBinaryData<uint16_t>(document));
	m_HeightUnit = Enum::displayUnitMap.at(ReadBinaryData<uint16_t>(document));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ResolutionInfoBlock::write(File& document)
{
	PROFILE_FUNCTION();

	Signature sig = Signature("8BPS");
	WriteBinaryData<uint32_t>(document, sig.m_Value);

	WriteBinaryData<uint16_t>(document, Enum::imageResourceToInt(m_UniqueId));
	m_Name.write(document, 2u);
	WriteBinaryData<uint32_t>(document, m_DataSize);

	// Write the ResolutionInfo struct
	auto horizontalResTuple = m_HorizontalRes.getNumbers();
	WriteBinaryData<uint16_t>(document, std::get<0>(horizontalResTuple));
	WriteBinaryData<uint16_t>(document, std::get<1>(horizontalResTuple));
	WriteBinaryData<uint16_t>(document, Enum::resolutionUnitMapRev.at(m_HorizontalResUnit));
	WriteBinaryData<uint16_t>(document, Enum::displayUnitMapRev.at(m_WidthUnit));

	auto verticalResTuple = m_VerticalRes.getNumbers();
	WriteBinaryData<uint16_t>(document, std::get<0>(verticalResTuple));
	WriteBinaryData<uint16_t>(document, std::get<1>(verticalResTuple));
	WriteBinaryData<uint16_t>(document, Enum::resolutionUnitMapRev.at(m_VerticalResUnit));
	WriteBinaryData<uint16_t>(document, Enum::displayUnitMapRev.at(m_HeightUnit));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ICCProfileBlock::ICCProfileBlock(std::vector<uint8_t>&& iccProfile)
{
	m_UniqueId = Enum::ImageResource::ICCProfile;
	m_Name = {};
	m_DataSize = RoundUpToMultiple<uint32_t>(iccProfile.size(), 2u);
	m_Size = ResourceBlock::calculateSize();

	m_RawICCProfile = std::move(iccProfile);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ICCProfileBlock::read(File& document, const uint64_t offset)
{
	PROFILE_FUNCTION();
	m_Offset = offset;
	m_UniqueId = Enum::ImageResource::ICCProfile;
	m_Name.read(document, 2u);
	m_DataSize = RoundUpToMultiple(ReadBinaryData<uint32_t>(document), 2u);
	m_Size = static_cast<uint64_t>(4u) + 2u + m_Name.m_Size + 4u + m_DataSize;

	m_RawICCProfile = ReadBinaryArray<uint8_t>(document, m_DataSize);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ICCProfileBlock::write(File& document)
{
	PROFILE_FUNCTION();

	Signature sig = Signature("8BPS");
	WriteBinaryData<uint32_t>(document, sig.m_Value);

	WriteBinaryData<uint16_t>(document, Enum::imageResourceToInt(m_UniqueId));
	m_Name.write(document, 2u);
	WriteBinaryData<uint32_t>(document, m_DataSize);	// This value is already padded

	WriteBinaryArray<uint8_t>(document, m_RawICCProfile);

	// Check that we didnt initialize m_DataSize incorrectly
	if (static_cast<int>(m_DataSize) - m_RawICCProfile.size() < 0) [[unlikely]]
	{
		PSAPI_LOG_ERROR("ICCProfileBlock", "Block would require writing %i padding bytes which is not possible, is m_DataSize initialized correctly?", 
			static_cast<int>(m_DataSize) - m_RawICCProfile.size());
	}
	// This will handle the 0 byte case
	WritePadddingBytes(document, m_DataSize - m_RawICCProfile.size());
}

PSAPI_NAMESPACE_END