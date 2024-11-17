// Include doctest and configure our own main function
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Macros.h"
#include "DetectArmMac.h"
#include "Util/Enum.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include "Util/Profiling/Perf/Instrumentor.h"
#include "Util/Profiling/Memory/CompressionTracker.h"

#include <filesystem>
#include <vector>
#include <memory>

#include <fmt/format.h>

/// Create a reporter which prints out failure statistics at the end
struct FailureReporter : public doctest::ConsoleReporter
{
	FailureReporter(const  doctest::ContextOptions& opt) : doctest::ConsoleReporter(opt) {}

	void test_case_start(const doctest::TestCaseData& in) override { tc = &in; }


	void test_case_end(const doctest::CurrentTestCaseStats& in) override 
	{
		if (in.failure_flags == doctest::TestCaseFailureReason::Exception ||
			in.failure_flags == doctest::TestCaseFailureReason::Crash ||
			in.failure_flags == doctest::TestCaseFailureReason::ShouldHaveFailedButDidnt
			)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (tc)
			{
				std::string reason = "";
				if (in.failure_flags == doctest::TestCaseFailureReason::Exception)
				{
					reason = "Exception";
				}
				else if (in.failure_flags == doctest::TestCaseFailureReason::Crash)
				{
					reason = "Crash";
				}
				else if (in.failure_flags == doctest::TestCaseFailureReason::ShouldHaveFailedButDidnt)
				{
					reason = "Expected failure not failing";
				}

				constexpr const char* blue = "\033[36m";
				constexpr const char* red = "\033[31m";
				constexpr const char* gold = "\033[33m";
				constexpr const char* reset = "\033[0m";
				std::cout 
					<< gold << "===============================================================================\n"
					<< blue << "[doctest]" << reset << " Failure in test case: " << red << std::string(tc->m_name) << reset
					<< " with reason: " << gold << reason << "\n"
					<< "===============================================================================\n" << reset
					<< std::endl;
			}
		}
	}


	void report_query(const doctest::QueryData& /*in*/) override {}

	void test_run_start() override {}

	void test_run_end(const doctest::TestRunStats& /*in*/) override {}

	void test_case_reenter(const doctest::TestCaseData& /*in*/) override {}

	void test_case_exception(const doctest::TestCaseException& /*in*/) override {}

	void subcase_start(const doctest::SubcaseSignature& /*in*/) override {}

	void subcase_end() override {}

	void log_assert(const doctest::AssertData& in) override {}

	void log_message(const doctest::MessageData& /*in*/) override {}

	void test_case_skipped(const doctest::TestCaseData& /*in*/) override {}


private:

	const doctest::TestCaseData* tc = nullptr;
	std::mutex						mutex;
};

REGISTER_LISTENER("failure", /*priority=*/1, FailureReporter);


int main()
{
#if ARM_MAC_ARCH
	PSAPI_LOG_WARNING("Test", "Detected we are running on an ARM-based MacOS system which means we disable any deliberately failing tests as these would segfault due to incorrect exception handling.");
#endif
	doctest::Context context;
	int res = context.run();

	if (context.shouldExit())
		return res;
	return res;
}