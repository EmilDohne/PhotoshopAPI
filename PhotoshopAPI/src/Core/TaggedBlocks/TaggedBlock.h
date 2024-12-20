#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Util/Logger.h"

#include "Core/FileIO/Write.h"

#include <memory>
#include <variant>

PSAPI_NAMESPACE_BEGIN


namespace Impl
{
	/// Write a length block that is either 4- or 8-bytes by simply subtracting the end and start offset
	/// and re-writing the length block at the given offset. If the size does not match the padding we insert padding bytes and write those too
	template <typename T> requires std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>
	void writeLengthBlock(File& document, size_t lenBlockOffset, size_t endOffset, size_t padding)
	{
		if (endOffset < lenBlockOffset)
		{
			PSAPI_LOG_ERROR("TaggedBlock", "Internal Error: Unable to write length block as end offset is supposedly before the length block");
		}

		auto size = endOffset - lenBlockOffset;
		WritePadddingBytes(document, size % padding);
		size += size % padding;
		endOffset = document.getOffset();


		if (size > std::numeric_limits<T>::max())
		{
			PSAPI_LOG_ERROR("TaggedBlock",
				"Unable to write out length block as its size would exceed the size of the numeric limits of T, can at most write %zu bytes but tried to write %zu bytes instead",
				static_cast<size_t>(std::numeric_limits<T>::max()), size);
		}

		document.setOffset(lenBlockOffset);
		WriteBinaryData<T>(document, static_cast<T>(size));
		document.setOffset(endOffset);
	}
}

// Generic Tagged Block which does not hold any data. If you wish to parse further tagged blocks extend this struct and add an implementation
struct TaggedBlock
{
	Signature m_Signature;
	uint64_t m_Offset = 0u;	// Demarkates the start of the taggedblock, not the start of the data
	std::variant<uint32_t, uint64_t> m_Length;

	// Get the total size in a bounds checked manner
	template <typename T = size_t>
	T totalSize() const
	{
		static_assert(std::is_integral_v<T>, "Return must be of integral type");
		// Since m_Size is unsigned we only need to check against max
		if (m_TotalLength > std::numeric_limits<T>::max())
		{
			PSAPI_LOG_ERROR("TaggedBlock", "Unable to access tagged block size with template argument T as it would overflow it");
			return {};
		}
		return static_cast<T>(m_TotalLength);
	}

	void totalSize(size_t value) { m_TotalLength = value; }

	void addTotalSize(size_t increment) { m_TotalLength += increment; }

	Enum::TaggedBlockKey getKey() const noexcept{ return m_Key; };

	virtual ~TaggedBlock() = default;
	TaggedBlock() = default;

	// Read a TaggedBlock from a file
	void read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding = 1u);
	virtual void write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding = 1u);
protected:
	Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Unknown;

private:
	// The length of the tagged block with all the the signature, key and length marker
	// use this value to determine how long the total structure is
	size_t m_TotalLength = 0u;
};


PSAPI_NAMESPACE_END