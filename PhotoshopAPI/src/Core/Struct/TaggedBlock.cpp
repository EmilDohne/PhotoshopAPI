#include "TaggedBlock.h"

#include "Macros.h"
#include "Enum.h"
#include "Logger.h"
#include "StringUtil.h"
#include "Core/Struct/Signature.h"
#include "Core/Struct/File.h"
#include "PhotoshopFile/FileHeader.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"

#include <cassert>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void TaggedBlock::read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding /* = 1u */)
{
	m_Offset = offset;
	m_Signature = signature;
	m_Key = key;
	if (Enum::isTaggedBlockSizeUint64(m_Key) && header.m_Version == Enum::Version::Psb)
	{
		uint64_t length = ReadBinaryData<uint64_t>(document);
		length = RoundUpToMultiple<uint64_t>(length, padding);
		m_Length = length;
		document.skip(length);

		m_TotalLength = length + 4u + 4u + 8u;
	}
	else
	{
		uint32_t length = ReadBinaryData<uint32_t>(document);
		length = RoundUpToMultiple<uint32_t>(length, padding);
		m_Length = length;
		document.skip(length);


		m_TotalLength = static_cast<uint64_t>(length) + 4u + 4u + 4u;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void TaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /* = 1u */)
{

	// Signatures are specified as being either '8BIM' or '8B64'. However, it isnt specified when we use which one.
	// For simplicity we will just write '8BIM' all the time and only write other signatures if we encounter them.
	// The 'FMsk' and 'cinf' tagged blocks for example have '8B64' in PSB mode
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	std::optional<std::vector<std::string>> keyStr = Enum::getTaggedBlockKey<Enum::TaggedBlockKey, std::vector<std::string>>(m_Key);
	if (!keyStr.has_value())
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Was unable to extract a string from the tagged block key");
	}
	else
	{
		// We use the first found value from the key matches
		WriteBinaryData<uint32_t>(document, Signature(keyStr.value()[0]).m_Value);
	}

	if (isTaggedBlockSizeUint64(m_Key) && header.m_Version == Enum::Version::Psb)
	{
		WriteBinaryData<uint64_t>(document, 0u);
	}
	else
	{
		WriteBinaryData<uint32_t>(document, 0u);
	}

	// No need to write any padding bytes here as the section will already be aligned to all the possible padding sizes (1u for LayerRecord TaggedBlocks and 4u
	// for "Global" Tagged Blocks (found at the end of the LayerAndMaskInformation section))
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LrSectionTaggedBlock::read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding)
{
	m_Key = Enum::TaggedBlockKey::lrSectionDivider;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	length = RoundUpToMultiple<uint32_t>(length, padding);
	m_Length = length;

	uint32_t type = ReadBinaryData<uint32_t>(document);
	if (type < 0 || type > 3)
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Layer Section Divider type has to be between 0 and 3, got %u instead", type);
	};
	auto sectionDividerType = Enum::getSectionDivider<uint32_t, Enum::SectionDivider>(type);
	if (sectionDividerType)
	{
		m_Type = sectionDividerType.value();
	}
	else
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Could not find Layer Section Divider type by value");
	}


	// This overrides the layer blend mode if it is present.
	if (length >= 12u)
	{
		Signature sig = Signature(ReadBinaryData<uint32_t>(document));
		if (sig != Signature("8BIM"))
		{
			PSAPI_LOG_ERROR("TaggedBlock", "Signature does not match '8BIM', got '%s' instead",
				uint32ToString(sig.m_Value).c_str());
		}

		std::string blendModeStr = uint32ToString(ReadBinaryData<uint32_t>(document));
		m_BlendMode = Enum::getBlendMode<std::string, Enum::BlendMode>(blendModeStr);
	}

	if (length >= 16u)
	{
		// This is the sub-type information, probably for animated photoshop files
		// we do not care about this currently
		document.skip(4u);
	}

	TaggedBlock::totalSize(static_cast<size_t>(length) + 4u + 4u + 4u);
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LrSectionTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("lsct").m_Value);
	WriteBinaryData<uint32_t>(document, TaggedBlock::totalSize<uint32_t>() - 12u);

	auto sectionDividerType = Enum::getSectionDivider<Enum::SectionDivider, uint32_t>(m_Type);
	if (sectionDividerType)
	{
		WriteBinaryData<uint32_t>(document, sectionDividerType.value());
	}
	else
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Could not find Layer Section Divider type by value");
	}
	
	// For some reason the blend mode has another 4 bytes for a 8BIM key
	if (m_BlendMode.has_value())
	{
		WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
		std::optional<std::string> blendModeStr = Enum::getBlendMode<Enum::BlendMode, std::string>(m_BlendMode.value());
		if (!blendModeStr.has_value())
			PSAPI_LOG_ERROR("LayerRecord", "Could not identify a blend mode string from the given key");
		else 
			WriteBinaryData<uint32_t>(document, Signature(blendModeStr.value()).m_Value);
	}

	// There is an additional variable here for storing information related to timelines, but seeing as we do not care 
	// about animated PhotoshopFiles at this moment we dont write anything here
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Lr16TaggedBlock::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const Signature signature, const uint16_t padding)
{
	m_Key = Enum::TaggedBlockKey::Lr16;
	m_Offset = offset;
	m_Signature = signature;
	uint64_t length = ExtractWidestValue<uint32_t, uint64_t>(ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version));
	length = RoundUpToMultiple<uint64_t>(length, padding);
	m_Length = length;
	m_Data.read(document, header, callback, document.getOffset(), true, std::get<uint64_t>(m_Length));

	TaggedBlock::totalSize(length + 4u + 4u + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version));
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Lr16TaggedBlock::write(File& document, const FileHeader& header, ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("Lr16").m_Value);

	// We dont need to write a size marker for this data as the size marker of the LayerInfo takes
	// care of that
	m_Data.write(document, header, callback);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Lr32TaggedBlock::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const Signature signature, const uint16_t padding)
{
	m_Key = Enum::TaggedBlockKey::Lr32;
	m_Offset = offset;
	m_Signature = signature;
	uint64_t length = ExtractWidestValue<uint32_t, uint64_t>(ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version));
	length = RoundUpToMultiple<uint64_t>(length, padding);
	m_Length = length;
	m_Data.read(document, header, callback, document.getOffset(), true, std::get<uint64_t>(m_Length));

	TaggedBlock::totalSize(length + 4u + 4u + SwapPsdPsb<uint32_t, uint64_t>(header.m_Version));
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Lr32TaggedBlock::write(File& document, const FileHeader& header, ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("Lr32").m_Value);

	// We dont need to write a size marker for this data as the size marker of the LayerInfo takes
	// care of that
	m_Data.write(document, header, callback);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ReferencePointTaggedBlock::read(File& document,const uint64_t offset, const Signature signature)
{
	m_Key = Enum::TaggedBlockKey::lrReferencePoint;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	// The data is always two doubles 
	if (length != 16u)
	{
		PSAPI_LOG_ERROR("ReferencePointTaggedBlock", "Invalid size for Reference Point found, expected 16 but got %u", length);
	}
	m_Length = length;
	m_ReferenceX = ReadBinaryData<float64_t>(document);
	m_ReferenceY = ReadBinaryData<float64_t>(document);
	TaggedBlock::totalSize(static_cast<size_t>(length) + 4u + 4u + 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ReferencePointTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /* = 1u */)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("fxrp").m_Value);
	WriteBinaryData<uint32_t>(document, TaggedBlock::totalSize<uint32_t>() - 12u);

	WriteBinaryData<float64_t>(document, m_ReferenceX);
	WriteBinaryData<float64_t>(document, m_ReferenceY);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void UnicodeLayerNameTaggedBlock::read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding /*= 1u*/)
{
	m_Key = Enum::TaggedBlockKey::lrUnicodeName;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	length = RoundUpToMultiple<uint32_t>(length, padding);
	m_Length = length;
	// Internally it appears that UnicodeStrings are always padded to a 4-byte boundary which decides whether there is a two bye
	// null character appended
	m_Name.read(document, 4u);

	TaggedBlock::totalSize(static_cast<size_t>(length) + 4u + 4u + 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void UnicodeLayerNameTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("luni").m_Value);
	WriteBinaryData<uint32_t>(document, TaggedBlock::totalSize<uint32_t>() - 12u);
	// Internally it appears that UnicodeStrings are always padded to a 4-byte boundary which decides whether there is a two bye
	// null character appended
	m_Name.write(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ProtectedSettingTaggedBlock::read(File& document, const uint64_t offset, const Signature signature)
{
	m_Key = Enum::TaggedBlockKey::lrProtectedSetting;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	m_Length = length;
	TaggedBlock::totalSize(static_cast<size_t>(length) + 4u + 4u + 4u);

	if (length != 4u)
	{
		PSAPI_LOG_WARNING("ProtectedSettingTaggedBlock", "Block size did not match expected size of 4, instead got %d, skipping reading this block", length);
		document.setOffset(offset + 4u + length);
		return;
	}

	auto flags = ReadBinaryData<uint8_t>(document);
	m_IsLocked = flags & 128u;	// Check if the flags at bit 7 are true
	// Skip the last 3 bytes
	document.skip(3u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ProtectedSettingTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("lspf").m_Value);
	WriteBinaryData<uint32_t>(document, TaggedBlock::totalSize<uint32_t>() - 12u);
	
	if (m_IsLocked)
	{
		WriteBinaryData<uint8_t>(document, 128u);
		WritePadddingBytes(document, 3u);
	}
	else
	{
		WritePadddingBytes(document, 4u);
	}
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LinkedLayerDate::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	return sizeof(uint32_t) + 4 * sizeof(uint8_t) + sizeof(float64_t);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerDate::read(File& document)
{
	this->year		= ReadBinaryData<uint32_t>(document);
	this->month		= ReadBinaryData<uint8_t>(document);
	this->day		= ReadBinaryData<uint8_t>(document);
	this->hour		= ReadBinaryData<uint8_t>(document);
	this->minute	= ReadBinaryData<uint8_t>(document);
	this->seconds	= ReadBinaryData<double>(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerDate::write(File& document) const
{
	WriteBinaryData<uint32_t>(document, this->year);
	WriteBinaryData<uint8_t>(document, this->month);
	WriteBinaryData<uint8_t>(document, this->day);
	WriteBinaryData<uint8_t>(document, this->hour);
	WriteBinaryData<uint8_t>(document, this->minute);
	WriteBinaryData<double>(document, this->seconds);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LinkedLayerDate::LinkedLayerDate()
{
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm* localTime = std::localtime(&t);

	// Set the struct fields
	year = 1900 + localTime->tm_year;
	month = 1 + localTime->tm_mon;
	day = localTime->tm_mday;
	hour = localTime->tm_hour;
	minute = localTime->tm_min;
	seconds = static_cast<double>(localTime->tm_sec);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerTaggedBlock::read(File& document, const FileHeader& header, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature, const uint16_t padding /*= 1u*/)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	uint64_t toRead = 0;
	if (m_Key == Enum::TaggedBlockKey::lrLinked)
	{
		uint32_t length = ReadBinaryData<uint32_t>(document);
		length = RoundUpToMultiple<uint32_t>(length, padding);
		toRead = length;
		m_Length = length;
	}
	else if (m_Key == Enum::TaggedBlockKey::lrLinked_8Byte)
	{
		uint64_t length = ReadBinaryData<uint64_t>(document);
		length = RoundUpToMultiple<uint64_t>(length, padding);
		toRead = length;
		m_Length = length;
	}
	else
	{
		PSAPI_LOG_ERROR("LinkedLayer", "Unknonw tagged block key, aborting parsing");
	}

	// A linked Layer tagged block may contain any number of LinkedLayers, and there is no explicit
	// number of layers we must keep reading LinkedLayers until we've reached the end of the taggedblock
	uint64_t startOffset = document.getOffset();
	uint64_t endOffset = document.getOffset() + toRead;
	while (document.getOffset() < endOffset)
	{
		LinkedLayerData data;

		auto size = ReadBinaryData<uint64_t>(document);

		data.m_Type = readType(document);
		data.m_Version = ReadBinaryData<uint32_t>(document);
		if (data.m_Version < 1 || data.m_Version > 7)
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unknown Linked Layer version %d encountered, aborting parsing", data.m_Version);
		}

		PascalString uniqueID;
		uniqueID.read(document, 1u);
		data.m_UniqueID = uniqueID.getString();

		auto fileTypeArr = ReadBinaryArray<char>(document, 4u);
		data.m_FileType = std::string(fileTypeArr.begin(), fileTypeArr.end());

		data.m_FileCreator = ReadBinaryData<uint32_t>(document);

		auto sizeRest = ReadBinaryData<uint64_t>(document);
		bool hasFileOpenDescriptor = ReadBinaryData<bool>(document);
		if (hasFileOpenDescriptor)
		{
			Descriptors::Descriptor fileOpenDescriptor;
			fileOpenDescriptor.read(document);
			data.m_FileOpenDescriptor = fileOpenDescriptor;
		}
		if (data.m_Type == LinkedLayerData::Type::External)
		{
			Descriptors::Descriptor linkedFileDescriptor;
			linkedFileDescriptor.read(document);
			data.m_LinkedFileDescriptor = linkedFileDescriptor;
		}

		if (data.m_Type == LinkedLayerData::Type::External && data.m_Version > 3)
		{
			
		}

		std::optional<uint64_t> fileSize;
		if (data.m_Type == LinkedLayerData::Type::External)
		{
			fileSize = ReadBinaryData<uint64_t>(document);
		}

		// Filler/padding bytes
		if (data.m_Type == LinkedLayerData::Type::Alias)
		{
			document.skip(4u);
		}
		document.skip(8u);

		// Raw file bytes
		std::vector<uint8_t> rawFileBytes;
		if (data.m_Type == LinkedLayerData::Type::External && fileSize)
		{
			rawFileBytes = ReadBinaryArray<uint8_t>(document, fileSize.value());
		}

		std::optional<UnicodeString> childDocumentID;
		if (data.m_Version > 5)
		{
			UnicodeString id;
			id.read(document, 2u);
			childDocumentID = id;
		}

		std::optional<double> assetModTime;
		if (data.m_Version > 6)
		{
			assetModTime = ReadBinaryData<double>(document);
		}

		std::optional<bool> assetLockedState;
		if (data.m_Version > 7)
		{
			assetModTime = ReadBinaryData<bool>(document);
		}

		// The file format ref mentions that if the type is External and the version is 2 we read the raw file bytes here but unless the 
		// Version 2 stored the raw file bytes twice this would seem weird as the call above for the raw file bytes would also still be there
		// We skip any loading now (also because version 2 is likely approaching 20 years in age now so finding files that old will be a challenge).
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LinkedLayerData::Type LinkedLayerTaggedBlock::readType(File& document)
{
	auto keyArr = ReadBinaryArray<char>(document, 4u);
	std::string key(keyArr.begin(), keyArr.end());

	if (key == "liFD")
	{
		return LinkedLayerData::Type::Data;
	}
	if (key == "liFE")
	{
		return LinkedLayerData::Type::External;
	}
	if (key == "liFA")
	{
		return LinkedLayerData::Type::Alias;
	}
	PSAPI_LOG_ERROR("LinkedLayer", "Unable to decode Linked Layer type '%s', aborting parsing", key.c_str());
}


PSAPI_NAMESPACE_END