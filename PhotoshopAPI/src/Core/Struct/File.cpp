#include "File.h"

#include "Macros.h"
#include "Profiling/Perf/Instrumentor.h"

PSAPI_NAMESPACE_BEGIN

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void File::read(std::span<uint8_t> buffer)
{
	PROFILE_FUNCTION();
	if (buffer.size() == 0)
	{
		return;
	}

	std::lock_guard<std::mutex> guard(m_Mutex);
	if (m_Offset + buffer.size() > m_Size) [[unlikely]]
	{
		PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from offset %" PRIu64 " as it would exceed the file size of %" PRIu64 "", buffer.size(), m_Offset, m_Size);
	}

	m_Document.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
	m_Offset += buffer.size();
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void File::readFromOffset(std::span<uint8_t> buffer, const uint64_t offset)
{
	PROFILE_FUNCTION();
	if (buffer.size() == 0)
	{
		return;
	}

	if (offset + buffer.size() > m_Size) [[unlikely]]
	{
		PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from offset %" PRIu64 " as it would exceed the file size of %" PRIu64 "", buffer.size(), offset, m_Size);
	}
	if (offset + buffer.size() > m_DocumentMMap.size()) [[unlikely]]
	{
		PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from offset %" PRIu64 " as it would exceed the file size of %" PRIu64 "", buffer.size(), offset, m_DocumentMMap.size());
	}

	// We use the memory-mapped file version here to access in parallel
	const uint8_t* srcPtr = m_DocumentMMap.data() + offset;
	std::memcpy(buffer.data(), srcPtr, buffer.size());
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
		PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be skipped from offset %" PRIu64 " as it would exceed the file size of %" PRIu64 "", size, m_Offset, m_Size);
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
	if (offset > m_Size) [[unlikely]]
	{
		PSAPI_LOG_ERROR("File", "Cannot set offset to %" PRIu64 " as it would exceed the file size of %" PRIu64 ".", offset, m_Size);
		return;
	}
	if (offset != m_Offset)
	{
		m_Offset = offset;
		m_Document.seekg(offset, std::ios::beg);
	}

	if (m_Offset + size > m_Size) [[unlikely]]
	{
		PSAPI_LOG_ERROR("File", "Size %" PRIu64 " cannot be read from offset %" PRIu64 " as it would exceed the file size of %" PRIu64 "", size, offset, m_Size);
	}
	{
		PROFILE_SCOPE("File::setOffsetAndRead FileIO");
		m_Document.read(buffer, size);
	}
	m_Offset += size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
File::File(std::filesystem::path file, const FileParams params)
{
	file = file.make_preferred();
	// Check if the parent path exists and if not we will create it
	if (!std::filesystem::is_directory(file.parent_path()) && (!file.parent_path().empty() || file.has_root_directory()))
	{
		std::filesystem::create_directories(file.parent_path());
		PSAPI_LOG("Created directory '%s' as it didnt exist", file.parent_path().string().c_str());
	}
	m_Offset = 0;
	m_Size = 0;

	// Check if the file exists and otherwise create it
	if (params.doRead == true)
	{
		if (std::filesystem::exists(file))
		{
			m_Document.open(file, std::ios::binary | std::fstream::in);
			m_DocumentMMap = mio::ummap_source(file.string());
		}
		else
		{
			PSAPI_LOG_ERROR("File", "File %s does not exist, aborting parsing", file.string().c_str());
		}
	}
	else
	{
		if (std::filesystem::exists(file))
		{
			if (params.forceOverwrite)
			{
				std::filesystem::remove(file);
				PSAPI_LOG("File", "Removed file %s", file.string().c_str());
			}
			m_Document.open(file, std::ios::binary | std::fstream::out | std::fstream::trunc);
			PSAPI_LOG("File", "Created file %s", file.string().c_str());
		}
		else
		{
			m_Document.open(file, std::ios::binary | std::fstream::out | std::fstream::trunc);
			PSAPI_LOG("File", "Created file %s", file.string().c_str());
		}
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

	m_FilePath = file;
}


PSAPI_NAMESPACE_END