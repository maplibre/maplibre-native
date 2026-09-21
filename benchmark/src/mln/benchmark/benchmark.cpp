#include <mln/benchmark.hpp>

#include <benchmark/benchmark.h>

namespace mln {

int runBenchmark(int argc, char* argv[]) {
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}

} // namespace mln
