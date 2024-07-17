#include "DescriptorStructure.h"


#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"

#include "Util/DescriptorUtil.h"

PSAPI_NAMESPACE_BEGIN


namespace Descriptors
{

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
		PROFILE_FUNCTION();
		std::vector<char> vec(key.begin(), key.end());
		// While the Photoshop File Format reference says that 4-byte keys have their length field implicitly set to 0
		// this is sadly not true and instead theres a large list of "known" keys which will have their length field set 
		// to 0 and otherwise they are simply set to 4
		if (vec.size() == 4u)
			if (std::find(knownFourByteKeys.begin(), knownFourByteKeys.end(), key) != std::end(knownFourByteKeys))
				WriteBinaryData<uint32_t>(document, 0u);
			else
				WriteBinaryData<uint32_t>(document, 4u);
		else
			WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(vec.size()));
		WriteBinaryArray(document, vec);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Impl::OSTypes Impl::getOSTypeFromKey(std::vector<char> key)
	{
		if (key.size() != 4u)
			PSAPI_LOG_ERROR("Descriptor", "Invalid length of OSType key passed, expected 4 but got %zu instead", key.size());
		for (const auto& [type, items] : Impl::descriptorKeys)
		{
			for (const auto& osKey : items)
			{
				if (key == osKey)
					return type;
			}
		}
		PSAPI_LOG_ERROR("Descriptor", "Unable to retrieve a OS type from key '%c%c%c%c'", key[0], key[1], key[2], key[3]);
		return Impl::OSTypes::RawData;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::tuple<std::string, DescriptorVariant> Impl::ReadDescriptorVariant(File& document, bool readKey/* = true */)
	{
		// Each descriptor has a key as well as a OSType which is the data type it actually is,
		// after this we dispatch to the actual read function
		std::string key = "";
		if (readKey)
			key = Impl::readLengthDenotedKey(document);
		auto ostype = Impl::readKey(document);
		auto osTypeEnum = getOSTypeFromKey(ostype);
		if (osTypeEnum == Impl::OSTypes::Double)
		{
			return std::make_tuple(key, ReadBinaryData<double>(document));
		}
		else if (osTypeEnum == Impl::OSTypes::Integer)
		{
			return std::make_tuple(key, ReadBinaryData<int32_t>(document));
		}
		else if (osTypeEnum == Impl::OSTypes::LargeInteger)
		{
			return std::make_tuple(key, ReadBinaryData<int64_t>(document));
		}
		else if (osTypeEnum == Impl::OSTypes::Boolean)
		{
			return std::make_tuple(key, ReadBinaryData<bool>(document));
		}
		else if (osTypeEnum == Impl::OSTypes::Alias)
		{
			// An alias is basically just raw data but even though its just
			// a length field with some raw data we need to disambiguate it through
			// the ostype so we can write it out correctly
			RawData alias(key, ostype);
			alias.read(document);
			return std::make_tuple(key, alias);
		}
		else if (osTypeEnum == Impl::OSTypes::UnitFloat)
		{
			UnitFloat unitFloat(key, ostype);
			unitFloat.read(document);
			return std::make_tuple(key, unitFloat);
		}
		else if (osTypeEnum == Impl::OSTypes::UnitFloats)
		{
			UnitFloats unitFloats(key, ostype);
			unitFloats.read(document);
			return std::make_tuple(key, unitFloats);
		}
		else if (osTypeEnum == Impl::OSTypes::Class)
		{
			Class clss(key, ostype);
			clss.read(document);
			return std::make_tuple(key, clss);
		}
		else if (osTypeEnum == Impl::OSTypes::Descriptor)
		{
			Descriptor descriptor(key, ostype);
			descriptor.read(document);
			return std::make_tuple(key, descriptor);
		}
		else if (osTypeEnum == Impl::OSTypes::ObjectArray)
		{
			ObjectArray objectarray(key, ostype);
			objectarray.read(document);
			return std::make_tuple(key, objectarray);
		}
		else if (osTypeEnum == Impl::OSTypes::Enumerated)
		{
			Enumerated enumerated(key, ostype);
			enumerated.read(document);
			return std::make_tuple(key, enumerated);
		}
		else if (osTypeEnum == Impl::OSTypes::EnumeratedReference)
		{
			EnumeratedReference enumeratedReference(key, ostype);
			enumeratedReference.read(document);
			return std::make_tuple(key, enumeratedReference);
		}
		else if (osTypeEnum == Impl::OSTypes::RawData)
		{
			RawData rawdata(key, ostype);
			rawdata.read(document);
			return std::make_tuple(key, rawdata);
		}
		else if (osTypeEnum == Impl::OSTypes::List)
		{
			List list(key, ostype);
			list.read(document);
			return std::make_tuple(key, list);
		}
		else if (osTypeEnum == Impl::OSTypes::Property)
		{
			Property property(key, ostype);
			property.read(document);
			return std::make_tuple(key, property);
		}
		else if (osTypeEnum == Impl::OSTypes::Offset)
		{
			Offset offset(key, ostype);
			offset.read(document);
			return std::make_tuple(key, offset);
		}
		else if (osTypeEnum == Impl::OSTypes::Identifier)
		{
			Identifier ident(key, ostype);
			ident.read(document);
			return std::make_tuple(key, ident);
		}
		else if (osTypeEnum == Impl::OSTypes::Index)
		{
			Index index(key, ostype);
			index.read(document);
			return std::make_tuple(key, index);
		}
		else if (osTypeEnum == Impl::OSTypes::Name)
		{
			Name name(key, ostype);
			name.read(document);
			return std::make_tuple(key, name);
		}
		else if (osTypeEnum == Impl::OSTypes::String)
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
	void Impl::WriteDescriptorVariant(File& document, const std::string& key, DescriptorVariant& value, bool writeKey /*= true*/)
	{
		if (writeKey)
			Impl::writeLengthDenotedKey(document, key);

		if (std::holds_alternative<double>(value))
		{
			auto item = std::get<double>(value);
			auto key = Impl::descriptorKeys.at(Impl::OSTypes::Double)[0];
			WriteBinaryArray(document, key);
			WriteBinaryData<double>(document, item);
		}
		else if (std::holds_alternative<int32_t>(value))
		{
			auto item = std::get<int32_t>(value);
			auto key = Impl::descriptorKeys.at(Impl::OSTypes::Integer)[0];
			WriteBinaryArray(document, key);
			WriteBinaryData<int32_t>(document, item);
		}
		else if (std::holds_alternative<int64_t>(value))
		{
			auto item = std::get<int64_t>(value);
			auto key = Impl::descriptorKeys.at(Impl::OSTypes::LargeInteger)[0];
			WriteBinaryArray(document, key);
			WriteBinaryData<int64_t>(document, item);
		}
		else if (std::holds_alternative<bool>(value))
		{
			auto item = std::get<bool>(value);
			auto key = Impl::descriptorKeys.at(Impl::OSTypes::Boolean)[0];
			WriteBinaryArray(document, key);
			WriteBinaryData<bool>(document, item);
		}
		else if (std::holds_alternative<RawData>(value))
		{
			// This could hold an alias or a rawdata type, we use the getOSKey function
			// so that it gets disambiguated
			WriteDescriptorBaseType(document, std::get<RawData>(value));
		}
		else if (std::holds_alternative<UnitFloat>(value))
		{
			WriteDescriptorBaseType(document, std::get<UnitFloat>(value));
		}
		else if (std::holds_alternative<UnitFloats>(value))
		{
			WriteDescriptorBaseType(document, std::get<UnitFloats>(value));
		}
		else if (std::holds_alternative<Class>(value))
		{
			WriteDescriptorBaseType(document, std::get<Class>(value));
		}
		else if (std::holds_alternative<Descriptor>(value))
		{
			WriteDescriptorBaseType(document, std::get<Descriptor>(value));
		}
		else if (std::holds_alternative<ObjectArray>(value))
		{
			WriteDescriptorBaseType(document, std::get<ObjectArray>(value));
		}
		else if (std::holds_alternative<Enumerated>(value))
		{
			WriteDescriptorBaseType(document, std::get<Enumerated>(value));
		}
		else if (std::holds_alternative<EnumeratedReference>(value))
		{
			WriteDescriptorBaseType(document, std::get<EnumeratedReference>(value));
		}
		else if (std::holds_alternative<List>(value))
		{
			WriteDescriptorBaseType(document, std::get<List>(value));
		}
		else if (std::holds_alternative<Property>(value))
		{
			WriteDescriptorBaseType(document, std::get<Property>(value));
		}
		else if (std::holds_alternative<Offset>(value))
		{
			WriteDescriptorBaseType(document, std::get<Offset>(value));
		}
		else if (std::holds_alternative<Identifier>(value))
		{
			WriteDescriptorBaseType(document, std::get<Identifier>(value));
		}
		else if (std::holds_alternative<Index>(value))
		{
			WriteDescriptorBaseType(document, std::get<Index>(value));
		}
		else if (std::holds_alternative<Name>(value))
		{
			WriteDescriptorBaseType(document, std::get<Name>(value));
		}
		else if (std::holds_alternative<UnicodeString>(value))
		{
			auto key = Impl::descriptorKeys.at(Impl::OSTypes::String)[0];
			WriteBinaryArray(document, key);
			const auto& str = std::get<UnicodeString>(value);
			str.write(document, 1u);
		}
		else
		{
			PSAPI_LOG_ERROR("Descriptor", "Unable to write DescriptorItem to disk, please ensure a proper parser was registered" \
				" for it");
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	DescriptorVariant& KeyValueMixin::operator[](const std::string_view key) noexcept
	{
		for (auto& [_key, value] : m_DescriptorItems)
		{
			if (_key == key)
				return value;
		}
		m_DescriptorItems.push_back(std::make_pair(key, DescriptorVariant{}));
		return m_DescriptorItems.back().second;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	DescriptorVariant& KeyValueMixin::at(const std::string_view key)
	{
		for (auto& [_key, value] : m_DescriptorItems)
		{
			if (_key == key)
				return value;
		}
		PSAPI_LOG_ERROR("Descriptor", "Unable to find child node with key '%s' in Descriptor", std::string(key).c_str());
		return m_DescriptorItems.back().second;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void KeyValueMixin::insert(std::pair<std::string, DescriptorVariant> item) noexcept
	{
		// If the key already exists we return
		if (contains(item.first))
			return;
		m_DescriptorItems.push_back(item);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void KeyValueMixin::insert(std::string key, DescriptorVariant value) noexcept
	{
		insert(std::make_pair(key, value));
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void KeyValueMixin::insert_or_assign(std::pair<std::string, DescriptorVariant> item) noexcept
	{
		// If the key already exists we simply override the value
		if (contains(item.first))
		{
			auto& valueRef = at(item.first);
			valueRef = item.second;
			return;
		}
		m_DescriptorItems.push_back(item);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void KeyValueMixin::insert_or_assign(std::string key, DescriptorVariant value) noexcept
	{
		insert_or_assign(std::make_pair(key, value));
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void KeyValueMixin::remove(int index)
	{
		m_DescriptorItems.erase(m_DescriptorItems.begin() + index);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void KeyValueMixin::remove(std::string_view key)
	{
		size_t idx = 0;
		for (const auto& [_key, _] : m_DescriptorItems)
		{
			if (_key == key)
			{
				remove(idx);
				return;
			}
			++idx;
		}
		PSAPI_LOG_WARNING("Descriptor", "Key '%s' was not found and could therefore not be removed from the map",
			std::string(key).c_str());
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool KeyValueMixin::contains(std::string_view key) const noexcept
	{
		for (const auto& [_key, value] : m_DescriptorItems)
		{
			if (_key == key)
				return true;
		}
		return false;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	size_t KeyValueMixin::size() const noexcept
	{
		return m_DescriptorItems.size();
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool KeyValueMixin::empty() const noexcept
	{
		return size() == 0;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Property::read(File& document)
	{
		m_Name.read(document, 1u);
		m_ClassID = Impl::readLengthDenotedKey(document);
		m_KeyID = Impl::readLengthDenotedKey(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Property::write(File& document)
	{
		m_Name.write(document, 1u);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		Impl::writeLengthDenotedKey(document, m_KeyID);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void UnitFloat::read(File& document)
	{
		std::vector<uint8_t> unitTypeData = ReadBinaryArray<uint8_t>(document, 4u);
		std::string unitTypeKey(unitTypeData.begin(), unitTypeData.end());
		try
		{
			m_UnitType = Impl::UnitFloatTypeMap.at(unitTypeKey);
		}
		catch (const std::out_of_range& e)
		{
			PSAPI_LOG_ERROR("UnitFloat", "Unknown key '%s' encountered while parsing UnitFloat struct", unitTypeKey.c_str());
		}
		m_Value = ReadBinaryData<double>(document);
	}



	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void UnitFloat::write(File& document)
	{
		auto unitTypeKey = Impl::UnitFloatTypeMap.at(m_UnitType);
		// TODO: this could very well insert a null-termination char at the end, please investigate
		std::vector<uint8_t> unitTypeData(unitTypeKey.begin(), unitTypeKey.end());
		WriteBinaryArray(document, unitTypeData);
		WriteBinaryData<double>(document, m_Value);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void UnitFloats::read(File& document)
	{
		std::vector<uint8_t> unitTypeData = ReadBinaryArray<uint8_t>(document, 4u);
		std::string unitTypeKey(unitTypeData.begin(), unitTypeData.end());
		try
		{
			m_UnitType = Impl::UnitFloatTypeMap.at(unitTypeKey);
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
	void UnitFloats::write(File& document)
	{
		auto unitTypeKey = Impl::UnitFloatTypeMap.at(m_UnitType);
		// TODO: this could very well insert a null-termination char at the end, please investigate
		std::vector<uint8_t> unitTypeData(unitTypeKey.begin(), unitTypeKey.end());
		WriteBinaryArray(document, unitTypeData);
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_Values.size()));
		WriteBinaryArray<double>(document, m_Values);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void RawData::read(File& document)
	{
		auto size = ReadBinaryData<uint32_t>(document);
		m_Data = ReadBinaryArray<uint8_t>(document, size);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void RawData::write(File& document)
	{
		WriteBinaryData<uint32_t>(document, m_Data.size());
		WriteBinaryArray<uint8_t>(document, m_Data);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Class::read(File& document)
	{
		m_Name.read(document, 1u);
		m_ClassID = Impl::readLengthDenotedKey(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Class::write(File& document)
	{
		m_Name.write(document, 1u);
		Impl::writeLengthDenotedKey(document, m_ClassID);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Enumerated::read(File& document)
	{
		m_TypeID = Impl::readLengthDenotedKey(document);
		m_Enum = Impl::readLengthDenotedKey(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Enumerated::write(File& document)
	{
		Impl::writeLengthDenotedKey(document, m_TypeID);
		Impl::writeLengthDenotedKey(document, m_Enum);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void EnumeratedReference::read(File& document)
	{
		m_Name.read(document, 1u);
		m_ClassID = Impl::readLengthDenotedKey(document);
		m_TypeID = Impl::readLengthDenotedKey(document);
		m_Enum = Impl::readLengthDenotedKey(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void EnumeratedReference::write(File& document)
	{
		m_Name.write(document, 1u);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		Impl::writeLengthDenotedKey(document, m_TypeID);
		Impl::writeLengthDenotedKey(document, m_Enum);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Offset::read(File& document)
	{
		m_Name.read(document, 1u);
		m_ClassID = Impl::readLengthDenotedKey(document);
		m_Offset = ReadBinaryData<uint32_t>(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Offset::write(File& document)
	{
		m_Name.read(document, 1u);
		m_ClassID = Impl::readLengthDenotedKey(document);
		m_Offset = ReadBinaryData<uint32_t>(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Identifier::read(File& document)
	{
		m_Identifier = ReadBinaryData<int32_t>(document);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Identifier::write(File& document)
	{
		WriteBinaryData<int32_t>(document, m_Identifier);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Index::read(File& document)
	{
		m_Identifier = ReadBinaryData<int32_t>(document);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Index::write(File& document)
	{
		WriteBinaryData<int32_t>(document, m_Identifier);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Name::read(File& document)
	{
		m_Name.read(document, 1u);
		m_ClassID = Impl::readLengthDenotedKey(document);
		m_Value.read(document, 1u);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Name::write(File& document)
	{
		m_Name.write(document, 1u);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		m_Value.write(document, 1u);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void List::read(File& document)
	{
		uint32_t count = ReadBinaryData<uint32_t>(document);
		for (int i = 0; i < count; ++i)
		{
			// Since key will just be "" we can safely ignore it
			auto [_, value] = Impl::ReadDescriptorVariant(document, false);
			m_Items.push_back(std::move(value));
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void List::write(File& document)
	{
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_Items.size()));
		for (auto& item : m_Items)
		{
			Impl::WriteDescriptorVariant(document, "", item, false);
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void ObjectArray::read(File& document)
	{
		m_ItemsCount = ReadBinaryData<uint32_t>(document);
		m_Name.read(document, 1u);
		m_ClassID = Impl::readLengthDenotedKey(document);
		uint32_t descriptorCount = ReadBinaryData<uint32_t>(document);
		for (uint32_t i = 0; i < descriptorCount; ++i)
		{
			auto [key, value] = Impl::ReadDescriptorVariant(document);
			m_DescriptorItems.push_back(std::make_pair(key, value));
		}
	}



	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void ObjectArray::write(File& document)
	{
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_ItemsCount));
		m_Name.write(document, 1u);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_DescriptorItems.size()));
		for (auto& [key, value] : m_DescriptorItems)
		{
			Impl::WriteDescriptorVariant(document, key, value);
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
			auto [key, value] = Impl::ReadDescriptorVariant(document);
			m_DescriptorItems.push_back(std::make_pair(key, value));
		}
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Descriptor::write(File& document)
	{
		m_Name.write(document, 1u);
		Impl::writeLengthDenotedKey(document, m_Key);
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_DescriptorItems.size()));
		for (auto& [key, value] : m_DescriptorItems)
		{
			Impl::WriteDescriptorVariant(document, key, value);
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptor::Descriptor(std::string key, std::vector<char> osKey, std::vector<std::pair<std::string, DescriptorVariant>> items)
		: DescriptorBase(key, osKey)
	{
		m_DescriptorItems = std::move(items);
	}

}

PSAPI_NAMESPACE_END