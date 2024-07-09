// Include doctest and configure our own main function
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Macros.h"
#include "DetectArmMac.h"
#include "Enum.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include "Profiling/Perf/Instrumentor.h"
#include "Profiling/Memory/CompressionTracker.h"

#include <filesystem>
#include <vector>
#include <memory>


int main()
{
#if ARM_MAC_ARCH
	PSAPI_LOG_WARNING("Test", "Detected we are running on an ARM-based MacOS system which means we disable any deliberately failing tests as these would segfault due to incorrect exception handling.");
#endif
	doctest::Context context;

	// set defaults
	context.setOption("abort-after", 5);	// stop test execution after 5 failed 

	int res = context.run();				// run

	if (context.shouldExit())
		return res;
	return res;
}