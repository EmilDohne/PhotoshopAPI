#include "TypeToolTaggedBlock.h"

#include "Core/Endian/EndianByteSwap.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Core/FileIO/LengthMarkers.h"
#include "Core/Struct/ByteStream.h"
#include "Util/Logger.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace
{
template <typename T>
void append_be(std::vector<std::byte>& data, const T value)
{
	const T encoded = endian_encode_be<T>(value);
	const auto* ptr = reinterpret_cast<const std::byte*>(&encoded);
	data.insert(data.end(), ptr, ptr + sizeof(T));
}

std::vector<std::byte> serialize_descriptor_body(const Descriptors::Descriptor& descriptor)
{
	std::vector<uint8_t> buffer{};
	buffer.reserve(32768u);
	File memory_file(std::move(buffer));
	descriptor.write(memory_file);

	std::vector<uint8_t> bytes(static_cast<size_t>(memory_file.getSize()), 0u);
	memory_file.setOffset(0u);
	memory_file.read(bytes);

	std::vector<std::byte> out(bytes.size());
	for (size_t i = 0u; i < bytes.size(); ++i)
	{
		out[i] = static_cast<std::byte>(bytes[i]);
	}
	return out;
}
}

TypeToolTaggedBlock::TypeToolTaggedBlock()
{
	m_Key = Enum::TaggedBlockKey::lrTypeTool;
}

void TypeToolTaggedBlock::read(
	File& document,
	const FileHeader& header,
	const uint64_t offset,
	const Signature signature,
	const uint16_t padding)
{
	TaggedBlock::read(document, header, offset, signature, Enum::TaggedBlockKey::lrTypeTool, padding);
	if (!parse_descriptors_from_data())
	{
		PSAPI_LOG_WARNING("TypeToolTaggedBlock", "Failed to parse TySh descriptors; keeping raw data only");
	}
}

void TypeToolTaggedBlock::write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding)
{
	if (m_DescriptorsParsed)
	{
		sync_data_from_descriptors();
	}
	TaggedBlock::write(document, header, callback, padding);
}

void TypeToolTaggedBlock::sync_data_from_descriptors()
{
	if (!m_DescriptorsParsed)
	{
		return;
	}

	std::vector<std::byte> data{};
	data.reserve(128u);

	append_be<uint16_t>(data, m_TyShVersion);
	for (const auto component : m_TransformationMatrix)
	{
		append_be<double>(data, component);
	}

	append_be<uint16_t>(data, m_TextVersion);
	append_be<uint32_t>(data, m_TextDescriptorVersion);
	{
		const auto text_body = serialize_descriptor_body(m_TextData);
		data.insert(data.end(), text_body.begin(), text_body.end());
	}

	append_be<uint16_t>(data, m_WarpVersion);
	append_be<uint32_t>(data, m_WarpDescriptorVersion);
	{
		const auto warp_body = serialize_descriptor_body(m_WarpData);
		data.insert(data.end(), warp_body.begin(), warp_body.end());
	}
	data.insert(data.end(), m_WarpTrailingData.begin(), m_WarpTrailingData.end());

	m_Data = std::move(data);
}

bool TypeToolTaggedBlock::parse_descriptors_from_data()
{
	m_DescriptorsParsed = false;
	if (m_Data.size() < (2u + 6u * 8u + 2u + 4u + 2u + 4u))
	{
		return false;
	}

	std::vector<uint8_t> bytes(m_Data.size(), 0u);
	for (size_t i = 0u; i < m_Data.size(); ++i)
	{
		bytes[i] = std::to_integer<uint8_t>(m_Data[i]);
	}
	ByteStream stream(bytes);

	m_TyShVersion = ReadBinaryData<uint16_t>(stream);
	for (size_t i = 0u; i < m_TransformationMatrix.size(); ++i)
	{
		m_TransformationMatrix[i] = ReadBinaryData<double>(stream);
	}

	m_TextVersion = ReadBinaryData<uint16_t>(stream);
	m_TextDescriptorVersion = ReadBinaryData<uint32_t>(stream);
	if (stream.getOffset() > bytes.size())
	{
		return false;
	}

	const size_t text_body_offset = static_cast<size_t>(stream.getOffset());
	std::vector<uint8_t> text_tail(
		bytes.begin() + static_cast<std::ptrdiff_t>(text_body_offset),
		bytes.end());
	File text_file(std::move(text_tail));
	Descriptors::Descriptor text_descriptor{};
	try
	{
		text_descriptor.read(text_file);
	}
	catch (...)
	{
		PSAPI_LOG_WARNING("TypeToolTaggedBlock", "Failed to parse TySh text descriptor body");
		return false;
	}
	const size_t text_body_size = static_cast<size_t>(text_file.getOffset());
	size_t cursor = text_body_offset + text_body_size;
	if (cursor + 6u > bytes.size())
	{
		return false;
	}

	stream.setOffset(static_cast<uint64_t>(cursor));
	m_WarpVersion = ReadBinaryData<uint16_t>(stream);
	m_WarpDescriptorVersion = ReadBinaryData<uint32_t>(stream);
	if (stream.getOffset() > bytes.size())
	{
		return false;
	}

	const size_t warp_body_offset = static_cast<size_t>(stream.getOffset());
	std::vector<uint8_t> warp_tail(
		bytes.begin() + static_cast<std::ptrdiff_t>(warp_body_offset),
		bytes.end());
	File warp_file(std::move(warp_tail));
	Descriptors::Descriptor warp_descriptor{};
	try
	{
		warp_descriptor.read(warp_file);
	}
	catch (...)
	{
		PSAPI_LOG_WARNING("TypeToolTaggedBlock", "Failed to parse TySh warp descriptor body");
		return false;
	}
	const size_t warp_body_size = static_cast<size_t>(warp_file.getOffset());
	cursor = warp_body_offset + warp_body_size;
	if (cursor > bytes.size())
	{
		return false;
	}

	m_WarpTrailingData.clear();
	m_WarpTrailingData.reserve(bytes.size() - cursor);
	for (size_t i = cursor; i < bytes.size(); ++i)
	{
		m_WarpTrailingData.push_back(static_cast<std::byte>(bytes[i]));
	}

	m_TextData = std::move(text_descriptor);
	m_WarpData = std::move(warp_descriptor);
	m_DescriptorsParsed = true;
	return true;
}

PSAPI_NAMESPACE_END
