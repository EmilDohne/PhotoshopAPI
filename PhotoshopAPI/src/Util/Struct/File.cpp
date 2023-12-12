#include "File.h"

#include "Macros.h"

PSAPI_NAMESPACE_BEGIN

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void File::read(char* buffer, uint64_t size)
{
	std::lock_guard<std::mutex> guard(m_Mutex);
	if (m_Offset + size > m_Size)
	{
		PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from the file as it would exceed the file size", size)
	}

	m_Document.read(buffer, size);
	m_Offset += size;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void File::write(std::span<uint8_t> buffer)
{
	std::lock_guard<std::mutex> guard(m_Mutex);
	m_Size += buffer.size();
	m_Offset += buffer.size();
	m_Document.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void File::skip(int64_t size)
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


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void File::setOffset(const uint64_t offset)
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


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void File::setOffsetAndRead(char* buffer, const uint64_t offset, const uint64_t size)
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
File::File(const std::filesystem::path& file)
{
	m_Offset = 0;
	m_Size = 0;


	// Check if the file exists and otherwise create it
	if (std::filesystem::exists(file))
	{
		m_Document.open(file, std::ios::binary | std::fstream::in | std::fstream::out);
	}
	else
	{
		PSAPI_LOG("File", "Created file %s", file.string().c_str())
		m_Document.open(file, std::ios::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
	}

	if (m_Document.is_open())
	{
		// Get the size of the file
		m_Document.seekg(0, std::ios::end);
		m_Size = static_cast<uint64_t>(m_Document.tellg());
		m_Document.seekg(0, std::ios::beg);
	}
	else
	{
		// Handle file open error
		PSAPI_LOG_ERROR("File", "Failed to open file: %s", file.string().c_str());
	}
}


PSAPI_NAMESPACE_END