#pragma once

#include "Logger.h"

#include <chrono>
#include <filesystem>
#include <string>


/// Simple profiler struct that initializes on construction and writes to file and stdout on destruction, can be used like a scoped timer
/// where it is initialized and destructed automatically
struct Profiler
{
	inline Profiler(const std::filesystem::path& outFile, const std::string& benchName)
	{
		m_Start = std::chrono::high_resolution_clock::now();
		m_Path = outFile;
		m_BenchName = benchName;
	}

	inline ~Profiler()
	{
		m_End = std::chrono::high_resolution_clock::now();

		std::ofstream outfile;
		outfile.open(m_Path, std::ios_base::app);
		outfile << m_BenchName << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(m_End - m_Start).count() << "ms\n";
		PSAPI_LOG("Benchmark", "Bench %s finished execution in %u ms", m_BenchName.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(m_End - m_Start).count());
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_End;
	std::filesystem::path m_Path;
	std::string m_BenchName;
};