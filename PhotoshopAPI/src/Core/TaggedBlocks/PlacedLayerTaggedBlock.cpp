#include "PlacedLayerTaggedBlock.h"


PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayer::Point::read(File& document)
{
	this->x = ReadBinaryData<double>(document);
	this->y = ReadBinaryData<double>(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayer::Point::write(File& document)
{
	WriteBinaryData<double>(document, this->x);
	WriteBinaryData<double>(document, this->y);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayer::Transform::read(File& document)
{
	this->topleft.read(document);
	this->topright.read(document);
	this->bottomright.read(document);
	this->bottomleft.read(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayer::Transform::write(File& document)
{
	this->topleft.write(document);
	this->topright.write(document);
	this->bottomright.write(document);
	this->bottomleft.write(document);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerTaggedBlock::read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	m_Length = ReadBinaryData<uint32_t>(document);
	TaggedBlock::totalSize(static_cast<size_t>(std::get<uint32_t>(m_Length)) + 4u + 4u + 4u);

	// The type is always going to be 'plcL' according to the docs
	auto type = Signature::read(document);
	if (type != "plcL")
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown placed layer type '%s' encountered", type.string().c_str());
	}

	m_Version = ReadBinaryData<uint32_t>(document);
	if (m_Version != 3)
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown placed layer version %d encountered", m_Version);
	}

	m_UniqueID.read(document, 1u);

	m_PageNumber = ReadBinaryData<uint32_t>(document);
	m_TotalPages = ReadBinaryData<uint32_t>(document);
	m_AnitAliasPolicy = ReadBinaryData<uint32_t>(document);

	auto layerType = ReadBinaryData<uint32_t>(document);
	if (layerType > 3)
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown placed layer LayerType %d encountered", layerType);
	}
	m_Type = PlacedLayer::s_TypeMap.at(layerType);
	if (m_Type != PlacedLayer::Type::Raster)
	{
		PSAPI_LOG_WARNING("PlacedLayer", "Currently unimplemented LayerType '%s' encountered", PlacedLayer::s_TypeStrMap.at(layerType).c_str());
	}

	m_Transform.read(document);

	auto warpVersion = ReadBinaryData<uint32_t>(document);
	auto descriptorVersion = ReadBinaryData<uint32_t>(document);
	if (warpVersion != 0 || descriptorVersion != 16)
	{
		PSAPI_LOG_ERROR("PlacedLayer", "Unknown warp or descriptor version encountered. Warp version: %d. Descriptor Version: %d. Expected 0 and 16 for these respectively", warpVersion, descriptorVersion);
	}
	m_WarpInformation.read(document);

	// This section is padded so we simply skip to the end
	document.setOffset(offset + TaggedBlock::totalSize<uint64_t>());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerTaggedBlock::write(File& document, const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("PlLd").m_Value);

	auto lenOffset = document.getOffset();
	if (header.m_Version == Enum::Version::Psd)
	{
		WriteBinaryData<uint32_t>(document, TaggedBlock::totalSize<uint32_t>() - 12u);
	}
	else
	{
		WriteBinaryData<uint64_t>(document, TaggedBlock::totalSize<uint64_t>() - 12u);
	}

	WriteBinaryData<uint32_t>(document, Signature("plcL").m_Value);
	WriteBinaryData<uint32_t>(document, m_Version);
	m_UniqueID.write(document);

	WriteBinaryData<uint32_t>(document, m_PageNumber);
	WriteBinaryData<uint32_t>(document, m_TotalPages);
	WriteBinaryData<uint32_t>(document, m_AnitAliasPolicy);

	WriteBinaryData<uint32_t>(document, PlacedLayer::s_TypeMap.at(m_Type));

	m_Transform.write(document);

	WriteBinaryData<uint32_t>(document, 0u);
	WriteBinaryData<uint32_t>(document, 16u);
	m_WarpInformation.write(document);

	// Write out the length block as well as any padding. This essentially skips back to the point where we wrote the 
	// zero-sized length block and writes it back out but now with the actual section length
	if (header.m_Version == Enum::Version::Psd)
	{
		Impl::writeLengthBlock<uint32_t>(document, lenOffset, document.getOffset(), 4u);
	}
	else
	{
		Impl::writeLengthBlock<uint64_t>(document, lenOffset, document.getOffset(), 4u);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerDataTaggedBlock::read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	m_Length = ReadBinaryData<uint32_t>(document);
	TaggedBlock::totalSize(static_cast<size_t>(std::get<uint32_t>(m_Length)) + 4u + 4u + 4u);

	// The identifier is always going to be 'soLD' according to the docs
	auto identifier = Signature::read(document);
	if (identifier != "soLD")
	{
		PSAPI_LOG_ERROR("PlacedLayerData", "Unknown placed layer identifier '%s' encountered", identifier.string().c_str());
	}

	m_Version = ReadBinaryData<uint32_t>(document);
	auto descriptorVersion = ReadBinaryData<uint32_t>(document);
	if (m_Version != 4 || descriptorVersion != 16)
	{
		PSAPI_LOG_ERROR("PlacedLayerData", "Unknown version or descriptor version encountered. Version: %d. Descriptor Version: %d. Expected 4 and 16 for these respectively", m_Version, descriptorVersion);
	}

	m_Descriptor.read(document);
	// Manually skip to the end as this section may be padded
	document.setOffset(offset + TaggedBlock::totalSize<uint64_t>());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PlacedLayerDataTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("SoLd").m_Value);

	auto lenOffset = document.getOffset();
	WriteBinaryData<uint32_t>(document, 0u);

	// Write key, version and descriptor version
	Signature("soLD").write(document);
	WriteBinaryData<uint32_t>(document, m_Version);
	WriteBinaryData<uint32_t>(document, 16u);

	m_Descriptor.write(document);

	// Write out the length block as well as any padding. This essentially skips back to the point where we wrote the 
	// zero-sized length block and writes it back out but now with the actual section length
	Impl::writeLengthBlock<uint32_t>(document, lenOffset, document.getOffset(), padding);
}

PSAPI_NAMESPACE_END