#pragma once

#include "Macros.h"
#include "EndianByteSwap.h"
#include "File.h"

#include <vector>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN


// A stream of binary data with its own internal offset and size markers.
// It is meant to replace the read functionality in sections where we cannot dynamically 
// read from the document itself. This object is meant to represent the binary stream fro a single thread
struct ByteStream
{

	void setOffset(const uint64_t offset);
	inline uint64_t getOffset() const { return m_Offset; };
	inline uint64_t getSize() const { return m_Size; };

	// Read n amount of bytes into the given buffer
	void read(char* buffer, uint64_t size);

	ByteStream(File& document, const uint64_t offset, const uint64_t size);

private:
	std::vector<uint8_t> m_Buffer;
	uint64_t m_Offset = 0u;	// Internal offset for our data
	uint64_t m_FileOffset = 0u; // The location in the file we are at
	uint64_t m_Size = 0u;	// Total size of the buffer
};


PSAPI_NAMESPACE_END