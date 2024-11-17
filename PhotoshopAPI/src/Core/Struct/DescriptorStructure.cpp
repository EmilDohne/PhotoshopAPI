#include "DescriptorStructure.h"


#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"

#include "Util/DescriptorUtil.h"

#include "fmt/core.h"

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
		{
			key = ReadBinaryArray<char>(document, keySize);
		}
		else
		{
			key = ReadBinaryArray<char>(document, 4u);
		}

		return std::string(key.begin(), key.end());
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Impl::writeLengthDenotedKey(File& document, const std::string& key)
	{
		PSAPI_PROFILE_FUNCTION();
		std::vector<char> vec(key.begin(), key.end());
		// While the Photoshop File Format reference says that 4-byte keys have their length field implicitly set to 0
		// this is sadly not true and instead theres a large list of "known" keys which will have their length field set 
		// to 0 and otherwise they are simply set to 4
		if (vec.size() == 4u)
		{
			if (std::find(knownFourByteKeys.begin(), knownFourByteKeys.end(), key) != std::end(knownFourByteKeys))
			{
				WriteBinaryData<uint32_t>(document, 0u);
			}
			else
			{
				WriteBinaryData<uint32_t>(document, 4u);
			}
		}
		else
		{
			WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(vec.size()));
		}
		WriteBinaryArray(document, vec);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Impl::OSTypes Impl::getOSTypeFromKey(std::vector<char> key)
	{
		if (key.size() != 4u)
			PSAPI_LOG_ERROR("Descriptor", "Invalid length of OSType key passed, expected 4 but got %zu instead", key.size());
		for (const auto& [type, osKey] : Impl::descriptorKeys)
		{
			if (key == osKey)
			{
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
		{
			key = Impl::readLengthDenotedKey(document);
		}
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
		else if (
			osTypeEnum == Impl::OSTypes::Class_1 ||
			osTypeEnum == Impl::OSTypes::Class_2 ||
			osTypeEnum == Impl::OSTypes::Class_3
			)
		{
			Class clss(key, ostype);
			clss.read(document);
			return std::make_tuple(key, clss);
		}
		else if (osTypeEnum == Impl::OSTypes::Descriptor)
		{
			Descriptor descriptor(key);
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
		else if (osTypeEnum == Impl::OSTypes::Reference)
		{
			Reference reference(key, ostype);
			reference.read(document);
			return std::make_tuple(key, reference);
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
	void Impl::WriteDescriptorVariant(File& document, const std::string& key, const DescriptorVariant& value, bool writeKey /*= true*/)
	{
		if (writeKey)
		{
			Impl::writeLengthDenotedKey(document, key);
		}

		if (std::holds_alternative<double>(value))
		{
			auto item = std::get<double>(value);
			auto descriptorKey = Impl::descriptorKeys.at(Impl::OSTypes::Double);
			WriteBinaryArray(document, descriptorKey);
			WriteBinaryData<double>(document, item);
		}
		else if (std::holds_alternative<int32_t>(value))
		{
			auto item = std::get<int32_t>(value);
			auto descriptorKey = Impl::descriptorKeys.at(Impl::OSTypes::Integer);
			WriteBinaryArray(document, descriptorKey);
			WriteBinaryData<int32_t>(document, item);
		}
		else if (std::holds_alternative<int64_t>(value))
		{
			auto item = std::get<int64_t>(value);
			auto descriptorKey = Impl::descriptorKeys.at(Impl::OSTypes::LargeInteger);
			WriteBinaryArray(document, descriptorKey);
			WriteBinaryData<int64_t>(document, item);
		}
		else if (std::holds_alternative<bool>(value))
		{
			auto item = std::get<bool>(value);
			auto descriptorKey = Impl::descriptorKeys.at(Impl::OSTypes::Boolean);
			WriteBinaryArray(document, descriptorKey);
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
		else if (std::holds_alternative<Reference>(value))
		{
			WriteDescriptorBaseType(document, std::get<Reference>(value));
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
			auto descriptorKey = Impl::descriptorKeys.at(Impl::OSTypes::String);
			WriteBinaryArray(document, descriptorKey);
			const auto& str = std::get<UnicodeString>(value);
			str.write(document);
		}
		else
		{
			PSAPI_LOG_ERROR("Descriptor", "Unable to write DescriptorItem to disk, please ensure a proper parser was registered" \
				" for it");
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered DescriptorBase::to_json(const DescriptorVariant& variant)
	{
		return std::visit([](const auto& value) -> json_ordered 
			{
			using T = std::decay_t<decltype(value)>;

			// If the type is derived from DescriptorBase, call its to_json method
			if constexpr (std::is_base_of_v<DescriptorBase, T>) 
			{
				return value.to_json();
			}
			// Special handling for integral and other built-in types
			else if constexpr (std::is_same_v<T, int32_t>) 
			{
				return {
					{"implementation", { {"_data_type", "int32_t"} }},
					{ "value", value }
				};
			}
			else if constexpr (std::is_same_v<T, int64_t>) 
			{
				return {
					{"implementation", { {"_data_type", "int64_t"} }},
					{ "value", value }
				};
			}
			else if constexpr (std::is_same_v<T, double>) 
			{
				return {
					{"implementation", { {"_data_type", "double"} }},
					{ "value", value }
				};
			}
			else if constexpr (std::is_same_v<T, bool>) 
			{
				return 
				{
					{"implementation", { {"_data_type", "bool"} }},
					{ "value", value }
				};
			}
			else if constexpr (std::is_same_v<T, UnicodeString>)
			{
				return
				{
					{"implementation", { {"_data_type", "UnicodeString"} }},
					{ "value", value.getString()}
				};
			}
			else 
			{
				PSAPI_LOG_ERROR("DescriptorBase", "No matching to_json() function or builtin found for type");
				return {};
			}
			}, variant);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered DescriptorBase::get_json_repr(std::string data_type) const
	{
		json_ordered data{};

		data["_data_type"]	= data_type;
		data["_key"]		= m_Key;
		data["_os_key"]		= std::string(m_OSKey.begin(), m_OSKey.end());

		return data;
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
		m_DescriptorItems.push_back(std::make_pair(std::string(key), DescriptorVariant{}));
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
	const DescriptorVariant& KeyValueMixin::at(const std::string_view key) const
	{
		for (const auto& [_key, value] : m_DescriptorItems)
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
	void KeyValueMixin::remove(size_t index)
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
	Property::Property(std::string key, std::vector<char> osKey, std::string name, std::string classID, std::string keyID)
		: DescriptorBase(key, osKey)
	{
		m_Name = UnicodeString(name, 1u);
		m_ClassID = classID;
		m_KeyID = keyID;
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
	void Property::write(File& document) const
	{
		m_Name.write(document);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		Impl::writeLengthDenotedKey(document, m_KeyID);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Property::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Property");
		data["name"] = m_Name.getString();
		data["class_id"] = m_ClassID;
		data["key_id"] = m_KeyID;

		return data;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Property::operator==(const Property& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}
		if (m_Name != other.m_Name)
		{
			return false;
		}
		if (m_ClassID != other.m_ClassID)
		{
			return false;
		};
		if (m_KeyID != other.m_KeyID)
		{
			return false;
		}
		return true;
	}

	

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	UnitFloat::UnitFloat(std::string key, std::vector<char> osKey, Impl::UnitFloatType type, double value)
		: DescriptorBase(key, osKey)
	{
		m_UnitType = type;
		m_Value = value;
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
		catch ([[maybe_unused]] const std::out_of_range& e)
		{
			PSAPI_LOG_ERROR("UnitFloat", "Unknown key '%s' encountered while parsing UnitFloat struct", unitTypeKey.c_str());
		}
		m_Value = ReadBinaryData<double>(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void UnitFloat::write(File& document) const
	{
		auto unitTypeKey = Impl::UnitFloatTypeMap.at(m_UnitType);
		// TODO: this could very well insert a null-termination char at the end, please investigate
		std::vector<uint8_t> unitTypeData(unitTypeKey.begin(), unitTypeKey.end());
		WriteBinaryArray(document, unitTypeData);
		WriteBinaryData<double>(document, m_Value);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered UnitFloat::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("UnitFloat");
		data["unit_type"] = Impl::UnitFloatTypeMap.at(m_UnitType);
		data["value"] = m_Value;

		return data;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool UnitFloat::operator==(const UnitFloat& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		bool unit_type_equal = m_UnitType == other.m_UnitType;
		// Use a scaled epsilon as a general solution to comparing the two values
		double epsilon = 1e-9;
		bool value_equal = std::fabs(m_Value - other.m_Value) <= (epsilon * std::max(std::fabs(m_Value), std::fabs(other.m_Value)));
		return unit_type_equal && value_equal;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	UnitFloats::UnitFloats(std::string key, std::vector<char> osKey, Impl::UnitFloatType type, std::vector<double> values)
		: DescriptorBase(key, osKey)
	{
		m_UnitType = type;
		m_Values = values;
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
		catch ([[maybe_unused]] const std::out_of_range& e)
		{
			PSAPI_LOG_ERROR("UnitFloat", "Unknown key '%s' encountered while parsing UnitFloats struct", unitTypeKey.c_str());
		}
		uint32_t count = ReadBinaryData<uint32_t>(document);
		m_Values = ReadBinaryArray<double>(document, count * sizeof(double));
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void UnitFloats::write(File& document) const
	{
		auto unitTypeKey = Impl::UnitFloatTypeMap.at(m_UnitType);
		// TODO: this could very well insert a null-termination char at the end, please investigate
		std::vector<uint8_t> unitTypeData(unitTypeKey.begin(), unitTypeKey.end());
		WriteBinaryArray(document, unitTypeData);
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_Values.size()));
		auto values = m_Values;
		WriteBinaryArray<double>(document, values);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered UnitFloats::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("UnitFloats");
		data["unit_type"] = Impl::UnitFloatTypeMap.at(m_UnitType);
		data["values"] = m_Values;

		return data;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool UnitFloats::operator==(const UnitFloats& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		bool type_equal = m_UnitType == other.m_UnitType;
		bool values_equal = m_Values == other.m_Values;
		return type_equal && values_equal;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	RawData::RawData(std::string key, std::vector<char> osKey, std::vector<uint8_t> data)
		: DescriptorBase(key, osKey)
	{
		m_Data = data;
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
	void RawData::write(File& document) const
	{
		if (m_Data.size() > std::numeric_limits<uint32_t>::max())
		{
			PSAPI_LOG_ERROR("RawDataDescriptor", "Data size would overflow numeric limit of uint32_t");
		}
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_Data.size()));
		auto data = m_Data;
		WriteBinaryArray<uint8_t>(document, data);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered RawData::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("RawData");
		
		// If the data is larger than a threshold we actually want to truncate it.
		// We set this to an arbitrary limit of 512 for now
		if (m_Data.size() > 512)
		{
			const auto first = m_Data.front();
			const auto last = m_Data.back();

			data["data"] = fmt::format("{}...{}", first, last);
		}
		else
		{
			data["data"] = m_Data;
		}
		return data;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool RawData::operator==(const RawData& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}
		return m_Data == other.m_Data;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Class::Class(std::string key, std::vector<char> osKey, std::string name, std::string classID)
		: DescriptorBase(key, osKey)
	{
		m_Name = UnicodeString(name, 1u);
		m_ClassID = classID;
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
	void Class::write(File& document) const
	{
		m_Name.write(document);
		Impl::writeLengthDenotedKey(document, m_ClassID);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Class::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Class");
		data["name"] = m_Name.getString();
		data["class_id"] = m_ClassID;

		return data;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Class::operator==(const Class& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}
		if (m_Name != other.m_Name)
		{
			return false;
		}
		if (m_ClassID != other.m_ClassID)
		{
			return false;
		};
		return true;
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
	void Enumerated::write(File& document) const
	{
		Impl::writeLengthDenotedKey(document, m_TypeID);
		Impl::writeLengthDenotedKey(document, m_Enum);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Enumerated::Enumerated(std::string key, std::vector<char> osKey, std::string typeID, std::string enumerator)
		: DescriptorBase(key, osKey)
	{
		m_TypeID = typeID;
		m_Enum = enumerator;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Enumerated::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Enumerated");
		data["type_id"] = m_TypeID;
		data["enum"] = m_Enum;

		return data;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Enumerated::operator==(const Enumerated& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}
		if (m_Enum != other.m_Enum)
		{
			return false;
		}
		if (m_TypeID != other.m_TypeID)
		{
			return false;
		};
		return true;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	EnumeratedReference::EnumeratedReference(
		std::string key,
		std::vector<char> osKey,
		std::string name,
		std::string classID,
		std::string typeID,
		std::string enumerator)
		: DescriptorBase(key, osKey)
	{
		m_Name = UnicodeString(name, 1u);
		m_ClassID = classID;
		m_TypeID = typeID;
		m_Enum = enumerator;
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
	void EnumeratedReference::write(File& document) const
	{
		m_Name.write(document);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		Impl::writeLengthDenotedKey(document, m_TypeID);
		Impl::writeLengthDenotedKey(document, m_Enum);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered EnumeratedReference::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("EnumeratedReference");
		data["name"] = m_Name.getString();
		data["class_id"] = m_ClassID;
		data["type_id"] = m_TypeID;
		data["enum"] = m_Enum;

		return data;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool EnumeratedReference::operator==(const EnumeratedReference& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}
		if (m_Name != other.m_Name)
		{
			return false;
		}
		if (m_ClassID != other.m_ClassID)
		{
			return false;
		};
		if (m_TypeID != other.m_TypeID)
		{
			return false;
		}
		if (m_Enum != other.m_Enum)
		{
			return false;
		}
		return true;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Offset::Offset(std::string key, std::vector<char> osKey, std::string name, std::string classID, uint32_t offset)
		: DescriptorBase(key, osKey)
	{
		m_Name = UnicodeString(name, 1u);
		m_ClassID = classID;
		m_Offset = offset;
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
	void Offset::write(File& document) const
	{
		m_Name.write(document);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		WriteBinaryData<uint32_t>(document, m_Offset);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Offset::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Offset");
		data["name"] = m_Name.getString();
		data["class_id"] = m_ClassID;
		data["offset"] = m_Offset;

		return data;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Offset::operator==(const Offset& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		if (m_Name != other.m_Name)
		{
			return false;
		}
		if (m_ClassID != other.m_ClassID)
		{
			return false;
		};
		if (m_Offset != other.m_Offset)
		{
			return false;
		}
		return true;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Identifier::read(File& document)
	{
		m_Identifier = ReadBinaryData<int32_t>(document);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Identifier::write(File& document) const
	{
		WriteBinaryData<int32_t>(document, m_Identifier);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Identifier::Identifier(std::string key, std::vector<char> osKey, int32_t identifier)
		: DescriptorBase(key, osKey)
	{
		m_Identifier = identifier;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Identifier::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Identifier");
		data["identifier"] = m_Identifier;

		return data;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Identifier::operator==(const Identifier& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		if (m_Identifier != other.m_Identifier)
		{
			return false;
		}
		return true;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Index::Index(std::string key, std::vector<char> osKey, int32_t identifier)
		: DescriptorBase(key, osKey)
	{
		m_Identifier = identifier;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Index::read(File& document)
	{
		m_Identifier = ReadBinaryData<int32_t>(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Index::write(File& document) const
	{
		WriteBinaryData<int32_t>(document, m_Identifier);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Index::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Index");
		data["identifier"] = m_Identifier;

		return data;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Index::operator==(const Index& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}
		if (m_Identifier != other.m_Identifier)
		{
			return false;
		}
		return true;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Name::Name(std::string key, std::vector<char> osKey, std::string name, std::string classID, std::string value)
		: DescriptorBase(key, osKey)
	{
		m_Name = UnicodeString(name, 1u);
		m_ClassID = classID;
		m_Value = UnicodeString(value, 1u);
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
	void Name::write(File& document) const
	{
		m_Name.write(document);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		m_Value.write(document);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Name::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Name");
		data["name"] = m_Name.getString();
		data["class_id"] = m_ClassID;
		data["value"] = m_Value.getString();

		return data;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Name::operator==(const Name& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		if (m_Name != other.m_Name)
		{
			return false;
		}
		if (m_ClassID != other.m_ClassID)
		{
			return false;
		};
		if (m_Value != other.m_Value)
		{
			return false;
		}
		return true;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	List::List(std::string key, std::vector<char> osKey, std::vector<DescriptorVariant> items)
		: DescriptorBase(key, osKey)
	{
		m_Items = items;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void List::read(File& document)
	{
		uint32_t count = ReadBinaryData<uint32_t>(document);
		for (uint32_t i = 0; i < count; ++i)
		{
			// Since key will just be "" we can safely ignore it
			auto [_, value] = Impl::ReadDescriptorVariant(document, false);
			m_Items.push_back(std::move(value));
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void List::write(File& document) const
	{
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_Items.size()));
		for (auto& item : m_Items)
		{
			Impl::WriteDescriptorVariant(document, "", item, false);
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered List::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("List");

		auto values = json_ordered::array();
		for (const auto& item : m_Items)
		{
			values.push_back(DescriptorBase::to_json(item));
		}
		data["values"] = std::move(values);

		return data;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool List::operator==(const List& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		if (m_Items.size() != other.m_Items.size())
		{
			return false;
		}

		for (size_t i = 0; i < m_Items.size(); ++i)
		{
			bool result = Impl::descriptors_are_equal(m_Items[i], other.m_Items[i]);
			if (!result)
			{
				return false;
			}
		}
		return true;
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
	void ObjectArray::write(File& document) const
	{
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_ItemsCount));
		m_Name.write(document);
		Impl::writeLengthDenotedKey(document, m_ClassID);
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_DescriptorItems.size()));
		for (auto& [key, value] : m_DescriptorItems)
		{
			Impl::WriteDescriptorVariant(document, key, value);
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	ObjectArray::ObjectArray(
		std::string key, 
		std::vector<char> osKey, 
		uint32_t itemsCount, 
		std::string name, 
		std::string classID,
		std::vector<std::pair<std::string, DescriptorVariant>> items)
		: DescriptorBase(key, osKey)
	{
		m_ItemsCount = itemsCount;
		m_Name = UnicodeString(name, 1u);
		m_ClassID = classID;
		m_DescriptorItems = items;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered ObjectArray::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("ObjectArray");
		data["items_count"] = m_ItemsCount;
		data["name"] = m_Name.getString();
		data["class_id"] = m_ClassID;

		auto values = json_ordered::basic_json();
		for (const auto& [key, item] : m_DescriptorItems)
		{
			values[key] = (DescriptorBase::to_json(item));
		}
		data["values"] = std::move(values);

		return data;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool ObjectArray::operator==(const ObjectArray& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		if (m_Name != other.m_Name)
		{
			return false;
		}
		if (m_ClassID != other.m_ClassID)
		{
			return false;
		};
		if (m_ItemsCount != other.m_ItemsCount)
		{
			return false;
		}
		

		if (m_DescriptorItems.size() != other.m_DescriptorItems.size())
		{
			return false;
		}
		
		for (size_t i = 0; i < m_DescriptorItems.size(); ++i)
		{
			auto& pair_self = m_DescriptorItems[i];
			auto& pair_other = other.m_DescriptorItems[i];

			if (std::get<0>(pair_self) != std::get<0>(pair_other))
			{
				return false;
			}

			if (!Impl::descriptors_are_equal(std::get<1>(pair_self), std::get<1>(pair_other)))
			{
				return false;
			}
		}
		return true;
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
	void Descriptor::write(File& document) const
	{
		m_Name.write(document);
		Impl::writeLengthDenotedKey(document, m_Key);
		WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(m_DescriptorItems.size()));
		for (auto& [key, value] : m_DescriptorItems)
		{
			Impl::WriteDescriptorVariant(document, key, value);
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptor::Descriptor(std::string key, std::vector<std::pair<std::string, DescriptorVariant>> items)
		: DescriptorBase(key, Impl::descriptorKeys.at(Impl::OSTypes::Descriptor))
	{
		m_DescriptorItems = std::move(items);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	json_ordered Descriptor::to_json() const
	{
		json_ordered data{};

		data["implementation"] = DescriptorBase::get_json_repr("Descriptor");
		data["name"] = m_Name.getString();

		auto values = json_ordered::basic_json();
		for (const auto& [key, item] : m_DescriptorItems)
		{
			values[key] = (DescriptorBase::to_json(item));
		}
		data["values"] = std::move(values);

		return data;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Descriptor::operator==(const Descriptor& other) const
	{
		if (!DescriptorBase::operator==(other))
		{
			return false;
		}

		if (m_Name != other.m_Name)
		{
			return false;
		}

		if (m_DescriptorItems.size() != other.m_DescriptorItems.size())
		{
			return false;
		}

		for (size_t i = 0; i < m_DescriptorItems.size(); ++i)
		{
			auto& pair_self = m_DescriptorItems[i];
			auto& pair_other = other.m_DescriptorItems[i];

			if (std::get<0>(pair_self) != std::get<0>(pair_other))
			{
				return false;
			}

			if (!Impl::descriptors_are_equal(std::get<1>(pair_self), std::get<1>(pair_other)))
			{
				return false;
			}
		}
		return true;
	}

}

PSAPI_NAMESPACE_END