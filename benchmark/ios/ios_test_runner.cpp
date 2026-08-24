#include <ios_test_runner.hpp>

#include <mln/benchmark.hpp>

#include <mln/util/logging.hpp>

#include <unistd.h>
#include <vector>

#define EXPORT __attribute__((visibility("default")))

EXPORT
bool TestRunner::startTest(const std::string& basePath) {
    std::vector<std::string> arguments = {"mbgl-benchmark-runner",
                                          "--benchmark_repetitions=10",
                                          "--benchmark_format=json",
                                          "--benchmark_out=" + basePath + "/benchmark/results.json"};
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back(const_cast<char*>(arg.data()));
    }
    argv.push_back(nullptr);

    if (chdir(basePath.c_str())) {
        mln::Log::Error(mln::Event::General, "Failed to change the directory to " + basePath);
        return false;
    }

    mln::Log::Info(mln::Event::General, "Start BenchmarkRunner");
    int status = mln::runBenchmark(static_cast<uint32_t>(argv.size()), argv.data());
    mln::Log::Info(mln::Event::General, "BenchmarkRunner finished with status: '%d'", status);

    return status == 0;
}
