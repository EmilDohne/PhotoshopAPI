#pragma once

#include "Macros.h"
#include "Logger.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN


// Thread-safe read and write by using a std::mutex to block any reading operations
struct File
{
	// Use this mutex as well for locking throughout the application when IO functions
	// are involved
	std::mutex m_Mutex;

	// Read n bytes from the file into the input buffer, make sure the buffer is 
	// properly allocated before running the function
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void read(char* buffer, uint64_t size);
	

	// Write n bytes to the file from the input buffer.
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void write(char* buffer, uint64_t size);


	// Skip n bytes in the file and increment our position marker, checks if the offset 
	// is possible or if it would exceed the file size. Note: this is a uint64_t
	// so skipping backwards is legal
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void skip(int64_t size);


	// Return the current offset from the file start
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	inline uint64_t getOffset() const { return m_Offset; }


	// Set the current offset to the specified value, checks if the offset is possible
	// or if it would exceed the file size
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void setOffset(const uint64_t offset);


	// Set the offset and read into a buffer using a singular lock. 
	// Use this if you need to skip to a section and read it in a multithreaded environment
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void setOffsetAndRead(char* buffer, const uint64_t offset, const uint64_t size);


	// Return the total size of the document
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	inline uint64_t getSize() const { return m_Size; }


	// Initialize our File object from a path on disk
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	File(const std::filesystem::path& file);


private:
	std::fstream m_Document;	// The file stream that represents our document
	uint64_t m_Size;			// The total size of the document
	uint64_t m_Offset;			// The current document offset.
};

PSAPI_NAMESPACE_END