#pragma once

#include "Macros.h"
#include "Util/Logger.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>
#include <cstring>

#include <mio/mmap.hpp>

#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif


#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN


/// Thread-safe read and write by using a std::mutex to block any reading operations
struct File
{
	struct FileParams
	{
		bool doRead;
		bool forceOverwrite;
		FileParams() : doRead(true), forceOverwrite(false) {};
	};

	// Use this mutex as well for locking throughout the application when IO functions
	// are involved
	std::mutex m_Mutex;

	/// Read n bytes from the file into the input buffer, make sure the buffer is 
	/// properly allocated before running the function
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void read(std::span<uint8_t> buffer);
	
	/// Read a specified number of bytes using a memory mapped file representation meaning
	/// this function is safe to call from any thread. This does not move around the 
	/// internal offset marker unlike setOffsetAndRead.
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void readFromOffset(std::span<uint8_t> buffer, const uint64_t offset);

	/// Write n bytes to the file from the input span.
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void write(std::span<uint8_t> buffer);


	/// Skip n bytes in the file and increment our position marker, checks if the offset 
	/// is possible or if it would exceed the file size. Note: this is a uint64_t
	/// so skipping backwards is legal
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void skip(int64_t size);


	/// Return the current offset from the file start
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	inline uint64_t getOffset() const { return m_Offset; }


	/// Set the current offset to the specified value, checks if the offset is possible
	/// or if it would exceed the file size
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void setOffset(const uint64_t offset);


	/// Set the offset and read into a buffer using a singular lock. 
	/// Use this if you need to skip to a section and read it in a multithreaded environment
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	void setOffsetAndRead(char* buffer, const uint64_t offset, const uint64_t size);


	/// Return the total size of the document
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	inline uint64_t getSize() const noexcept { return m_Size; }


	/// Return the path of the file associated with the File object
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	inline std::filesystem::path getPath() const noexcept { return m_FilePath; };


	/// Return whether we can read the given file from the document or if it would exceed the file size
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	bool can_read(const uint64_t size) const noexcept;

	/// Initialize our File object from a path on disk. If doRead is true the file is only
	/// open for reading while if we set it to false it is only open for writing
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	File(std::filesystem::path file, const FileParams params = FileParams());


private:
	std::filesystem::path m_FilePath;
	std::fstream m_Document;	// The file stream that represents our document
	mio::ummap_source m_DocumentMMap;
	uint64_t m_Size;			// The total size of the document
	uint64_t m_Offset;			// The current document offset.
};

PSAPI_NAMESPACE_END