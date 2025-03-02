#include "TypeToolTaggedBlock.h"

#include "Core/FileIO/LengthMarkers.h"

#include <iostream>

PSAPI_NAMESPACE_BEGIN


void TypeToolTaggedBlock::read(File& document, const uint64_t offset, const Signature signature, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	m_Key = Enum::TaggedBlockKey::lrTypeTool;
	m_Offset = offset;
	m_Signature = signature;

	m_Length = ReadBinaryData<uint32_t>(document);
	auto len_offset = document.get_offset();

	auto version = ReadBinaryData<uint16_t>(document);
	if (version != 1)
	{
		PSAPI_LOG_ERROR("TypeToolTaggedBlock", "Expected to get version 1, instead received %d", static_cast<int>(version));
	}

	// Read transformation
	{
		auto xx = ReadBinaryData<double>(document);
		auto xy = ReadBinaryData<double>(document);
		auto yx = ReadBinaryData<double>(document);
		auto yy = ReadBinaryData<double>(document);
		auto tx = ReadBinaryData<double>(document);
		auto ty = ReadBinaryData<double>(document);

		m_TransformationMatrix[0] = xx;
		m_TransformationMatrix[1] = xy;
		m_TransformationMatrix[2] = tx;
		m_TransformationMatrix[3] = yx;
		m_TransformationMatrix[4] = yy;
		m_TransformationMatrix[5] = ty;
		m_TransformationMatrix[6] = 0.0f;
		m_TransformationMatrix[7] = 0.0f;
		m_TransformationMatrix[7] = 1.0f;
	}

	// Text data section
	auto text_version = ReadBinaryData<uint16_t>(document);
	auto descriptor_version = ReadBinaryData<uint32_t>(document);
	if (text_version != 50)
	{
		PSAPI_LOG_ERROR("TypeToolTaggedBlock", "Expected to get text version 50, instead received %d", static_cast<int>(text_version));
	}
	if (descriptor_version != 16)
	{
		PSAPI_LOG_ERROR("TypeToolTaggedBlock", "Expected to get text descriptor version 16, instead received %d", static_cast<int>(descriptor_version));
	}
	m_TextData.read(document);

	// Warp information section
	auto warp_version = ReadBinaryData<uint16_t>(document);
	auto warp_descriptor_version = ReadBinaryData<uint32_t>(document);
	if (warp_version != 1)
	{
		PSAPI_LOG_ERROR("TypeToolTaggedBlock", "Expected to get warp version 1, instead received %d", static_cast<int>(warp_version));
	}
	if (warp_descriptor_version != 16)
	{
		PSAPI_LOG_ERROR("TypeToolTaggedBlock", "Expected to get warp descriptor version 16, instead received %d", static_cast<int>(warp_descriptor_version));
	}
	m_WarpData.read(document);

	// TODO : Remove tmp
	std::cout << std::setw(4) << m_TextData.to_json() << std::endl;
	std::cout << "\n" << std::endl;
	std::cout << std::setw(4) << m_WarpData.to_json() << std::endl;

	auto left = ReadBinaryData<double>(document);
	auto top = ReadBinaryData<double>(document);
	auto right = ReadBinaryData<double>(document);
	auto bottom = ReadBinaryData<double>(document);
	m_BoundingBox = Geometry::BoundingBox<double>(Geometry::Point2D<double>(left, top), Geometry::Point2D<double>(right, bottom));


	document.set_offset(len_offset + std::get<uint32_t>(m_Length));
}


void TypeToolTaggedBlock::write([[maybe_unused]] File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{

}

PSAPI_NAMESPACE_END