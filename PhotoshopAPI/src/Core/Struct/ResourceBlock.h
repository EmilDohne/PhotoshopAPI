#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Core/Struct/PascalString.h"
#include "Core/Struct/Section.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Core/Struct/PhotoshopTypes.h"

#include <vector>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN

/// A generic ResourceBlock basetype for all unimplemented tags. It does not store any data and any struct
/// implementing this basetyp must store the data
struct ResourceBlock : public FileSection
{
	Enum::ImageResource m_UniqueId;
	PascalString m_Name;
	/// Size of the data, padded to 2-bytes
	uint32_t m_DataSize = 0;

	ResourceBlock() : m_UniqueId(Enum::ImageResource::NotImplemented), m_Name("", 2u), m_DataSize(0) {m_Size = this->calculateSize(); };
	
	// This calculates the size of m_Size only, not m_DataSize!
	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	/// Force an override for the write function as we do not store
	/// ResourceBlocks when roundtripping so we only have what we want here
	virtual void write(File& document) = 0;

	virtual ~ResourceBlock() = default;
};


/// This ResourceBlock holds information about the document DPI
struct ResolutionInfoBlock : public ResourceBlock
{
	/// This value is always stored internally as PixelsPerInch even if the ResolutionUnit is set to PixelsPerInch
	/// so when writing it we must actually convert it by multiplying by 2.54
	FixedFloat4 m_HorizontalRes = { 72.0f };
	Enum::ResolutionUnit m_HorizontalResUnit = Enum::ResolutionUnit::PixelsPerInch;
	Enum::DisplayUnit m_WidthUnit = Enum::DisplayUnit::Cm;

	// These values are not exposed through the Photoshop UI and also seem to be identical to the 
	// horizontal resolutions. It could be that there are some non-square pixel setups which would
	// make these different but we do not care about that for the time being so these just mirror
	// the horizontal res
	FixedFloat4 m_VerticalRes = { 72.0f };
	Enum::ResolutionUnit m_VerticalResUnit = Enum::ResolutionUnit::PixelsPerInch;
	Enum::DisplayUnit m_HeightUnit = Enum::DisplayUnit::Cm;

	// We dont overwrite calculateSize here since we read m_DataSize which gives us all the info to know the size

	ResolutionInfoBlock();
	ResolutionInfoBlock(float resolution, Enum::ResolutionUnit resolutionUnit = Enum::ResolutionUnit::PixelsPerInch, Enum::DisplayUnit displayUnit = Enum::DisplayUnit::Cm);

	void read(File& document, const uint64_t offset);
	void write(File& document) override;
};


/// This ResourceBlock holds the ICC Profile associated with the document. This is equivalent to Photoshops
/// Edit -> Assign Profile which just visually adjusts the colours but does not convert the colours
struct ICCProfileBlock : public ResourceBlock
{
	/// Stores the raw bytes of an ICC profile such as the ones found on C:\Windows\System32\spool\drivers\color for Windows.
	/// This does not include the padding bytes which are explicitly written on write
	std::vector<uint8_t> m_RawICCProfile;

	// We dont overwrite calculateSize here since we read m_DataSize which gives us all the info to know the size

	ICCProfileBlock() = default;
	ICCProfileBlock(std::vector<uint8_t>&& iccProfile);
	 
	void read(File& document, const uint64_t offset);
	void write(File& document) override;
};

PSAPI_NAMESPACE_END