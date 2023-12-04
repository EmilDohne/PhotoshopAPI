#pragma once

#include "Macros.h"
#include "Logger.h"

#include <vector>
#include <string>


#if PSAPI_PROFILING
#define REGISTER_COMPRESSION_TRACK(compressedBytes, uncompressedBytes) NAMESPACE_PSAPI::CompressionTracker::Get().WriteProfile({compressedBytes, uncompressedBytes})
#else
#define REGISTER_COMPRESSION_TRACK(compressedBytes, uncompressedBytes)
#endif

#include <mutex>

PSAPI_NAMESPACE_BEGIN

struct CompressionResults
{
    uint64_t compressedSize;
    uint64_t uncompressedSize;
};

struct CompressionTrackerSession
{
    std::string Name;
};

class CompressionTracker
{
private:
    CompressionTrackerSession* m_CurrentSession;
    uint64_t m_CompressedSize = 0u;
    uint64_t m_UncompressedSize = 0u;
    std::mutex m_lock;
public:
    CompressionTracker()
        : m_CurrentSession(nullptr)
    {
    }

    void BeginSession(const std::string& name)
    {
        m_CurrentSession = new CompressionTrackerSession{ name };
    }

    void EndSession()
    {
        PSAPI_LOG("CompressionTracker", "Total size compressed %" PRIu64 " Megabytes", m_CompressedSize / 1024 / 1024)
        PSAPI_LOG("CompressionTracker", "Total size uncompressed %" PRIu64 " Megabytes", m_UncompressedSize / 1024 / 1024)
        delete m_CurrentSession;
        m_CurrentSession = nullptr;
    }

    void WriteProfile(const CompressionResults& result)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_CompressedSize += result.compressedSize;
        m_UncompressedSize += result.uncompressedSize;
    }

    static CompressionTracker& Get()
    {
        static CompressionTracker instance;
        return instance;
    }
};


PSAPI_NAMESPACE_END