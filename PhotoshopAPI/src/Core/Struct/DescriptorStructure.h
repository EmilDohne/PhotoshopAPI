#pragma once

#include "Macros.h"

#include "UnicodeString.h"
#include "Section.h"
#include "BidirectionalMap.h"

#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"

#include "Core/Geometry/Point.h"

#include <variant>
#include <unordered_map>
#include <vector>
#include <set>

#include "fmt/core.h"
#include "json.hpp"

PSAPI_NAMESPACE_BEGIN

using json_ordered = nlohmann::ordered_json;


namespace Descriptors
{

	/// Base struct for descriptor items which stores information about the key as well as the OSType it is. All subclasses
	/// must implement the read() and write() methods as well as calculateSize()
	struct DescriptorBase
	{
		DescriptorBase() = default;
		DescriptorBase(std::string key, std::vector<char> osKey) : m_Key(key), m_OSKey(osKey) {};

		virtual ~DescriptorBase() = default;

		/// Read the DescriptorItem from disk decoding it as well as populating any child nodes (if present)
		///
		/// \param document The file instance to read from, must point to a valid descriptor start
		virtual void read(File& document) = 0;
		/// Write the DescriptorItem to disk encoding it as well as any of its child nodes (if present)
		///
		/// \param document The file instance to write to, must be open for write access
		virtual void write(File& document) const = 0;

		/// Recursively convert the descriptor into a json object for easy visualization 
		/// of the data or writing to disk for debugging
		virtual json_ordered to_json() const = 0;

		/// Check the two descriptors for equality
		bool operator==(const DescriptorBase& other) const
		{
			if (this->os_key() != other.os_key())
			{
				return false;
			}
			if (this->key() != other.key())
			{
				return false;
			}
			return true;
		};

		/// Retrieve the key associated with the given descriptor item. This may be empty in the case of a list.
		/// In most cases retrieving this should not be necessary
		std::string key() const noexcept { return m_Key; }
		/// Retrieve the OSKey (type) of the descriptor item, since our OSType mapping is lossy this holds
		/// the original key and is intended to be used for identifying the OSType associated with the item
		std::vector<char> os_key() const noexcept { return m_OSKey; }

	protected:
		std::string m_Key{};
		std::vector<char> m_OSKey{};

		/// Get a json representation of the implementation, this includes things like 
		/// Class name, m_Key, m_OSKey
		/// 
		/// The json structure then looks something like this:
		/// {
		///		"_data_type": "Descriptor",
		///		"_key": m_Key,
		///		"_os_key": m_OSKey,
		/// }
		json_ordered get_json_repr(std::string data_type) const;
	};


	// Wrapper types for polymorphism to work properly, these just wrap the basic types and only have the necessary 
	// methods implemented to make them work
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

	struct double_Wrapper : public DescriptorBase
	{
		double m_Value{};

		double_Wrapper() = default;
		double_Wrapper(double value);
		double_Wrapper(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		double_Wrapper(std::string key, std::vector<char> osKey, double value) : DescriptorBase(key, osKey) { m_Value = value; };

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const double_Wrapper& other) const;

		json_ordered to_json() const override;
	};


	struct int32_t_Wrapper : public DescriptorBase
	{
		int32_t m_Value{};

		int32_t_Wrapper() = default;
		int32_t_Wrapper(int32_t value);
		int32_t_Wrapper(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		int32_t_Wrapper(std::string key, std::vector<char> osKey, int32_t value) : DescriptorBase(key, osKey) { m_Value = value; };

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const int32_t_Wrapper& other) const;

		json_ordered to_json() const override;
	};


	struct int64_t_Wrapper : public DescriptorBase
	{
		int64_t m_Value{};

		int64_t_Wrapper() = default;
		int64_t_Wrapper(int64_t value);
		int64_t_Wrapper(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		int64_t_Wrapper(std::string key, std::vector<char> osKey, int64_t value) : DescriptorBase(key, osKey) { m_Value = value; };

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const int64_t_Wrapper& other) const;

		json_ordered to_json() const override;
	};


