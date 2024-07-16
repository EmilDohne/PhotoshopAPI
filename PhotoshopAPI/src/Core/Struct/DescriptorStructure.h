#pragma once

#include "Macros.h"

#include "UnicodeString.h"
#include "Section.h"
#include "BidirectionalMap.h"

#include <variant>
#include <vector>

PSAPI_NAMESPACE_BEGIN

// Forward declaration for use in some of the DescriptorItems
struct Descriptor;


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

	/// Scans to the next known descriptor key and returns the raw byte data it read until then as well as leaving
	/// the document in a state where the following bytes read would be the valid key. This is to be used if we 
	/// find an unknown key value as descriptor items do not by default store a data length value.
	/// 
	/// If no valid key is found within the maxScanDist we raise an error.
	/// 
	/// \param document The File object to read from, this will be modified in a way that the next bytes read will point to a valid descriptor item
	/// \param maxScanDist The maximum distance in bytes to scan and search for
	/// \return The raw bytes of the descriptor
	std::vector<char> scanToNextDescriptor(File& document, size_t maxScanDist = 1024);
}


namespace DescriptorItems
{
	enum class OSTypes
	{
		Reference,
		Descriptor,
		ObjectArray, // Undocumented but basically the same as Descriptor
		List,
		Double,
		UnitFloat,
		UnitFloats,
		String,
		Enumerated,
		Integer,
		LargeInteger,
		Boolean,
		Class,
		Alias,
		RawData,
		Property,
		EnumeratedReference,
		Offset,
		Identifier,
		Index,
		Name
	};

	// A list of all the valid descriptor keys 
	static const std::unordered_map<OSTypes, std::vector<std::vector<char>>> descriptorKeys
	{
		// OSType keys
		{ OSTypes::Descriptor,			{{ 'O', 'b', 'j', 'c' }, { 'G', 'l', 'b', 'O' }}},	// Descriptor & GlobalObject
		{ OSTypes::ObjectArray,			{{ 'O', 'b', 'A', 'r' }}},	// ObjectArray
		{ OSTypes::List,				{{ 'V', 'l', 'L', 's' }, { 'o', 'b', 'j', ' ' }}},	// List & Reference
		{ OSTypes::Double,				{{ 'd', 'o', 'u', 'b' }}},	// Double
		{ OSTypes::UnitFloat,			{{ 'U', 'n', 't', 'F' }}},	// Unit Float
		{ OSTypes::UnitFloats,			{{ 'U', 'n', 'F', 'l' }}},	// Unit Float
		{ OSTypes::String,				{{ 'T', 'E', 'X', 'T' }}},	// String
		{ OSTypes::Enumerated,			{{ 'e', 'n', 'u', 'm' }}},	// Enumerated
		{ OSTypes::Integer,				{{ 'l', 'o', 'n', 'g' }}},	// Integer (int32_t)
		{ OSTypes::LargeInteger,		{{ 'c', 'o', 'm', 'p' }}},	// Large integer (int64_t)
		{ OSTypes::Boolean,				{{ 'b', 'o', 'o', 'l' }}},	// Boolean
		{ OSTypes::Class,				{{ 't', 'y', 'p', 'e' }, { 'G', 'l', 'b', 'C' }, { 'C', 'l', 's', 's' }}},	// Class
		{ OSTypes::Alias,				{{ 'a', 'l', 'i', 's' }}},	// Alias
		{ OSTypes::RawData,				{{ 't', 'd', 't', 'a' }, { 'P', 't', 'h', ' '}}},	// Raw Data & Path (both are the same)
		// OSType Reference Keys	
		{ OSTypes::Property,			{{ 'p', 'r', 'o', 'p' }}}, // Property
		{ OSTypes::EnumeratedReference, {{ 'E', 'n', 'm', 'r' }}}, // Enumerated Reference
		{ OSTypes::Offset,				{{ 'r', 'e', 'l', 'e' }}}, // Offset
		{ OSTypes::Identifier,			{{ 'I', 'd', 'n', 't' }}}, // Identifier
		{ OSTypes::Index,				{{ 'i', 'n', 'd', 'x' }}}, // Index
		{ OSTypes::Name,				{{ 'n', 'a', 'm', 'e' }}}  // Name
	};

	/// Get a specified OS Type from a 4-byte char array key, raises an error if the specified
	/// key does not exist in the descriptorKeys mapping
	OSTypes getOSTypeFromKey(std::vector<char> key);

	// Forward declare these items to use them in the ItemVariant type
	struct Property;
	struct Class;
	struct Enumerated;
	struct Index;		
	struct Name;
	struct EnumeratedReference;
	struct ObjectArray;
	struct Offset;
	struct RawData;
	struct Identifier;
	struct UnitFloat;
	struct UnitFloats;
	struct List;

	/// std::variant type which holds all the types that may be encountered within a Descriptor Structure
	using ItemVariant = std::variant<
		Descriptor,
		List,
		RawData,
		double,
		UnitFloat,
		UnitFloats,
		Enumerated,
		int32_t,	// Integer type
		int64_t,	// Large Integer type
		bool,
		UnicodeString,	// String type
		Class,
		Property,
		EnumeratedReference,
		Offset,
		Identifier,
		Index,
		Name,
		ObjectArray>;


