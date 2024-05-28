#pragma once

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"

#include <memory>
#include <variant>

PSAPI_NAMESPACE_BEGIN



// Generic Tagged Block which does not hold any data. If you wish to parse further tagged blocks extend this struct and add an implementation
struct TaggedBlock
{
	Signature m_Signature;
	uint64_t m_Offset = 0u;	// Demarkates the start of the taggedblock, not the start of the data
	std::variant<uint32_t, uint64_t> m_Length;

	uint64_t getTotalSize() const noexcept{ return m_TotalLength; };
	Enum::TaggedBlockKey getKey() const noexcept{ return m_Key; };

	virtual ~TaggedBlock() = default;
	TaggedBlock() = default;

	// Read a TaggedBlock from a file
	void read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding = 1u);
	virtual void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u);
protected:
	Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Unknown;
	// The length of the tagged block with all the the signature, key and length marker
	// use this value to determine how long the total structure is
	uint64_t m_TotalLength = 0u;
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
		m_TotalLength = 4u;		// Signature
		m_TotalLength += 4u;	// Key
		m_TotalLength += 4u;	// Length marker
		m_TotalLength += 4u;	// LrSection type
		if (blendMode.has_value())
		{
			m_TotalLength += 4u;	// LrSection Signature
			m_TotalLength += 4u;	// LrSection Blendmode Key
		}
	};
	
	void read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


// 16-bit files store this tagged block at the end of the layer and mask information section which contains the 
// layer info section
struct Lr16TaggedBlock : TaggedBlock
{
	LayerInfo m_Data;

	Lr16TaggedBlock() = default;
	Lr16TaggedBlock(LayerInfo& lrInfo, const FileHeader& header) : m_Data(std::move(lrInfo))
	{
		// We cant actually calculate the size of the tagged block here as that would require the channels to be compressed first
		m_TotalLength = 0u;
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
	Lr32TaggedBlock(LayerInfo& lrInfo, const FileHeader& header) : m_Data(std::move(lrInfo)) 
	{
		// We cant actually calculate the size of the tagged block here as that would require the channels to be compressed first
		m_TotalLength = 0u;
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
		m_TotalLength = 16u + 4u + 4u + 4u;
	};

	void read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};

PSAPI_NAMESPACE_END