	struct bool_Wrapper : public DescriptorBase
	{
		bool m_Value{};

		bool_Wrapper() = default;
		bool_Wrapper(bool value);
		bool_Wrapper(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		bool_Wrapper(std::string key, std::vector<char> osKey, bool value) : DescriptorBase(key, osKey) { m_Value = value; };

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const bool_Wrapper& other) const;

		json_ordered to_json() const override;
	};

	struct UnicodeString_Wrapper : public DescriptorBase
	{
		UnicodeString m_Value{};

		UnicodeString_Wrapper() = default;
		UnicodeString_Wrapper(UnicodeString value);
		UnicodeString_Wrapper(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		UnicodeString_Wrapper(std::string key, std::vector<char> osKey, UnicodeString value) : DescriptorBase(key, osKey) { m_Value = value; };

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const UnicodeString_Wrapper& other) const;

		json_ordered to_json() const override;
	};

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	// End wrapper types

	namespace Impl
	{
		/// Read a 4-byte key and return it
		std::vector<char>  readKey(File& document);

		/// Write a 4-byte key, throws if the key's size is not 4
		void writeKey(File& document, std::vector<char> key);

		/// Read a length denoted key which starts with a 4-byte count and if the count is 0 it is simply 4 bytes
		/// otherwise the length denotes how many chars to read. Returns this value while incrementing the document
		std::string readLengthDenotedKey(File& document);

		/// Write a length denoted key which starts with a 4-byte count which is 0 if the length is exactly 4, otherwise
		/// it is the actual length of the key
		void writeLengthDenotedKey(File& document, const std::string& key);

		/// OSType is the internal nomenclature for a Type as we know it in cpp. So for example an "Integer" OSType
		/// would be an int32_t in cpp. These OSTypes are stored as a 4-byte key which can be found at the 
		/// start of each descriptor item
		enum class OSTypes
		{
			Descriptor,
			GlobalObject,
			ObjectArray, // Undocumented but basically the same as Descriptor
			List,
			Reference,
			Double,
			UnitFloat,
			UnitFloats,
			String,
			Enumerated,
			Integer,
			LargeInteger,
			Boolean,
			Class_1,
			Class_2,
			Class_3,
			Alias,
			RawData,
			Path,
			Property,
			EnumeratedReference,
			Offset,
			Identifier,
			Index,
			Name
		};

		/// A list of all the valid descriptor keys we know of, some of these are undocumented and 
		/// others are grouped for convenience as the way they are read is identical and we store
		/// the OSType on the struct anyways
		static const std::unordered_map<OSTypes, std::vector<char>> descriptorKeys
		{
			// OSType keys
			{ OSTypes::Descriptor,			{ 'O', 'b', 'j', 'c' }},	// Descriptor
			{ OSTypes::GlobalObject,		{ 'G', 'l', 'b', 'O' }},	// GlobalObject (same as Descriptor)
			{ OSTypes::ObjectArray,			{ 'O', 'b', 'A', 'r' }},	// ObjectArray
			{ OSTypes::List,				{ 'V', 'l', 'L', 's' }},	// List
			{ OSTypes::Reference,			{ 'o', 'b', 'j', ' ' }},	// Reference (same as List)
			{ OSTypes::Double,				{ 'd', 'o', 'u', 'b' }},	// Double
			{ OSTypes::UnitFloat,			{ 'U', 'n', 't', 'F' }},	// Unit Float
			{ OSTypes::UnitFloats,			{ 'U', 'n', 'F', 'l' }},	// Unit Floats
			{ OSTypes::String,				{ 'T', 'E', 'X', 'T' }},	// String
			{ OSTypes::Enumerated,			{ 'e', 'n', 'u', 'm' }},	// Enumerated
			{ OSTypes::Integer,				{ 'l', 'o', 'n', 'g' }},	// Integer (int32_t)
			{ OSTypes::LargeInteger,		{ 'c', 'o', 'm', 'p' }},	// Large integer (int64_t)
			{ OSTypes::Boolean,				{ 'b', 'o', 'o', 'l' }},	// Boolean
			{ OSTypes::Class_1,				{ 't', 'y', 'p', 'e' }},	// Class
			{ OSTypes::Class_2,				{ 'G', 'l', 'b', 'C' }},	// Class
			{ OSTypes::Class_3,				{ 'C', 'l', 's', 's' }},	// Class
			{ OSTypes::Alias,				{ 'a', 'l', 'i', 's' }},	// Alias
			{ OSTypes::RawData,				{ 't', 'd', 't', 'a' }},	// Raw data
			{ OSTypes::Path,				{ 'P', 't', 'h', ' ' }},	// Path (same as raw data)
			// OSType Reference Keys	
			{ OSTypes::Property,			{ 'p', 'r', 'o', 'p' }},	// Property
			{ OSTypes::EnumeratedReference, { 'E', 'n', 'm', 'r' }},	// Enumerated Reference
			{ OSTypes::Offset,				{ 'r', 'e', 'l', 'e' }},	// Offset
			{ OSTypes::Identifier,			{ 'I', 'd', 'n', 't' }},	// Identifier
			{ OSTypes::Index,				{ 'i', 'n', 'd', 'x' }},	// Index
			{ OSTypes::Name,				{ 'n', 'a', 'm', 'e' }}		// Name
		};


