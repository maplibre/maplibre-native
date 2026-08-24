#include <unistd.h>

#include <ios_test_runner.hpp>
#include <mln/test.hpp>
#include <mln/util/logging.hpp>
#include <vector>

#define EXPORT __attribute__((visibility("default")))

EXPORT
bool TestRunner::startTest(const std::string& basePath) {
    std::vector<std::string> arguments = {"mbgl-test-runner", "--gtest_output=xml:" + basePath + "/test/results.xml"};
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back(const_cast<char*>(arg.data()));
    }
    argv.push_back(nullptr);

    if (chdir(basePath.c_str())) {
        mln::Log::Error(mln::Event::General, "Failed to change the directory to " + basePath);
        return false;
    }

    mln::Log::Info(mln::Event::General, "Start TestRunner");
    int status = mln::runTests(static_cast<uint32_t>(argv.size()), argv.data());
    mln::Log::Info(mln::Event::General, "TestRunner finished with status: '" + std::to_string(status) + "'");

    return status == 0;
}