	/// Read a descriptor variant from the given File document and return it. It will handle any nested calls so this can be done
	/// once per item and if any nested levels are encountered a Descriptor is returned with its child nodes filled.
	/// If we do not have a parser for a given item we will scan until we can find one of them to leave the following nodes in a valid
	/// state, if this doesn't happen within a scan threshold an error is raised.
	/// 
	/// \param document The File object to read from, must start at a valid descriptor key and will read the full structure.
	/// \param readKey Whether to read the "key" field as well as the OSType field when parsing the data, for a List item this e.g. needs
	///				   to be set to false while for a descriptor this should be true, defaults to initializing the key with ""
	/// 
	/// \returns the key of the data as well as one of the filled Descriptor types which can be extracted with std::get<type>. 
	std::tuple<std::string, ItemVariant> read(File& document, bool readKey = true);


	struct DescriptorBase
	{
		DescriptorBase() = default;
		DescriptorBase(std::string key, std::vector<char> osKey) : m_Key(key), m_OSKey(osKey) {};


		virtual void read(File& document) = 0;
		inline virtual void write(File& document) {};

	protected:
		std::string m_Key{};
		std::vector<char> m_OSKey{};
	};

	struct Property : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		std::string m_KeyID;

		void read(File& document) override;

		Property(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};
	};

	struct Class: public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;

		Class(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};


	struct Enumerated : public DescriptorBase
	{
		std::string m_TypeID;
		std::string m_Enum;

		Enumerated(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};


	struct Index : public DescriptorBase
	{
		int32_t m_Identifier{};

		Index(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
		void write(File& document) override;
	};

	struct EnumeratedReference : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		std::string m_TypeID;
		std::string m_Enum;	

		EnumeratedReference(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};

	struct Offset : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		uint32_t m_Offset{};

		Offset(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};


	struct Identifier : public DescriptorBase
	{
		int32_t m_Identifier{};

		Identifier(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		/// \brief Read a Identifier structure within a Descriptor from a file stream parsing it to memory
		/// \param document The File object to read from
		void read(File& document) override;
		/// \brief Write a Identifier structure to disk
		/// \param document The File object to write to 
		void write(File& document) override;
	};


	struct Reference : public DescriptorBase
	{
		using valueType = std::variant<Property, Class, EnumeratedReference, Offset, Identifier>;
		std::vector<valueType> m_Items;

		Reference(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};


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

	struct UnitFloat : public DescriptorBase
	{
		
		/// The type of unit this UnitFloat stores
		UnitFloatType m_UnitType{};
		/// The actual value of the UnitFloat
		double m_Value{};

		UnitFloat(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		/// \brief Read a UnitFloat structure within a Descriptor from a file stream parsing it to memory
		/// 
		/// Throws if we cannot identify a valid UnitFloat type from the structure
		/// 
		/// \param document The File object to read from
		void read(File& document) override;
		/// \brief Write a UnitFloat structure to disk
		/// \param document The File object to write to 
		void write(File& document) override;
	};

	// Same as a UnitFloat but stores multiple values instead of a single one.
	struct UnitFloats : public DescriptorBase
	{
		UnitFloatType m_UnitType{};
		std::vector<double> m_Values{};

		UnitFloats(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
		void write(File& document) override;
	};


	/// Sort of like a descriptor structure but instead of storing a key-value pair it is stored as a flat
	/// list where each item just stores the OSType and the actual item
	struct List : public DescriptorBase
	{
		std::vector<ItemVariant> m_Items;

		List(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};

	struct RawData : public DescriptorBase
	{
		std::vector<uint8_t> m_Data{};

		RawData(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
		void write(File& document) override;
	};

	struct Name : public DescriptorBase
	{
		UnicodeString m_Name;
		std::string m_ClassID;
		UnicodeString m_Value;

		Name(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};


	/// The object array struct is very similar to the descriptor struct
	struct ObjectArray : public DescriptorBase
	{
		uint32_t m_ItemsCount{};
		UnicodeString m_Name{};
		std::string m_Key{};
		std::vector<char> m_OSKey;
		std::unordered_map<std::string, DescriptorItems::ItemVariant> m_DescriptorItems;

		ObjectArray(std::string key, std::vector<char> osKey) : DescriptorBase(key, osKey) {};

		void read(File& document) override;
	};
}


/// Descriptor structures are Photoshops' native json/xml-like representation of key-value pairs 
/// which can be nested to any level
struct Descriptor : public FileSection
{
	UnicodeString m_Name{};
	std::string m_Key{};
	std::vector<char> m_OSKey;
	std::unordered_map<std::string, DescriptorItems::ItemVariant> m_DescriptorItems;

	Descriptor() = default;
	Descriptor(std::string key, std::vector<char> oskey) : m_Key(key), m_OSKey(oskey) {};

	/// Read the descriptor structure into all of its subcomponents 
	void read(File& document);

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override {
		return 0;
	};
};


PSAPI_NAMESPACE_END