#pragma once

#include "../../Macros.h"

#include "../Logger.h"

#include <filesystem>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <fstream>

PSAPI_NAMESPACE_BEGIN

struct File
{

	inline void read(char* buffer, uint64_t size)
	{
		if (m_Offset + size > m_Size)
		{
			PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from the file as it would exceed the file size", size)
		}

		m_Document.read(buffer, size);
		m_Offset += size;
	}


	inline void skip(int64_t size)
	{
		if (size <= 0)
		{
			return;
		}
		if (this->m_Offset + size > this->m_Size)
		{
			PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from the file as it would exceed the file size", size)
		}
		m_Document.ignore(size);
		this->m_Offset += size;
	}


	inline uint64_t getOffset()
	{
		return m_Offset;
	}


	inline void setOffset(const uint64_t offset)
	{
		if (offset == this->m_Offset)
		{
			return;
		}
		if (offset > m_Size)
		{
			PSAPI_LOG_ERROR("File", "Cannot set offset to %" PRIu64 " as it would exceed the file size of %" PRIu64 ".", offset, this->m_Size);
			return;
		}
		this->m_Offset = offset;
		this->m_Document.seekg(offset, std::ios::beg);
	}


	inline uint64_t getSize()
	{
		return m_Size;
	}

	File(const std::filesystem::path& file);

private:
	std::ifstream m_Document;
	uint64_t m_Size;
	uint64_t m_Offset;
};

PSAPI_NAMESPACE_END