		/// Types a UnitFloat or UnitFloats struct may hold
		enum class UnitFloatType
		{
			Angle,		// '#Ang'
			Density,	// '#Rsl'
			Distance,	// '#Rlt'
			None,		// '#Nne'
			Percent,	// '#Prc'
			Pixel,		// '#Pxl'
			Points,		// '#Pnt'
			Milimeters,	// '#Mlm'
		};


		/// Mapping of all the known valid UnitFloat unit types to their respective string representation and vice versa
		static const bidirectional_unordered_map<std::string, UnitFloatType> UnitFloatTypeMap
		{
			{ "#Ang", UnitFloatType::Angle },
			{ "#Rsl", UnitFloatType::Density },
			{ "#Rlt", UnitFloatType::Distance },
			{ "#Nne", UnitFloatType::None },
			{ "#Prc", UnitFloatType::Percent },
			{ "#Pxl", UnitFloatType::Pixel },
			{ "#Pnt", UnitFloatType::Points },
			{ "#Mlm", UnitFloatType::Milimeters }
		};


		/// Get a specified OS Type from a 4-byte char array key, raises an error if the specified
		/// key does not exist in the descriptorKeys mapping
		OSTypes getOSTypeFromKey(std::vector<char> key);


		template <typename T>
			requires std::is_base_of_v<DescriptorBase, T>
		std::tuple<std::string, std::unique_ptr<DescriptorBase>> construct_descriptor(File& document, std::string key, std::vector<char> ostype)
		{
			auto descriptor = std::make_unique<T>(key, ostype);
			descriptor->read(document);
			return std::make_tuple(key, std::move(descriptor));
		}

		/// Read a descriptor variant from the given File document and return it. It will handle any nested calls so this can be done
		/// once per item and if any nested levels are encountered a Descriptor is returned with its child nodes filled.
		/// 
		/// \param document The File object to read from, must start at a valid descriptor key and will read the full structure.
		/// \param readKey Whether to read the "key" field as well as the OSType field when parsing the data, for a List item this e.g. needs
		///				   to be set to false while for a descriptor this should be true, defaults to initializing the key with ""
		/// 
		/// \returns the key of the data as well as one of the filled Descriptor types which can be extracted with std::get<type>. 
		std::tuple<std::string, std::unique_ptr<DescriptorBase>> ReadDescriptorVariant(File& document, bool readKey = true);


