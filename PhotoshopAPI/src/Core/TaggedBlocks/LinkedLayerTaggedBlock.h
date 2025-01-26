#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/BidirectionalMap.h"
#include "Core/Struct/Signature.h"
#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/UnicodeString.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"


PSAPI_NAMESPACE_BEGIN


namespace LinkedLayerItem
{
	/// Date structure for a linked layer
	struct Date : public FileSection
	{
		uint32_t year{};
		uint8_t month{};
		uint8_t day{};
		uint8_t hour{};
		uint8_t minute{};
		float64_t seconds{};

		/// Default initialize this date struct to the current day and time
		Date();

		void read(File& document);
		void write(File& document) const;
	};

	enum class Type
	{
		Data,		// Stored on the file
		External,	// Linked externally
		Alias		// Aliased, means the data is zero but not sure how to parse exactly.
	};

	/// Data representation of a single LinkedLayer record, there may be multiple of these per LinkedLayerTaggedBlock
	/// Photoshop knows of multiple versions of these which may or may not contain certain information. When writing 
	/// these out we only care about version 7 
	struct Data
	{
		Type m_Type = Type::Data;		// How the data is (or isnt) stored in the file
		int32_t m_Version = 7u;			// 1-7. In our case should always be 7 for write
		std::string m_UniqueID;			// Mirrors the UniqueID on a PlacedLayerTaggedBlock, this must be referenced somewhere
		UnicodeString m_FileName;		// The actual filename itself, this does not necessarily represent a path to an actual file
		std::string m_FileType;			// E.g. "png " for png files etc.
		uint32_t m_FileCreator = 0;		// Unknown what this is, seems to just be filled with 0 across all 4 bytes

		std::unique_ptr<Descriptors::Descriptor> m_FileOpenDescriptor = nullptr;
		std::unique_ptr<Descriptors::Descriptor> m_LinkedFileDescriptor = nullptr;

		std::optional<Date> m_Date;

		std::vector<uint8_t> m_RawFileBytes;	// May be empty, this only appears on an External/Data LinkedLayer

		// Only available in version 5, 6 and 7 of the descriptor respectively
		std::optional<UnicodeString> m_ChildDocumentID;
		std::optional<float64_t> m_AssetModTime;
		std::optional<bool> m_AssetIsLocked;

		Data() = default;
		Data(std::string unique_id, std::filesystem::path filepath, Type type, std::vector<uint8_t> bytes, std::filesystem::path photoshop_file_path);

		void read(File& document);
		/// Write the LinkedLayerData struct. Unlike the other write methods this is non-const since otherwise we would have to copy
		/// on write the raw file data. 
		void write(File& document);

	private:
		uint64_t m_Size = 0u;

		Type readType(File& document) const;
		void writeType(File& document, Type type) const;

		/// Generate the filetype component for smart object layer. this is e.g.
		/// 'JPEG' or 'png '
		static std::string generate_file_type(const std::filesystem::path& filepath);
	};
}


/// LinkedLayers are how Photoshop stores smart objects, these are stored on the Global Tagged blocks and store the information
/// related to a smart object such as the FilePath, data size, file information etc.
/// It additionally stores a unique ID for each of the layers which get mirrored in the PlacedLayer TaggedBlock such that on layer parsing
/// we can map the layer specific PlacedLayerTaggedBlock -> LinkedLayerTaggedBlock.
/// 
/// Photoshop has 3 different ways of storing SmartObject data, either as Linked into the file, Linked to an external file or as an Alias (unknown)
struct LinkedLayerTaggedBlock : TaggedBlock
{
	std::string m_LinkKey = "lnk2";
	std::vector<LinkedLayerItem::Data> m_LayerData;	// A single LinkedLayer block may have multiple file descriptions stored in it

	void read(File& document, const FileHeader& header, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


PSAPI_NAMESPACE_END