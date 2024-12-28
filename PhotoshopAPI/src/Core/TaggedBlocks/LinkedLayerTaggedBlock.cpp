#include "LinkedLayerTaggedBlock.h"

#include "Core/FileIO/LengthMarkers.h"
#include "Util/StringUtil.h"

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerItem::Date::read(File& document)
{
	this->year = ReadBinaryData<uint32_t>(document);
	this->month = ReadBinaryData<uint8_t>(document);
	this->day = ReadBinaryData<uint8_t>(document);
	this->hour = ReadBinaryData<uint8_t>(document);
	this->minute = ReadBinaryData<uint8_t>(document);
	this->seconds = ReadBinaryData<double>(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerItem::Date::write(File& document) const
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
LinkedLayerItem::Date::Date()
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
void LinkedLayerItem::Data::read(File& document)
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
		// From what I can tell this data is just for internal consistency anyways.
		document.skip<uint64_t>();
		if (m_Version > 2)
		{
			m_RawFileBytes = ReadBinaryArray<uint8_t>(document, dataSize);
		}
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

	if (m_Version == 2)
	{
		m_RawFileBytes = ReadBinaryArray<uint8_t>(document, dataSize);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerItem::Data::write(File& document)
{
	Impl::ScopedLengthBlock<uint64_t> len_block(document, 1u);

	writeType(document, m_Type);
	WriteBinaryData<uint32_t>(document, m_Version);

	PascalString(m_UniqueID, 1u).write(document);
	m_FileName.write(document);

	Signature(m_FileType).write(document);
	WriteBinaryData<uint32_t>(document, m_FileCreator);

	// Externally linked files dont hold the image data so we clear the raw file bytes in case they were stored
	if (m_Type == Type::External)
	{
		m_RawFileBytes.clear();
	}
	WriteBinaryData(document, m_RawFileBytes.size());	// This may be 0
	WriteBinaryData<bool>(document, m_FileOpenDescriptor.has_value());

	if (m_FileOpenDescriptor)
	{
		// Descriptor version and descriptor.
		WriteBinaryData<uint32_t>(document, 16u);
		m_FileOpenDescriptor.value().write(document);
	}

	// Write out the data related to the different types of linked data
	if (m_Type == Type::External)
	{
		if (m_LinkedFileDescriptor)
		{
			// Descriptor version and descriptor.
			WriteBinaryData<uint32_t>(document, 16u);
			m_LinkedFileDescriptor.value().write(document);

			// If we didnt populate a specific date we write the default initialized date which is just the current timestamp
			if (m_Version > 3)
			{
				m_Date.value_or(Date{}).write(document);
			}

			// This here is the file size which is probably stored for internal consistency.
			auto file = std::ifstream(m_LinkedFileDescriptor.value().at<UnicodeString>("originalPath").string(), std::ifstream::ate | std::ifstream::binary);
			WriteBinaryData<uint64_t>(document, file.tellg());

			if (m_Version > 2)
			{
				WriteBinaryArray<uint8_t>(document, m_RawFileBytes);
			}
		}
		else
		{
			PSAPI_LOG_ERROR("LinkedLayer", "External file link set as m_Type but m_LinkedFileDescriptor is not populated");
		}
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
		WriteBinaryData<float64_t>(document, m_AssetModTime.value_or(static_cast<float64_t>(0.0f)));
	}
	if (m_Version >= 7)
	{
		WriteBinaryData<bool>(document, m_AssetIsLocked.value_or(false));
	}

	if (m_Version == 2)
	{
		WriteBinaryArray<uint8_t>(document, m_RawFileBytes);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LinkedLayerItem::Type LinkedLayerItem::Data::readType(File& document) const
{
	auto key = Signature::read(document);
	if (key == "liFD")
	{
		return LinkedLayerItem::Type::Data;
	}
	if (key == "liFE")
	{
		return LinkedLayerItem::Type::External;
	}
	if (key == "liFA")
	{
		return LinkedLayerItem::Type::Alias;
	}
	PSAPI_LOG_ERROR("LinkedLayer", "Unable to decode Linked Layer type '%s', aborting parsing", key.string().c_str());
	return LinkedLayerItem::Type::Data;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerItem::Data::writeType(File& document, Type type) const
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
LinkedLayerItem::Data::Data(std::string unique_id, std::filesystem::path filepath, Type type, std::vector<uint8_t> bytes, std::filesystem::path photoshop_file_path)
{
	m_Type = type;
	m_RawFileBytes = std::move(bytes);
	m_UniqueID = unique_id;
	m_FileName = UnicodeString(filepath.filename().string(), 2u);
	m_FileType = generate_file_type(filepath);

	// Generate the version specific parameters
	m_ChildDocumentID.emplace(UnicodeString(generate_random_sequence(), 2u));
	m_AssetModTime.emplace(0.0f);	// appears to just be 0 unless this links to an asset which is unimplemented
	m_AssetIsLocked.emplace(false);

	{
		auto file_open_descriptor = Descriptors::Descriptor("null");
		constexpr int comp_id = -1;
		constexpr int original_comp_id = -1;

		auto comp_info_descriptor = Descriptors::Descriptor("null");
		comp_info_descriptor["compID"] = comp_id;
		comp_info_descriptor["originalCompID"] = original_comp_id;

		file_open_descriptor["compInfo"] = comp_info_descriptor;

		m_FileOpenDescriptor.emplace(std::move(file_open_descriptor));
	}


	if (m_Type == Type::External)
	{
		auto linked_file_descriptor = Descriptors::Descriptor("ExternalFileLink");

		linked_file_descriptor["descVersion"] = static_cast<int32_t>(2);	// Seems to be fixed at 2
		linked_file_descriptor["Nm  "] = m_FileName;

		// Photoshop stores the filepath in 3 different ways, using a URI path, a preferred filesystem path and a relative path.
		// However, even with all of these things photoshop will still show an exclamation mark when loading externally linked files
		// written by the PhotoshopAPI. I assume this is because it is looking for a link to the file in its xml metadata. However transforming
		// this data in photoshop itself will lead to this warning to go away and the data is still live.
		auto filepath_full = "file:///" + filepath.string();
		std::replace(filepath_full.begin(), filepath_full.end(), '\\', '/');

		auto filepath_preferred = filepath;
		filepath_preferred.make_preferred();

		linked_file_descriptor["fullPath"] = UnicodeString(filepath_full, 2u);
		linked_file_descriptor["originalPath"] = UnicodeString(filepath_preferred.string(), 2u);

		auto relpath = std::filesystem::relative(filepath, photoshop_file_path.parent_path());
 		linked_file_descriptor["relPath"] = UnicodeString(relpath.string(), 2u);

		m_LinkedFileDescriptor.emplace(linked_file_descriptor);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::string LinkedLayerItem::Data::generate_file_type(const std::filesystem::path& filepath)
{
	if (!filepath.has_filename() && !filepath.has_extension())
	{
		throw std::invalid_argument("Passed a filepath without a file extension, unable to deduce type from that.");
	}

	auto extension = filepath.extension();

	if (extension == ".jpg" || extension == ".jpeg")
	{
		return "JPEG";
	}
	if (extension == ".png")
	{
		return "png ";	// space is not a mistake
	}
	if (extension == ".tiff" || extension == ".tif")
	{
		return "TIFF";
	}
	if (extension == ".mpo")
	{
		return "MPO ";
	}
	if (extension == ".psd")
	{
		return "8BPS";
	}
	if (extension == ".psb")
	{
		return "8BPB";
	}
	if (extension == ".bmp")
	{
		return "BMP ";
	}
	if (extension == ".dcm")
	{
		return "DCIM";
	}
	if (extension == ".gif")
	{
		return "GIFf";
	}
	if (extension == ".eps")
	{
		return "EPSF";
	}
	if (extension == ".jps")
	{
		return "JPS ";
	}

	// what photoshop considers unknown or maybe doesn't have internal parsers for it skips over here
	// by explicly setting spaces, not zero but spaces.
	return "    ";
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerTaggedBlock::read(File& document, const FileHeader& header, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	uint64_t toRead = 0;
	if (m_Key == Enum::TaggedBlockKey::lrLinked || (m_Key == Enum::TaggedBlockKey::lrLinked_8Byte && header.m_Version == Enum::Version::Psd))
	{
		uint32_t length = ReadBinaryData<uint32_t>(document);
		length = RoundUpToMultiple<uint32_t>(length, 4u);
		toRead = length;
		m_Length = length;
	}
	else if (m_Key == Enum::TaggedBlockKey::lrLinked_8Byte && header.m_Version == Enum::Version::Psb)
	{
		uint64_t length = ReadBinaryData<uint64_t>(document);
		length = RoundUpToMultiple<uint64_t>(length, 4u);
		toRead = length;
		m_Length = length;
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
		LinkedLayerItem::Data data;
		data.read(document);
		m_LayerData.push_back(std::move(data));
	}

	document.setOffset(endOffset);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LinkedLayerTaggedBlock::write(File& document, const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	auto is_linked_externally = m_LinkKey == "lnkE";

	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature(m_LinkKey).m_Value);

	// The regular lnk2 block is 8 bytes in psb while lnkE is always 4 bytes.
	if (is_linked_externally)
	{
		Impl::ScopedLengthBlock<uint32_t> len_block(document, 4u);
		for (auto& item : m_LayerData)
		{
			item.write(document);
		}
	}
	else
	{
		Impl::ScopedLengthBlock<Impl::VariadicSize<uint32_t, uint64_t>> len_block(document, header, 4u);
		for (auto& item : m_LayerData)
		{
			item.write(document);
		}
	}
}


PSAPI_NAMESPACE_END