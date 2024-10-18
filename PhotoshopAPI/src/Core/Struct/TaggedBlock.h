#pragma once

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Core/Struct/UnicodeString.h"

#include <memory>
#include <variant>

PSAPI_NAMESPACE_BEGIN



// Generic Tagged Block which does not hold any data. If you wish to parse further tagged blocks extend this struct and add an implementation
struct TaggedBlock
{
	Signature m_Signature;
	uint64_t m_Offset = 0u;	// Demarkates the start of the taggedblock, not the start of the data
	std::variant<uint32_t, uint64_t> m_Length;

	// Get the total size in a bounds checked manner
	template <typename T = size_t>
	T totalSize() const
	{
		static_assert(std::is_integral_v<T>, "Return must be of integral type");
		// Since m_Size is unsigned we only need to check against max
		if (m_TotalLength > std::numeric_limits<T>::max())
		{
			PSAPI_LOG_ERROR("TaggedBlock", "Unable to access tagged block size with template argument T as it would overflow it");
			return {};
		}
		return static_cast<T>(m_TotalLength);
	}

	void totalSize(size_t value) { m_TotalLength = value; }

	void addTotalSize(size_t increment) { m_TotalLength += increment; }

	Enum::TaggedBlockKey getKey() const noexcept{ return m_Key; };

	virtual ~TaggedBlock() = default;
	TaggedBlock() = default;

	// Read a TaggedBlock from a file
	void read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding = 1u);
	virtual void write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding = 1u);
protected:
	Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Unknown;

private:
	// The length of the tagged block with all the the signature, key and length marker
	// use this value to determine how long the total structure is
	size_t m_TotalLength = 0u;
};


// This tagged block demarkates the start or end of a layer section (group). It may additionally store the Passthrough blend mode
struct LrSectionTaggedBlock : TaggedBlock
{
	Enum::SectionDivider m_Type = Enum::SectionDivider::Any;

	// This is a bit weird, but if the blend mode for the layer is Passthrough, it stores BlendMode::Normal
	// on the layer itself and includes the blend mode over here. This is only present if the length is >= 12u
	std::optional<Enum::BlendMode> m_BlendMode;

	LrSectionTaggedBlock() = default;
	LrSectionTaggedBlock(Enum::SectionDivider sectionDivider, std::optional<Enum::BlendMode> blendMode) : 
		m_Type(sectionDivider), 
		m_BlendMode(blendMode) 
	{
		m_Key = Enum::TaggedBlockKey::lrSectionDivider;
		TaggedBlock::totalSize(4u);		// Signature
		TaggedBlock::addTotalSize(4u);	// Key
		TaggedBlock::addTotalSize(4u);	// Length marker
		TaggedBlock::addTotalSize(4u);	// LrSection type
		if (blendMode.has_value())
		{
			TaggedBlock::addTotalSize(4u);	// LrSection Signature
			TaggedBlock::addTotalSize(4u);	// LrSection Blendmode Key
		}
	};
	
	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


// 16-bit files store this tagged block at the end of the layer and mask information section which contains the 
// layer info section
struct Lr16TaggedBlock : TaggedBlock
{
	LayerInfo m_Data;

	Lr16TaggedBlock() = default;
	Lr16TaggedBlock(LayerInfo& lrInfo) : m_Data(std::move(lrInfo))
	{
		// We cant actually calculate the size of the tagged block here as that would require the channels to be compressed first
		TaggedBlock::totalSize(0u);
	};
	
	void read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};



// 32-bit files store this tagged block at the end of the layer and mask information section which contains the 
// layer info section
struct Lr32TaggedBlock : TaggedBlock
{
	LayerInfo m_Data;

	Lr32TaggedBlock() = default;
	Lr32TaggedBlock(LayerInfo& lrInfo) : m_Data(std::move(lrInfo)) 
	{
		// We cant actually calculate the size of the tagged block here as that would require the channels to be compressed first
		TaggedBlock::totalSize(0u);
	};
	
	void read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


/// This Tagged block appears to store information about how the layer is tranformed (flipped rotated etc.) The "Reference Point" as it is called stores the absolute world 
/// location of what the top left pixel would be. That means if we have a layer with an imaginary extent of [16, 16, 48, 48] (the scene size does not matter).
/// a reference point of (48.0f, 16.0f) would tell us the top left of the image is actually currently at the top right extents which would relate to a horizontal flip.
/// conversely a reference point of (16.0f, 48.0f) would relate to a vertical flip. A flip on both axis would be (48.0f, 48.0f).
/// 
/// Unfortunately it is currently unclear how photoshop distinguishes between rotations and flips as a 90 degree turn clockwise (which does not look the same as a horizontal
/// flip) relates to the same reference point as a horizontal flip (48.0f, 16.0f).
/// 
/// For 90 degree turns for example the Reference point coordinates are the same as a horizontal flip (48.0f, 16.0f) but the actual image data is rotated
/// 
/// Due to this uncertain behaviour this block is only for roundtripping for the time being
struct ReferencePointTaggedBlock : TaggedBlock
{
	/// The absolute X Coordinate reference point for transforms, this must be within the bounding box
	/// of the layer (or less than .5 pixels away as bbox is stored in float while 
	double m_ReferenceX = 0.0f;
	// The absolute Y Coordinate reference point for transforms
	double m_ReferenceY = 0.0f;

	ReferencePointTaggedBlock() = default;
	ReferencePointTaggedBlock(double refX, double refY) : m_ReferenceX(refX), m_ReferenceY(refY) 
	{
		// This is the size of 2 doubles + 4 bytes for the signature, 4 bytes for the key and 4 bytes for the length
		TaggedBlock::totalSize(16u + 4u + 4u + 4u);
	};

	void read(File& document, const uint64_t offset, const Signature signature);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


/// The layer name stored in UTF16 BE order on disk, this is the preferred way of retrieving the layer name as it is 
/// not limited to 255 chars like the layers PascalString but instead can hold up to 2^32 code points or 2^33 bytes
/// This tagged block is not required but Photoshop usually will always write this out in more modern versions 
struct UnicodeLayerNameTaggedBlock : TaggedBlock
{
	UnicodeString m_Name = {};

	UnicodeLayerNameTaggedBlock() = default;
	UnicodeLayerNameTaggedBlock(std::string name, const uint8_t padding = 1u)
	{
		m_Name = UnicodeString(name, padding);
		TaggedBlock::totalSize(12u + m_Name.calculateSize());
	}

	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


/// The layers' pixel protection settings. This is internally a uint32_t of which only the first byte seems to hold relevant 
/// information with the rest being for padding and/or aligment. 
struct ProtectedSettingTaggedBlock : TaggedBlock
{
	bool m_IsLocked = false;	// 01000000 of the first byte

	ProtectedSettingTaggedBlock() = default;
	ProtectedSettingTaggedBlock(bool isLocked) : m_IsLocked(isLocked) 
	{ 
		// This is the size of a uin32_t + 4 bytes for the signature, 4 bytes for the key and 4 bytes for the length
		TaggedBlock::totalSize(4u + 4u + 4u + 4u);
	};

	void read(File& document, const uint64_t offset, const Signature signature);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};

PSAPI_NAMESPACE_END