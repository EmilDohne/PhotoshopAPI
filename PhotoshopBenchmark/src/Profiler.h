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


	/// Register an individual time point, if this is called at least once we use this way of measuring instead of the ctor dtor one
	inline void startTimePoint()
	{
		m_StartPoints.push_back(std::chrono::high_resolution_clock::now());

	}

	/// End a given timePoint
	inline void endTimePoint()
	{
		m_EndPoints.push_back(std::chrono::high_resolution_clock::now());
	}

	inline ~Profiler()
	{
		m_End = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds totalTime{};

		if (m_StartPoints.size() == m_EndPoints.size() && m_StartPoints.size() > 0)
		{
			for (size_t i = 0; i < m_StartPoints.size(); ++i)
			{
				totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(m_EndPoints[i] - m_StartPoints[i]);
			}
			PSAPI_LOG("Benchmark", "Bench %s finished execution in %u ms with a wall time of %u ms", m_BenchName.c_str(), totalTime.count(),
				std::chrono::duration_cast<std::chrono::milliseconds>(m_End - m_Start).count());
		}
		else
		{
			totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_End - m_Start);
			PSAPI_LOG("Benchmark", "Bench %s finished execution in %u ms", m_BenchName.c_str(), totalTime.count());
		}

		std::ofstream outfile;
		outfile.open(m_Path, std::ios_base::app);
		outfile << m_BenchName << ": " << totalTime.count() << "ms\n";
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_End;

	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> m_StartPoints;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> m_EndPoints;

	std::filesystem::path m_Path;
	std::string m_BenchName;
};