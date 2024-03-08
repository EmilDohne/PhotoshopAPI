// Include doctest and configure our own main function
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Macros.h"
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
	doctest::Context context;

	// set defaults
	context.setOption("abort-after", 9999);	// stop test execution after 5 failed assertions

	int res = context.run();				// run

	if (context.shouldExit())				// important - query flags (and --exit) rely on the user doing this
		return res;							// propagate the result of the tests
}