		/// Write a given key-value pair to disk, use this function to write a generic DescriptorVariant to disk without knowing
		/// the exact type and without having to deal with the parsing of basic types (such as double, int64_t etc.). If present it
		/// will call the objects write() function. This function is intended to only be used in the high-level Descriptor struct
		/// as well as any DescriptorBase inherited items that store a DescriptorVariant type
		/// 
		/// \param document The File object to write to.
		/// \param key		The key to write to disk, will be ignored if writeKey is set to false
		/// \param value	The value you intend to write to disk, will write the child items as well if present
		/// \param readKey	Whether to write the key to disk or discard it (such as with lists), this does not propagate to any child 
		///					nodes and only applies to the given node
		inline void WriteDescriptor(File& document, const std::string& key, const std::unique_ptr<DescriptorBase>& value, bool writeKey = true)
		{
			if (writeKey)
			{
				Impl::writeLengthDenotedKey(document, key);
			}
			WriteBinaryArray(document, value->os_key());
			value->write(document);
		}
	}


	/// A mixin for any classes that need key-value like access to descriptor items. These items are insertion-ordered
	/// but no other guarantees for ordering are done. Items may not occur more than once
	struct KeyValueMixin
	{

		KeyValueMixin() = default;

		// Delete copy constructor and copy assignment operator
		KeyValueMixin(const KeyValueMixin&) = delete;
		KeyValueMixin& operator=(const KeyValueMixin&) = delete;

		// Enable move constructor and move assignment operator
		KeyValueMixin(KeyValueMixin&&) noexcept = default;
		KeyValueMixin& operator=(KeyValueMixin&& other) noexcept;


		/// Access one of the sub-elements, if the key doesnt exist we create a new Descriptor at the given key
		///
		/// \param key The key to search for
		/// \returns The Descriptor ptr at the given key or a default constructred ItemVariant if the key doesnt exist
		std::unique_ptr<DescriptorBase>& operator[] (const std::string_view key) noexcept;

		/// Access one of the sub-elements performing bounds checking and throwing if the specified 
		/// key does not exist
		/// 
		/// \param key The key to search for
		/// 
		/// \returns A reference to the ItemVariant at the given key
		std::unique_ptr<DescriptorBase>& at(const std::string_view key);
		const std::unique_ptr<DescriptorBase>& at(const std::string_view key) const;

		/// Access one of the sub-elements as the given type, performing bounds checking and 
		/// throwing if the specified key does not exist or if the template type T does not 
		/// match the item
		/// 
		/// \param key The key to search for
		/// \tparam T The type to retrieve it as
		/// 
		/// \returns A const ref to the item, no copy is performed
		template <typename T> 
			requires std::is_base_of_v<DescriptorBase, T>
		const T* at(const std::string key) const
		{
			for (auto& [stored_key, value] : m_DescriptorItems) 
			{
				if (stored_key == key) 
				{
					const T* casted = dynamic_cast<const T*>(value.get());
					if (casted)
					{
						return casted;
					}
					throw std::invalid_argument(fmt::format("Invalid type T while accessing key {}", key));
				}
			}
			throw std::out_of_range(fmt::format("Key {} not found in descriptor.", key));
		}

		/// Access one of the sub-elements as the given type, performing bounds checking and 
		/// throwing if the specified key does not exist or if the template type T does not 
		/// match the item. This is an overload to directly acces value of type `bool`, `int32_t`, 
		/// `int64_t`, `double` and `UnicodeString` directly without first getting their wrapper types.
		/// Copies the parameters, if you wish to modify them you must instead call the non-templated 
		/// at() function
		/// 
		/// \param key The key to search for
		/// \tparam T The type to retrieve it as
		/// 
		/// \returns A copy of the descriptor
		template <typename T>
			requires std::is_same_v<T, bool> || std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, double> || std::is_same_v<T, UnicodeString>
		T at(const std::string key) const
		{
			for (auto& [stored_key, value] : m_DescriptorItems)
			{
				if (stored_key == key)
				{
					if constexpr (std::is_same_v<T, bool>)
					{
						const bool_Wrapper* casted = dynamic_cast<const bool_Wrapper*>(value.get());
						if (casted)
						{
							return casted->m_Value;
						}
					}
					else if constexpr (std::is_same_v<T, int32_t>)
					{
						const int32_t_Wrapper* casted = dynamic_cast<const int32_t_Wrapper*>(value.get());
						if (casted)
						{
							return casted->m_Value;
						}
					}
					else if constexpr (std::is_same_v<T, int64_t>)
					{
						const int64_t_Wrapper* casted = dynamic_cast<const int64_t_Wrapper*>(value.get());
						if (casted)
						{
							return casted->m_Value;
						}
					}
					else if constexpr (std::is_same_v<T, double>)
					{
						const double_Wrapper* casted = dynamic_cast<const double_Wrapper*>(value.get());
						if (casted)
						{
							return casted->m_Value;
						}
					}
					else if constexpr (std::is_same_v<T, UnicodeString>)
					{
						const UnicodeString_Wrapper* casted = dynamic_cast<const UnicodeString_Wrapper*>(value.get());
						if (casted)
						{
							return casted->m_Value;
						}
					}
					throw std::invalid_argument(fmt::format("Invalid type T while accessing key {}", key));
				}
			}
			throw std::out_of_range(fmt::format("Key {} not found in descriptor.", key));
		}

