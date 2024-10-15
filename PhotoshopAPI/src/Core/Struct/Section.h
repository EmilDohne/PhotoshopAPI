#pragma once

#include "Macros.h"
#include "File.h"

#include <optional>
#include <type_traits>
#include <limits>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

// Forward declare FileHeader to avoid any issues
struct FileHeader;

struct FileSection
{
	/// Initialize the file section with a size and offset. If re-initialization is needed this function may be 
	/// called again
	inline void initialize(size_t offset, size_t size) noexcept { m_Offset = offset; m_Size = size; }

	/// Each FileSection must implement a way to calculate its own section size in bytes based on the 
	/// data it holds. This is important for writing to disk as we sometimes hold a section size marker.
	/// The size must include this marker (if applicable) and when writing the marker we subtract the size of it.
	/// 
	/// \note Any sections that include image data (LayerAndMaskInformation and ImageData) will not be able to 
	///		calculate the section size due to offloading the compression to the write step. For these
	///		sections the size calculation gets done on write.
	/// 
	/// \param header An optional ptr to the document FileHeader, only some functions require this to be present
	virtual uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const  = 0;

	/// Get the size of the FileSection as the given integral type checking internally if this access would 
	/// overflow the template argument T. This function is primarily intended for structures such as a PascalString
	/// which may only be a max of uint8_t in size.
	template <typename T = size_t>
	T size() const
	{
		static_assert(std::is_integral_v<T>, "Return must be of integral type");

		// Since m_Size is unsigned we only need to check against max
		if (m_Size > static_cast<size_t>(std::numeric_limits<T>::max()))
		{
			PSAPI_LOG_ERROR("FileSection", "Unable to access file section size with template argument T as it would overflow it");
			return {};
		}
		return static_cast<T>(m_Size);
	}

	/// Set the size of the file section
	inline void size(size_t size) noexcept { m_Size = size; }

	/// Add to the size of the FileSection.
	inline void addSize(size_t increment) { m_Size += increment; }

	size_t offset() const noexcept { return m_Offset; }

private:
	size_t m_Offset = 0u;
	size_t m_Size = 0u;	// Store the size of the whole section (including the length marker if applicable)
};


PSAPI_NAMESPACE_END