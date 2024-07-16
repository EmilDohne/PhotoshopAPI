#include "DescriptorStructure.h"


#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"

PSAPI_NAMESPACE_BEGIN



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::vector<char> Impl::readKey(File& document)
{
	return ReadBinaryArray<char>(document, 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Impl::writeKey(File& document, std::vector<char> key)
{
	if (key.size() != 4u)
		PSAPI_LOG_ERROR("Descriptor", "Error when writing key, key did not have a size of 4, instead got a size of %zu", key.size());
	WriteBinaryArray(document, key);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::string Impl::readLengthDenotedKey(File& document)
{
	auto keySize = ReadBinaryData<uint32_t>(document);
	std::vector<char> key;
	if (keySize)
		key = ReadBinaryArray<char>(document, keySize);
	else
		key = ReadBinaryArray<char>(document, 4u);

	return std::string(key.begin(), key.end());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Impl::writeLengthDenotedKey(File& document, const std::string& key)
{
	std::vector<char> vec(key.begin(), key.end());
	if (vec.size() == 4u)
		WriteBinaryData<uint32_t>(document, 0u);
	else
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(vec.size()));
	WriteBinaryArray(document, vec);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::vector<char> Impl::scanToNextDescriptor(File& document, size_t maxScanDist /*= 1024u */)
{
	size_t beginOffset = document.getOffset();
	auto buffer = ReadBinaryArray<char>(document, maxScanDist);
	for (const auto& [type, items] : DescriptorItems::descriptorKeys)
	{
		for (const auto& key : items)
		{
			auto it = std::search(buffer.begin(), buffer.begin(), key.begin(), key.end());
			if (it != buffer.end())
			{
				size_t position = std::distance(buffer.begin(), it);
				document.setOffset(beginOffset + position);
				return std::vector<char>(buffer.begin(), it);
			}
		}
	}
	PSAPI_LOG_ERROR("Descriptor", "Unable to find another known descriptor key while seeking" \
					" with a maximum scan distance of% zu from file offset% zu", maxScanDist, beginOffset);
	return std::vector<char>();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
DescriptorItems::OSTypes DescriptorItems::getOSTypeFromKey(std::vector<char> key)
{
	if (key.size() != 4u)
		PSAPI_LOG_ERROR("Descriptor", "Invalid length of OSType key passed, expected 4 but got %zu instead", key.size());

	for (const auto& [type, items] : DescriptorItems::descriptorKeys)
	{
		for (const auto& osKey : items)
		{
			if (key == osKey)
				return type;
		}
	}
	PSAPI_LOG_ERROR("Descriptor", "Unable to retrieve a OS type from key '%c%c%c%c'", key[0], key[1], key[2], key[3]);
	return OSTypes::RawData;
}




// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::tuple<std::string, DescriptorItems::ItemVariant> DescriptorItems::read(File& document, bool readKey/* = true */)
{
	// Each descriptor has a key as well as a OSType which is the data type it actually is,
	// after this we dispatch to the actual read function
	std::string key = "";
	if (readKey)
		key = Impl::readLengthDenotedKey(document);
	auto ostype = Impl::readKey(document);
	auto osTypeEnum = getOSTypeFromKey(ostype);
	if (osTypeEnum == OSTypes::Double)
	{
		return std::make_tuple(key, ReadBinaryData<double>(document));
	}
	else if (osTypeEnum == OSTypes::Integer)
	{
		return std::make_tuple(key, ReadBinaryData<int32_t>(document));
	}
	else if (osTypeEnum == OSTypes::LargeInteger)
	{
		return std::make_tuple(key, ReadBinaryData<int64_t>(document));
	}
	else if (osTypeEnum == OSTypes::Boolean)
	{
		return std::make_tuple(key, ReadBinaryData<bool>(document));
	}
	else if (osTypeEnum == OSTypes::Alias)
	{
		// An alias is basically just raw data but even though its just
		// a length field with some raw data we need to disambiguate it through
		// the ostype so we can write it out correctly
		RawData alias(key, ostype);
		alias.read(document);
		return std::make_tuple(key, alias);
	}
	else if (osTypeEnum == OSTypes::UnitFloat)
	{
		UnitFloat unitFloat(key, ostype);
		unitFloat.read(document);
		return std::make_tuple(key, unitFloat);
	}
	else if (osTypeEnum == OSTypes::UnitFloats)
	{
		UnitFloats unitFloats(key, ostype);
		unitFloats.read(document);
		return std::make_tuple(key, unitFloats);
	}
	else if (osTypeEnum == OSTypes::Class)
	{
		Class clss(key, ostype);
		clss.read(document);
		return std::make_tuple(key, clss);
	}
	else if (osTypeEnum == OSTypes::Descriptor)
	{
		Descriptor descriptor(key, ostype);
		descriptor.read(document);
		return std::make_tuple(key, descriptor);
	}
	else if (osTypeEnum == OSTypes::ObjectArray)
	{
		ObjectArray objectarray(key, ostype);
		objectarray.read(document);
		return std::make_tuple(key, objectarray);
	}
	else if (osTypeEnum == OSTypes::Enumerated)
	{
		Enumerated enumerated(key, ostype);
		enumerated.read(document);
		return std::make_tuple(key, enumerated);
	}
	else if (osTypeEnum == OSTypes::EnumeratedReference)
	{
		EnumeratedReference enumeratedReference(key, ostype);
		enumeratedReference.read(document);
		return std::make_tuple(key, enumeratedReference);
	}
	else if (osTypeEnum == OSTypes::RawData)
	{
		RawData rawdata(key, ostype);
		rawdata.read(document);
		return std::make_tuple(key, rawdata);
	}
	else if (osTypeEnum == OSTypes::List)
	{
		List list(key, ostype);
		list.read(document);
		return std::make_tuple(key, list);
	}
	else if (osTypeEnum == OSTypes::Property)
	{
		Property property(key, ostype);
		property.read(document);
		return std::make_tuple(key, property);
	}
	else if (osTypeEnum == OSTypes::Offset)
	{
		Offset offset(key, ostype);
		offset.read(document);
		return std::make_tuple(key, offset);
	}
	else if (osTypeEnum == OSTypes::Identifier)
	{
		Identifier ident(key, ostype);
		ident.read(document);
		return std::make_tuple(key, ident);
	}
	else if (osTypeEnum == OSTypes::Index)
	{
		Index index(key, ostype);
		index.read(document);
		return std::make_tuple(key, index);
	}
	else if (osTypeEnum == OSTypes::Name)
	{
		Name name(key, ostype);
		name.read(document);
		return std::make_tuple(key, name);
	}
	else if (osTypeEnum == OSTypes::String)
	{
		UnicodeString str{};
		str.read(document, 1u);
		return std::make_tuple(key, str);
	}
	else
	{
		PSAPI_LOG_ERROR("Descriptor", "Unable to find type match for OSType '%c%c%c%c' while searching key '%s'",
			ostype.at(0), ostype.at(1), ostype.at(2), ostype.at(3), key.c_str());
		return std::make_tuple(key, 0u);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Property::read(File& document)
{
	m_Name.read(document, 1u);
	m_ClassID = Impl::readLengthDenotedKey(document);
	m_KeyID = Impl::readLengthDenotedKey(document);
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::UnitFloat::read(File& document)
{
	std::vector<uint8_t> unitTypeData = ReadBinaryArray<uint8_t>(document, 4u);
	std::string unitTypeKey(unitTypeData.begin(), unitTypeData.end());
	try
	{
		m_UnitType = UnitFloatTypeMap.at(unitTypeKey);
	}
	catch (const std::out_of_range& e)
	{
		PSAPI_LOG_ERROR("UnitFloat", "Unknown key '%s' encountered while parsing UnitFloat struct", unitTypeKey.c_str());
	}
	m_Value = ReadBinaryData<double>(document);
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::UnitFloat::write(File& document)
{
	auto unitTypeKey = UnitFloatTypeMap.at(m_UnitType);
	// TODO: this could very well insert a null-termination char at the end, please investigate
	std::vector<uint8_t> unitTypeData(unitTypeKey.begin(), unitTypeKey.end());
	WriteBinaryArray(document, unitTypeData);
	WriteBinaryData<double>(document, m_Value);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::UnitFloats::read(File& document)
{
	std::vector<uint8_t> unitTypeData = ReadBinaryArray<uint8_t>(document, 4u);
	std::string unitTypeKey(unitTypeData.begin(), unitTypeData.end());
	try
	{
		m_UnitType = UnitFloatTypeMap.at(unitTypeKey);
	}
	catch (const std::out_of_range& e)
	{
		PSAPI_LOG_ERROR("UnitFloat", "Unknown key '%s' encountered while parsing UnitFloats struct", unitTypeKey.c_str());
	}
	uint32_t count = ReadBinaryData<uint32_t>(document);
	m_Values = ReadBinaryArray<double>(document, count * sizeof(double));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::UnitFloats::write(File& document)
{
	auto unitTypeKey = UnitFloatTypeMap.at(m_UnitType);
	// TODO: this could very well insert a null-termination char at the end, please investigate
	std::vector<uint8_t> unitTypeData(unitTypeKey.begin(), unitTypeKey.end());
	WriteBinaryArray(document, unitTypeData);
	WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_Values.size()));
	WriteBinaryArray<double>(document, m_Values);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::RawData::read(File& document)
{
	auto size = ReadBinaryData<uint32_t>(document);
	m_Data = ReadBinaryArray<uint8_t>(document, size);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::RawData::write(File& document)
{
	WriteBinaryData<uint32_t>(document, m_Data.size());
	WriteBinaryArray<uint8_t>(document, m_Data);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Class::read(File& document)
{
	m_Name.read(document, 1u);
	m_ClassID = Impl::readLengthDenotedKey(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Enumerated::read(File& document)
{
	m_TypeID = Impl::readLengthDenotedKey(document);
	m_Enum = Impl::readLengthDenotedKey(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::EnumeratedReference::read(File& document)
{
	m_Name.read(document, 1u);
	m_ClassID = Impl::readLengthDenotedKey(document);
	m_TypeID = Impl::readLengthDenotedKey(document);
	m_Enum = Impl::readLengthDenotedKey(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Offset::read(File& document)
{
	m_Name.read(document, 1u);
	m_ClassID = Impl::readLengthDenotedKey(document);
	m_Offset = ReadBinaryData<uint32_t>(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Identifier::read(File& document)
{
	m_Identifier = ReadBinaryData<int32_t>(document);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Identifier::write(File& document)
{
	WriteBinaryData<int32_t>(document, m_Identifier);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Index::read(File& document)
{
	m_Identifier = ReadBinaryData<int32_t>(document);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Index::write(File& document)
{
	WriteBinaryData<int32_t>(document, m_Identifier);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::Name::read(File& document)
{
	m_Name.read(document, 1u);
	m_ClassID = Impl::readLengthDenotedKey(document);
	m_Value.read(document, 1u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::List::read(File& document)
{
	uint32_t count = ReadBinaryData<uint32_t>(document);
	for (int i = 0; i < count; ++i)
	{
		// Since key will just be "" we can safely ignore it
		auto [_, value] = DescriptorItems::read(document, false);
		m_Items.push_back(std::move(value));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void DescriptorItems::ObjectArray::read(File& document)
{
	m_ItemsCount = ReadBinaryData<uint32_t>(document);
	m_Name.read(document, 1u);
	m_Key = Impl::readLengthDenotedKey(document);
	uint32_t descriptorCount = ReadBinaryData<uint32_t>(document);
	for (uint32_t i = 0; i < descriptorCount; ++i)
	{
		auto [key, value] = DescriptorItems::read(document);
		m_DescriptorItems[key] = value;
	}
}




// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Descriptor::read(File& document)
{
	m_Name.read(document, 1u);
	m_Key = Impl::readLengthDenotedKey(document);
	uint32_t descriptorCount = ReadBinaryData<uint32_t>(document);
	for (uint32_t i = 0; i < descriptorCount; ++i)
	{
		auto [key, value] = DescriptorItems::read(document);
		m_DescriptorItems[key] = value;
	}
}




PSAPI_NAMESPACE_END