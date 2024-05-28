#pragma once

#include "Macros.h"

#include <string>
#include <mutex>

PSAPI_NAMESPACE_BEGIN


/// A simple callback which can be attached to some of the most common read/write operations to query the status of the operation
/// during execution especially when its a long running task. This querying should be done asynchronously by either launching the read/write 
/// asynchronously or in a different thread. The default constructor is the one that the user should be using most of the time
/// as the called code will take over to calculate and set the max value as well as incrementing the counter. 
/// 
/// See the "ProgressCallback" example in the PhotoshopExamples/ folder of the repository for more information on how this can be 
/// achieved
struct ProgressCallback
{
	/// Default ctor
	ProgressCallback() = default;

	/// Initialize the progress callback with a maximum number of elements
	ProgressCallback(size_t maxElement) : m_Max(maxElement) {};

	/// On destruction check if m_Count was able to reach m_Max, otherwise raise a warning
	~ProgressCallback()
	{
		if (m_Count < m_Max)
		{
			PSAPI_LOG_WARNING("Progress", "Counter was deleted before it was able to complete,"\
				" only managed to reach %zu/%zu. Stopped on task: '%s'", m_Count, m_Max, m_CurrentTask.c_str());
		}
	}

	/// Get the current progress of the callback from 0-1 where 1 represents completion
	inline float getProgress() const noexcept { return static_cast<float>(m_Count) / m_Max; }

	/// Get the current task the PhotoshopAPI is working on, this may be empty
	inline std::string getTask() const noexcept { return m_CurrentTask; }

	/// Query whether the current progress is completed or if it is still running
	inline bool isComplete() const noexcept { return m_Count == m_Max; }

	// Increment the internal counter, will never increment past m_Max. This is called by the code 
	// executing the long operation, not the user itself. 
	// This function is thread-safe
	inline void increment() noexcept 
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		if (m_Count + 1 <= m_Max)
			++m_Count;
		else
			PSAPI_LOG_WARNING("Progress", "Incrementing the counter would exceed the maximum value of %zu, ignoring this increment", m_Max);
	}

	// Reset the internal counter back to 0. This is called by the code 
	// executing the long operation, not the user itself
	// This function is not thread-safe
	inline void resetCount() noexcept { m_Count = 0; }

	// Set the max value the counter can increment to. This is called by the code 
	// executing the long operation, not the user itself
	// This function is not thread-safe
	inline void setMax(size_t max) noexcept	{ m_Max = max; }

	// Set the currently executing task. This is called by the code 
	// executing the long operation, not the user itself
	// This function is thread-safe
	inline void setTask(std::string task) noexcept { std::lock_guard<std::mutex> guard(m_Mutex); m_CurrentTask = task; }

	// Return whether or not the callback is initialized, to be used by the internal API
	// only to check if we need to still set m_Max or not if for example we call write()
	// on the LayeredFile it will initialize the values there whereas the PhotoshopFile
	// also has a write() method and this method needs to somehow if this struct was initialized
	// or if it needs to do this itself.
	inline bool isInitialized() const noexcept { return m_Max != 0; }

private:
	/// The current counter progressing towards m_Max
	size_t m_Count = 0;

	/// The maximum m_Count is expected to reach
	size_t m_Max = 0;

	std::string m_CurrentTask = "";

	std::mutex m_Mutex;
};


PSAPI_NAMESPACE_END