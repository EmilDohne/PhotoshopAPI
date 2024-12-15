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
#include <ctime>

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
void PlacedLayer::Point::read(File& document)
{
	this->x = ReadBinaryData<double>(document);
	this->y = ReadBinaryData<double>(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayer::Point::write(File& document)
{
	WriteBinaryData<double>(document, this->x);
	WriteBinaryData<double>(document, this->y);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayer::Transform::read(File& document)
{
	this->topleft.read(document);
	this->topright.read(document);
	this->bottomright.read(document);
	this->bottomleft.read(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayer::Transform::write(File& document)
{
	this->topleft.write(document);
	this->topright.write(document);
	this->bottomright.write(document);
	this->bottomleft.write(document);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerTaggedBlock::read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	m_Length = ReadBinaryData<uint32_t>(document);
	TaggedBlock::totalSize(static_cast<size_t>(std::get<uint32_t>(m_Length)) + 4u + 4u + 4u);

	// The type is always going to be 'plcL' according to the docs
	auto type = Signature::read(document);
	if (type != "plcL")
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown placed layer type '%s' encountered", type.string().c_str());
	}

	m_Version = ReadBinaryData<uint32_t>(document);
	if (m_Version != 3)
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown placed layer version %d encountered", m_Version);
	}

	m_UniqueID.read(document, 1u);

	m_PageNumber = ReadBinaryData<uint32_t>(document);
	m_TotalPages = ReadBinaryData<uint32_t>(document);
	m_AnitAliasPolicy = ReadBinaryData<uint32_t>(document);

	auto layerType = ReadBinaryData<uint32_t>(document);
	if (layerType > 3)
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown placed layer LayerType %d encountered", layerType);
	}
	m_Type = PlacedLayer::s_TypeMap.at(layerType);
	if (m_Type != PlacedLayer::Type::Raster)
	{
		PSAPI_LOG_WARNING("PlacedLayer", "Currently unimplemented LayerType '%s' encountered", PlacedLayer::s_TypeStrMap.at(layerType).c_str());
	}

	m_Transform.read(document);

	auto warpVersion = ReadBinaryData<uint32_t>(document);
	auto descriptorVersion = ReadBinaryData<uint32_t>(document);
	if (warpVersion != 0 || descriptorVersion != 16)
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown warp or descriptor version encountered. Warp version: %d. Descriptor Version: %d. Expected 0 and 16 for these respectively", warpVersion, descriptorVersion);
	}
	m_WarpInformation.read(document);

	// This section is padded so we simply skip to the end
	document.setOffset(offset + TaggedBlock::totalSize<uint64_t>());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerTaggedBlock::write(File& document, const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("PlLd").m_Value);
	
	auto lenOffset = document.getOffset();
	if (header.m_Version == Enum::Version::Psd)
	{
		WriteBinaryData<uint32_t>(document, TaggedBlock::totalSize<uint32_t>() - 12u);
	}
	else
	{
		WriteBinaryData<uint64_t>(document, TaggedBlock::totalSize<uint64_t>() - 12u);
	}

	WriteBinaryData<uint32_t>(document, Signature("plcL").m_Value);
	WriteBinaryData<uint32_t>(document, m_Version);
	m_UniqueID.write(document);

	WriteBinaryData<uint32_t>(document, m_PageNumber);
	WriteBinaryData<uint32_t>(document, m_TotalPages);
	WriteBinaryData<uint32_t>(document, m_AnitAliasPolicy);

	WriteBinaryData<uint32_t>(document, PlacedLayer::s_TypeMap.at(m_Type));
	
	m_Transform.write(document);

	WriteBinaryData<uint32_t>(document, 0u);
	WriteBinaryData<uint32_t>(document, 16u);
	m_WarpInformation.write(document);

	// Write out the length block as well as any padding. This essentially skips back to the point where we wrote the 
	// zero-sized length block and writes it back out but now with the actual section length
	if (header.m_Version == Enum::Version::Psd)
	{
		Impl::writeLengthBlock<uint32_t>(document, lenOffset, document.getOffset(), 4u);
	}
	else
	{
		Impl::writeLengthBlock<uint64_t>(document, lenOffset, document.getOffset(), 4u);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerDataTaggedBlock::read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	m_Length = ReadBinaryData<uint32_t>(document);
	TaggedBlock::totalSize(static_cast<size_t>(std::get<uint32_t>(m_Length)) + 4u + 4u + 4u);

	// The identifier is always going to be 'soLD' according to the docs
	auto identifier = Signature::read(document);
	if (identifier != "soLD")
	{
		PSAPI_LOG_ERROR("PlacedLayerData", "Unknown placed layer identifier '%s' encountered", identifier.string().c_str());
	}

	m_Version = ReadBinaryData<uint32_t>(document);
	auto descriptorVersion = ReadBinaryData<uint32_t>(document);
	if (m_Version != 4 || descriptorVersion != 16)
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown version or descriptor version encountered. Version: %d. Descriptor Version: %d. Expected 4 and 16 for these respectively", m_Version, descriptorVersion);
	}

	auto tmp_offset = document.getOffset();
	std::vector<uint8_t> data = ReadBinaryArray<uint8_t>(document, std::get<uint32_t>(m_Length) - 12);
	std::ofstream fstream(fmt::format("C:/Users/emild/Desktop/linkedlayers/TaggedBlock_{}.bin", tmp_offset), std::ofstream::binary);
	fstream.write(reinterpret_cast<const char*>(data.data()), data.size());
	document.setOffset(tmp_offset);
	


	m_Descriptor.read(document);
	// Manually skip to the end as this section may be padded
	document.setOffset(offset + TaggedBlock::totalSize<uint64_t>());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerDataTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("SoLd").m_Value);

	auto lenOffset = document.getOffset();
	WriteBinaryData<uint32_t>(document, 0u);

	// Write key, version and descriptor version
	Signature("soLD").write(document);
	WriteBinaryData<uint32_t>(document, m_Version);
	WriteBinaryData<uint32_t>(document, 16u);

	m_Descriptor.write(document);

	// Write out the length block as well as any padding. This essentially skips back to the point where we wrote the 
	// zero-sized length block and writes it back out but now with the actual section length
	Impl::writeLengthBlock<uint32_t>(document, lenOffset, document.getOffset(), padding);
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t LinkedLayer::Date::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	return sizeof(uint32_t) + 4 * sizeof(uint8_t) + sizeof(float64_t);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayer::Date::read(File& document)
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
void LinkedLayer::Date::write(File& document) const
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
LinkedLayer::Date::Date()
{
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	auto localTime = std::make_unique<std::tm>();
	localtime_s(localTime.get(), &t);

	// Set the struct fields
	year = 1900 + localTime->tm_year;
	month = static_cast<uint8_t>(1 + localTime->tm_mon);
	day = static_cast<uint8_t>(localTime->tm_mday);
	hour = static_cast<uint8_t>(localTime->tm_hour);
	minute = static_cast<uint8_t>(localTime->tm_min);
	seconds = static_cast<double>(localTime->tm_sec);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayer::Data::read(File& document)
{
	m_Size = ReadBinaryData<uint64_t>(document);

	m_Type = readType(document);
	m_Version = ReadBinaryData<uint32_t>(document);
	if (m_Version < 1 || m_Version > 7)
	{
		PSAPI_LOG_ERROR("LinkedLayer", "Unknown Linked Layer version %d encountered, aborting parsing", m_Version);
	}

	// Read the UniqueID identifying which layer this belongs to
	m_UniqueID = PascalString::readString(document, 1u);
	m_FileName.read(document, 2u);

	// Read the file type such as " png", " jpg" etc.
	// This may be empty in some cases such as exr, likely when photoshop itself doesnt have a parser for the file
	m_FileType = Signature::read(document).string();

	// Unkown what exactly this is
	m_FileCreator = ReadBinaryData<uint32_t>(document);

	// Read the size of the rest of the data as well as the Descriptors
	auto dataSize = ReadBinaryData<uint64_t>(document);
	bool hasFileOpenDescriptor = ReadBinaryData<bool>(document);
	if (hasFileOpenDescriptor)
	{
		auto descriptorVersion = ReadBinaryData<uint32_t>(document);
		if (descriptorVersion != 16u)
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unknown descriptor version passed. Expected 16 but got %d instead", descriptorVersion);
		}
		Descriptors::Descriptor fileOpenDescriptor;
		fileOpenDescriptor.read(document);
		m_FileOpenDescriptor = fileOpenDescriptor;
	}

	// Decode the actual "data" section of the LinkedLayer
	if (m_Type == Type::External)
	{
		auto descriptorVersion = ReadBinaryData<uint32_t>(document);
		if (descriptorVersion != 16u)
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unknown descriptor version passed. Expected 16 but got %d instead", descriptorVersion);
		}
		Descriptors::Descriptor linkedFileDescriptor;
		linkedFileDescriptor.read(document);
		m_LinkedFileDescriptor = linkedFileDescriptor;

		if (m_Version > 3)
		{
			Date date;
			date.read(document);
			m_Date = date;
		}
		uint64_t externalDataFileSize = ReadBinaryData<uint64_t>(document);
		m_RawFileBytes = ReadBinaryArray<uint8_t>(document, externalDataFileSize);
	}
	else if (m_Type == Type::Alias)
	{
		document.skip(8u);
	}
	else if (m_Type == Type::Data)
	{
		m_RawFileBytes = ReadBinaryArray<uint8_t>(document, dataSize);
	}
	
	// Read data likely pertaining to assets linked in from the asset library.
	if (m_Version >= 5)
	{
		UnicodeString id;
		id.read(document, 2u);
		m_ChildDocumentID = id;
	}
	if (m_Version >= 6)
	{
		m_AssetModTime = ReadBinaryData<double>(document);
	}
	if (m_Version >= 7)
	{
		m_AssetIsLocked = ReadBinaryData<bool>(document);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayer::Data::write(File& document)
{
	auto lenOffset = document.getOffset();
	WriteBinaryData<uint64_t>(document, 0);

	writeType(document, m_Type);
	WriteBinaryData<uint32_t>(document, m_Version);

	PascalString(m_UniqueID, 1u).write(document);
	m_FileName.write(document);

	Signature(m_FileType).write(document);
	WriteBinaryData<uint32_t>(document, m_FileCreator);
	WriteBinaryData(document, m_RawFileBytes.size());	// This may be 0
	WriteBinaryData<bool>(document, m_FileOpenDescriptor.has_value());

	if (m_FileOpenDescriptor)
	{
		m_FileOpenDescriptor.value().write(document);
	}

	// Write out the data related to the different types of linked data
	if (m_Type == Type::External)
	{
		if (m_LinkedFileDescriptor)
		{
			m_LinkedFileDescriptor.value().write(document);
		}
		else
		{
			PSAPI_LOG_ERROR("LinkedLayer", "External file link set as m_Type but m_LinkedFileDescriptor is not populated");
		}
		// If we didnt populate a specific date we write the default initialized date which is just the current timestamp
		if (m_Version > 3)
		{
			m_Date.value_or(Date{}).write(document);
		}
		// The documentation mentions that if version is equals to 2 the file data would instead be at the end of the section.
		// Because we however wouldn't write any more data after this point if it was version 2 this point is irrelevant
		WriteBinaryData<uint64_t>(document, m_RawFileBytes.size());
		WriteBinaryArray<uint8_t>(document, m_RawFileBytes);
	}
	else if (m_Type == Type::Alias)
	{
		WritePadddingBytes(document, 8u);
	}
	else if (m_Type == Type::Data)
	{
		WriteBinaryArray<uint8_t>(document, m_RawFileBytes);
	}


	if (m_Version >= 5)
	{
		m_ChildDocumentID.value_or(UnicodeString("", 2u)).write(document);
	}
	if (m_Version >= 6)
	{
		WriteBinaryData<float64_t>(document, m_AssetModTime.value_or(static_cast<float64_t>(20240923.1f)));
	}
	if (m_Version >= 7)
	{
		WriteBinaryData<bool>(document, m_AssetIsLocked.value_or(false));
	}

	// Write out the length block as well as any padding. This essentially skips back to the point where we wrote the 
	// zero-sized length block and writes it back out but now with the actual section length
	Impl::writeLengthBlock<uint64_t>(document, lenOffset, document.getOffset(), 1u);
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LinkedLayer::Data::Type LinkedLayer::Data::readType(File& document) const
{
	auto key = Signature::read(document);
	if (key == "liFD")
	{
		return LinkedLayer::Data::Type::Data;
	}
	if (key == "liFE")
	{
		return LinkedLayer::Data::Type::External;
	}
	if (key == "liFA")
	{
		return LinkedLayer::Data::Type::Alias;
	}
	PSAPI_LOG_ERROR("LinkedLayer", "Unable to decode Linked Layer type '%s', aborting parsing", key.string().c_str());
	return LinkedLayer::Data::Type::Data;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayer::Data::writeType(File& document, Type type) const
{
	if (type == Type::Data)
	{
		Signature("liFD").write(document);
	}
	else if (type == Type::External)
	{
		Signature("liFE").write(document);
	}
	else if (type == Type::Alias)
	{
		Signature("liFA").write(document);
	}
	else
	{
		PSAPI_LOG_ERROR("LinkedLayer", "Unknown LinkedLayer type encountered while writing to file, aborting write.");
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerTaggedBlock::read(File& document, const FileHeader& header, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature, const uint16_t padding /*= 1u*/)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	uint64_t toRead = 0;
	if (m_Key == Enum::TaggedBlockKey::lrLinked || (m_Key == Enum::TaggedBlockKey::lrLinked_8Byte && header.m_Version == Enum::Version::Psd))
	{
		uint32_t length = ReadBinaryData<uint32_t>(document);
		length = RoundUpToMultiple<uint32_t>(length, padding);
		toRead = length;
		m_Length = length;
		TaggedBlock::totalSize(static_cast<size_t>(length) + 4u + 4u + 4u);
	}
	else if (m_Key == Enum::TaggedBlockKey::lrLinked_8Byte && header.m_Version == Enum::Version::Psb)
	{
		uint64_t length = ReadBinaryData<uint64_t>(document);
		length = RoundUpToMultiple<uint64_t>(length, padding);
		toRead = length;
		m_Length = length;
		TaggedBlock::totalSize(static_cast<size_t>(length) + 4u + 4u + 4u);
	}
	else
	{
		PSAPI_LOG_ERROR("LinkedLayer", "Unknown tagged block key, aborting parsing");
	}

	// A linked Layer tagged block may contain any number of LinkedLayers, and there is no explicit
	// number of layers we must keep reading LinkedLayers until we've reached the end of the taggedblock
	uint64_t endOffset = document.getOffset() + toRead;
	// Need to be able to read at least 8 bytes in order to read another block
	while (document.getOffset() < (endOffset - 8))
	{
		LinkedLayer::Data data;
		data.read(document);
		m_LayerData.push_back(std::move(data));
	}

	document.setOffset(endOffset);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerTaggedBlock::write(File& document, const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("lnk2").m_Value);

	auto lenOffset = document.getOffset();
	if (header.m_Version == Enum::Version::Psd)
	{
		WriteBinaryData<uint32_t>(document, 0u);
	}
	else
	{
		WriteBinaryData<uint64_t>(document, 0u);
	}

	for (auto& item : m_LayerData)
	{
		item.write(document);
	}

	// Write out the length block as well as any padding. This essentially skips back to the point where we wrote the 
	// zero-sized length block and writes it back out but now with the actual section length
	if (header.m_Version == Enum::Version::Psd)
	{
		Impl::writeLengthBlock<uint32_t>(document, lenOffset, document.getOffset(), 1u);
	}
	else
	{
		Impl::writeLengthBlock<uint64_t>(document, lenOffset, document.getOffset(), 1u);
	}
}

PSAPI_NAMESPACE_END