#pragma once

#include "Macros.h"
#include "Logger.h"

#include <filesystem>
#include <fstream>
#include <mutex>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN


// Thread-safe read and write by using a std::mutex to block any reading operations
struct File
{
	// Use this mutex as well for locking throughout the application
	std::mutex m_Mutex;

	inline void read(char* buffer, uint64_t size)
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		if (m_Offset + size > m_Size)
		{
			PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from the file as it would exceed the file size", size)
		}

		m_Document.read(buffer, size);
		m_Offset += size;
	}
	

	inline void skip(int64_t size)
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		if (size <= 0)
		{
			return;
		}
		if (m_Offset + size > m_Size)
		{
			PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from the file as it would exceed the file size", size)
		}
		m_Document.ignore(size);
		m_Offset += size;
	}


	inline uint64_t getOffset() const
	{
		return m_Offset;
	}


	inline void setOffset(const uint64_t offset)
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		if (offset == m_Offset)
		{
			return;
		}
		if (offset > m_Size)
		{
			PSAPI_LOG_ERROR("File", "Cannot set offset to %" PRIu64 " as it would exceed the file size of %" PRIu64 ".", offset, m_Size);
			return;
		}
		m_Offset = offset;
		m_Document.seekg(offset, std::ios::beg);
	}

	// Set the offset and read into a buffer using a singular lock. Use this if you need to skip to a section and read it in a multithreaded environment
	inline void setOffsetAndRead(char* buffer, const uint64_t offset, const uint64_t size)
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		if (offset > m_Size)
		{
			PSAPI_LOG_ERROR("File", "Cannot set offset to %" PRIu64 " as it would exceed the file size of %" PRIu64 ".", offset, m_Size);
			return;
		}
		if (offset != m_Offset)
		{
			m_Offset = offset;
			m_Document.seekg(offset, std::ios::beg);
		}

		if (m_Offset + size > m_Size)
		{
			PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from the file as it would exceed the file size", size)
		}

		m_Document.read(buffer, size);
		m_Offset += size;
	}


	inline uint64_t getSize() const
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