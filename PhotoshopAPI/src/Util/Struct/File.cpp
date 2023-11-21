#include "File.h"

#include "Macros.h"

PSAPI_NAMESPACE_BEGIN

File::File(const std::filesystem::path& file)
{
	m_Offset = 0;
	m_Size = 0;

	m_Document.open(file, std::ios::binary);

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