		/// Insert the given key-value pair into the Descriptor. If the key is already present the new item is ignored
		void insert(std::pair<std::string, std::unique_ptr<DescriptorBase>> item) noexcept;
		/// Insert the given key-value pair into the Descriptor. If the key is already present the new item is ignored
		void insert(std::string key, std::unique_ptr<DescriptorBase> value) noexcept;
		/// Insert the given key-value pair into the Descriptor. If the key is already present the new item is ignored
		/// under the hood creates a wrapper around these types
		void insert(std::string key, std::variant<bool, int32_t, int64_t, double, UnicodeString> value);

		/// Insert the given key-value pair into the Descriptor overriding the value if the key is identical
		void insert_or_assign(std::pair<std::string, std::unique_ptr<DescriptorBase>> item) noexcept;
		/// Insert the given key-value pair into the Descriptor overriding the value if the key is identical
		void insert_or_assign(std::string key, std::unique_ptr<DescriptorBase> value) noexcept;
		/// Remove an item by its logical index, throws if the index is not valid
		void remove(size_t index);
		/// Remove an item by its key, throws if the key is not valid
		void remove(std::string_view key);

		/// Check if the Descriptor contains a specified key
		bool contains(std::string_view key) const noexcept;

		/// Return the number of elements stored in the Descriptor
		size_t size() const noexcept;

		/// Is the Descriptor empty?
		bool empty() const noexcept;

	protected:
		/// The storage of our key-value items
		std::vector<std::pair<std::string, std::unique_ptr<DescriptorBase>>> m_DescriptorItems;
	};



	struct Property : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		std::string m_KeyID;

		Property() = default;
		Property(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		Property(std::string key, std::vector<char> osKey, std::string name, std::string classID, std::string keyID);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Property& other) const;

