#include <benchmark/benchmark.h>

#include <mbgl/util/dtoa.hpp>

#include <cfloat>
#include <cmath>

using namespace mbgl;

static void Util_dtoa(::benchmark::State& state) {
    while (state.KeepRunning()) {
        util::dtoa(0.);
        util::dtoa(M_E);
        util::dtoa(M_LOG2E);
        util::dtoa(M_LOG10E);
        util::dtoa(M_LN2);
        util::dtoa(M_LN10);
        util::dtoa(M_PI);
        util::dtoa(M_PI_2);
        util::dtoa(M_PI_4);
        util::dtoa(M_1_PI);
        util::dtoa(M_2_PI);
        util::dtoa(M_2_SQRTPI);
        util::dtoa(M_SQRT2);
        util::dtoa(M_SQRT1_2);
    }
}

static void Util_standardDtoa(::benchmark::State& state) {
    while (state.KeepRunning()) {
        std::ignore = std::to_string(0.);
        std::ignore = std::to_string(M_E);
        std::ignore = std::to_string(M_LOG2E);
        std::ignore = std::to_string(M_LOG10E);
        std::ignore = std::to_string(M_LN2);
        std::ignore = std::to_string(M_LN10);
        std::ignore = std::to_string(M_PI);
        std::ignore = std::to_string(M_PI_2);
        std::ignore = std::to_string(M_PI_4);
        std::ignore = std::to_string(M_1_PI);
        std::ignore = std::to_string(M_2_PI);
        std::ignore = std::to_string(M_2_SQRTPI);
        std::ignore = std::to_string(M_SQRT2);
        std::ignore = std::to_string(M_SQRT1_2);
    }
}

static void Util_dtoaLimits(::benchmark::State& state) {
    while (state.KeepRunning()) {
        util::dtoa(DBL_MIN);
        util::dtoa(DBL_MAX);
    }
}

static void Util_standardDtoaLimits(::benchmark::State& state) {
    while (state.KeepRunning()) {
        std::ignore = std::to_string(DBL_MIN);
        std::ignore = std::to_string(DBL_MAX);
    }
}

BENCHMARK(Util_dtoa);
BENCHMARK(Util_standardDtoa);

BENCHMARK(Util_dtoaLimits);
BENCHMARK(Util_standardDtoaLimits);
