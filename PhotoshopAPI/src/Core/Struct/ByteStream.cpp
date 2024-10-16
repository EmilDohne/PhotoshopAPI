#include "ByteStream.h"

#include "Profiling/Perf/Instrumentor.h"

#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif


#include <cstring>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ByteStream::setOffset(const uint64_t offset)
{
	if (offset > m_Size)
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to access illegal offset, maximum is %" PRIu64 " but got %" PRIu64 " instead", m_Size, offset);
	}
	m_Offset = offset;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ByteStream::read(std::span<uint8_t> buffer)
{
	PROFILE_FUNCTION();
	if (buffer.size() == 0)
	{
		return;
	}

	if (m_Offset + buffer.size() > m_Size)
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to read too much data, maximum is %" PRIu64 " but got %" PRIu64 " instead", m_Size, m_Offset + buffer.size());
	}
	if (m_Buffer.size() - 1 < m_Offset + buffer.size())
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to read too much data, maximum is %zu but got %" PRIu64 " instead", m_Buffer.size() - 1, m_Offset + buffer.size());
	}

	// Use memcpy to copy data from m_Buffer to the provided buffer
	std::memcpy(buffer.data(), m_Buffer.data() + m_Offset, buffer.size());
	m_Offset += buffer.size();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ByteStream::read(std::span<uint8_t> buffer, uint64_t offset)
{
	PROFILE_FUNCTION();
	if (buffer.size() == 0)
	{
		return;
	}

	if (offset > m_Size)
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to access illegal offset, maximum is %" PRIu64 " but got %" PRIu64 " instead", m_Size, offset);
	}
	if (offset + buffer.size() > m_Size)
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to read too much data, maximum is %" PRIu64 " but got %" PRIu64 " instead", m_Size, m_Offset + buffer.size());
	}
	if (m_Buffer.size() < offset + buffer.size())
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to read too much data, maximum is %zu but got %" PRIu64 " instead", m_Buffer.size() - 1, m_Offset + buffer.size());
	}

	// Use memcpy to copy data from m_Buffer to the provided buffer
	std::memcpy(buffer.data(), m_Buffer.data() + offset, buffer.size());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::span<uint8_t> ByteStream::read(uint64_t size)
{
	PROFILE_FUNCTION();
	if (m_Offset + size > m_Size)
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to read too much data, maximum is %" PRIu64 " but got %" PRIu64 " instead", m_Size, m_Offset + size);
	}
	return std::span<uint8_t>(m_Buffer.data() + m_Offset, size);
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::span<uint8_t> ByteStream::read(uint64_t offset, uint64_t size)
{
	PROFILE_FUNCTION();
	if (offset > m_Size)
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to access illegal offset, maximum is %" PRIu64 " but got %" PRIu64 " instead", m_Size, offset);
	}
	if (offset + size > m_Size)
	{
		PSAPI_LOG_ERROR("ByteStream", "Trying to read too much data, maximum is %" PRIu64 " but got %" PRIu64 " instead", m_Size, m_Offset + size);
	}
	return std::span<uint8_t>(m_Buffer.data() + offset, size);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ByteStream::ByteStream(File& document, const uint64_t offset, const uint64_t size)
{
	PROFILE_FUNCTION();
	{
		PROFILE_SCOPE("Vector malloc");
		m_Buffer = std::vector<uint8_t>(size);
	}
	m_Size = size;
	document.readFromOffset(m_Buffer, offset);
	m_FileOffset = offset;
}


PSAPI_NAMESPACE_END