		json_ordered to_json() const override;
	};


	struct Class : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;

		Class() = default;
		Class(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		Class(std::string key, std::vector<char> osKey, std::string name, std::string classID);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Class& other) const;

		json_ordered to_json() const override;
	};


	struct Enumerated : public DescriptorBase
	{
		std::string m_TypeID;
		std::string m_Enum;

		Enumerated() = default;
		Enumerated(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		Enumerated(std::string key, std::vector<char> osKey, std::string typeID, std::string enumerator);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Enumerated& other) const;

		json_ordered to_json() const override;

	};


	struct Index : public DescriptorBase
	{
		int32_t m_Identifier{};

		Index() = default;
		Index(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		Index(std::string key, std::vector<char> osKey, int32_t identifier);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Index& other) const;

		json_ordered to_json() const override;
	};


	struct EnumeratedReference : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		std::string m_TypeID;
		std::string m_Enum;

		EnumeratedReference() = default;
		EnumeratedReference(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		EnumeratedReference(
			std::string key, 
			std::vector<char> osKey, 
			std::string name, 
			std::string classID, 
			std::string typeID, 
			std::string enumerator
		);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const EnumeratedReference& other) const;

		json_ordered to_json() const override;

	};


	struct Offset : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		uint32_t m_Offset{};

		Offset() = default;
		Offset(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		Offset(std::string key, std::vector<char> osKey, std::string name, std::string classID, uint32_t offset);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Offset& other) const;

		json_ordered to_json() const override;
	};


	struct Identifier : public DescriptorBase
	{
		int32_t m_Identifier{};

		Identifier() = default;
		Identifier(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		Identifier(std::string key, std::vector<char> osKey, int32_t identifier);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Identifier& other) const;

		json_ordered to_json() const override;
	};


	struct UnitFloat : public DescriptorBase
	{

		/// The type of unit this UnitFloat stores
		Impl::UnitFloatType m_UnitType{};
		/// The value of the UnitFloat, please refer to m_UnitType for how to interpret this value
		double m_Value{};

		UnitFloat() = default;
		UnitFloat(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		UnitFloat(std::string key, std::vector<char> osKey, Impl::UnitFloatType type, double value);

		bool operator==(const UnitFloat& other) const;

		void read(File& document) override;
		void write(File& document) const override;

		json_ordered to_json() const override;
	};


	// Same as a UnitFloat but stores multiple values instead of a single one.
	struct UnitFloats : public DescriptorBase
	{
		Impl::UnitFloatType m_UnitType{};
		std::vector<double> m_Values{};

		UnitFloats() = default;
		UnitFloats(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		UnitFloats(std::string key, std::vector<char> osKey, Impl::UnitFloatType type, std::vector<double> values);

		bool operator==(const UnitFloats& other) const;

		void read(File& document) override;
		void write(File& document) const override;

		json_ordered to_json() const override;

	};


	/// Sort of like a descriptor structure but instead of storing a key-value pair it is stored as a flat
	/// list where each item just stores the OSType and the actual item
	struct List : public DescriptorBase
	{
		std::vector<std::unique_ptr<DescriptorBase>> m_Items;

		List() = default;
		List(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		List(std::string key, std::vector<char> osKey, std::vector<std::unique_ptr<DescriptorBase>> items);

		void read(File& document) override;
		void write(File& document) const override;

		/// Get the List items as a certain type, requires that all list items are the exact same. Returns 
		/// a copy of all the items so you may not modify them in-place
		template <typename T>
			requires std::is_base_of_v<DescriptorBase, T>
		std::vector<T> as() const
		{
			std::vector<T> out;
			out.reserve(m_Items.size());

			for (const auto& item : m_Items) 
			{
				const T* casted = dynamic_cast<const T*>(item.get());
				if (casted) 
				{
					out.push_back(*casted); // Dereference and copy the object
				}
				else 
				{
					throw std::runtime_error("Unable to access item as type T; it is not of that type");
				}
			}
			return out;
		}

		template <typename T>
			requires std::is_same_v<T, bool> || std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, double> || std::is_same_v<T, UnicodeString>
		std::vector<T> as() const
		{
			std::vector<T> out;
			out.reserve(m_Items.size());

			for (const auto& value : m_Items)
			{
				if constexpr (std::is_same_v<T, bool>)
				{
					const bool_Wrapper* casted = dynamic_cast<const bool_Wrapper*>(value.get());
					if (casted)
					{
						out.push_back(casted->m_Value);
					}
				}
				else if constexpr (std::is_same_v<T, int32_t>)
				{
					const int32_t_Wrapper* casted = dynamic_cast<const int32_t_Wrapper*>(value.get());
					if (casted)
					{
						out.push_back(casted->m_Value);
					}
				}
				else if constexpr (std::is_same_v<T, int64_t>)
				{
					const int64_t_Wrapper* casted = dynamic_cast<const int64_t_Wrapper*>(value.get());
					if (casted)
					{
						out.push_back(casted->m_Value);
					}
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					const double_Wrapper* casted = dynamic_cast<const double_Wrapper*>(value.get());
					if (casted)
					{
						out.push_back(casted->m_Value);
					}
				}
				else if constexpr (std::is_same_v<T, UnicodeString>)
				{
					const UnicodeString_Wrapper* casted = dynamic_cast<const UnicodeString_Wrapper*>(value.get());
					if (casted)
					{
						out.push_back(casted->m_Value);
					}
				}
				else
				{
					throw std::runtime_error("Unable to access item as type T; it is not of that type");
				}
			}
			return out;
		}

		bool operator==(const List& other) const;

		json_ordered to_json() const override;

	};


	/// Exactly the same as a List
	struct Reference : public List
	{
		using List::List;
	};


	struct RawData : public DescriptorBase
	{
		std::vector<uint8_t> m_Data{};

		RawData() = default;
		RawData(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		RawData(std::string key, std::vector<char> osKey, std::vector<uint8_t> data);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const RawData& other) const;

		json_ordered to_json() const override;
	};


	struct Path : public RawData
	{
		using RawData::RawData;
	};


	struct Name : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		UnicodeString m_Value;

		Name() = default;
		Name(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		Name(std::string key, std::vector<char> osKey, std::string name, std::string classID, std::string value);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Name& other) const;

		json_ordered to_json() const override;
	};


	/// The object array struct is very similar to the descriptor struct
	struct ObjectArray : public DescriptorBase, public KeyValueMixin
	{
		/// The number of items in the descriptor types this object holds.
		/// Not the amount of descriptors we hold, I am assuming these are always
		/// the same size
		uint32_t m_ItemsCount{};	
		UnicodeString m_Name{ "", 2u };
		std::string m_ClassID{};

		ObjectArray() = default;
		ObjectArray(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
		ObjectArray(
			std::string key, 
			std::vector<char> osKey, 
			uint32_t itemsCount, 
			std::string name, 
			std::string classID,
			std::vector<std::pair<std::string, std::unique_ptr<DescriptorBase>>> items);

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const ObjectArray& other) const;

		json_ordered to_json() const override;
	};


	/// Descriptor structures are Photoshops' native json/xml-like representation of key-value pairs 
	/// which can be nested to any level. The items are stored as a std::variant which can represent any of the different types.
	/// Accessing e.g. a unit float struct can look something like this:
	/// ~~~{.cpp}
	/// Descriptor descriptor{};	// We assume this is filled out
	/// auto unitFloatItem = std::get<Descriptors::UnitFloat>(descriptor["key"]);
	/// 
	/// ~~~
	/// 
	/// Similary you can get nested items using the same syntax:
	/// 
	/// ~~~{.cpp}
	/// Descriptor descriptor{};	// We assume this is filled out
	/// auto nestedDescriptor = std::get<Descriptor>(descriptor["key"]);
	/// auto nestedUnitFloat = std::get<UnitFloat>(nestedDescriptor["key"]);
	/// ~~~
	/// 
	/// Please check out the individual types as well for references on how to access their data
	/// and inspect the parsed output of the Descriptor parsing to get a feeling for how the structure
	/// of something you wish to parse is built up
	struct Descriptor : public DescriptorBase, public KeyValueMixin
	{
		UnicodeString m_Name{ "", 2u };

		Descriptor() { m_OSKey = Impl::descriptorKeys.at(Impl::OSTypes::Descriptor); };
		Descriptor(std::string key) : DescriptorBase(key, Impl::descriptorKeys.at(Impl::OSTypes::Descriptor)) {};
		Descriptor(std::string key, std::vector<char> ostype) : DescriptorBase(key, ostype) {};
		Descriptor(std::string key, std::vector<std::pair<std::string, std::unique_ptr<DescriptorBase>>> items);

		// Delete copy constructor and copy assignment operator
		Descriptor(const Descriptor&) = delete;
		Descriptor& operator=(const Descriptor&) = delete;

		// Enable move constructor and move assignment operator
		Descriptor(Descriptor&&) noexcept = default;
		Descriptor& operator=(Descriptor&&) noexcept = default;

		void read(File& document) override;
		void write(File& document) const override;

		bool operator==(const Descriptor& other) const;

		json_ordered to_json() const override;
	};


	struct GlobalObject : public Descriptor
	{
		using Descriptor::Descriptor;
	};



} // Namespace Descriptors



PSAPI_NAMESPACE_END