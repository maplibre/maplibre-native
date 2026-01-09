#include <benchmark/benchmark.h>

#include <string>
#include <vector>
#include <mbgl/util/color.hpp>

static const std::vector<std::string> testStrings = {"#000000",
                                                     "#FFFFFF",
                                                     "#FF00FFAA",
                                                     "rgba(255, 0, 0, 1.0)",
                                                     "rgb(0, 255, 0)",
                                                     "blue",
                                                     "red",
                                                     "invalid-color",
                                                     "rgba(255, 255, 255, 0.5)",
                                                     "#123"};

namespace {

void ColorParse(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& str : testStrings) {
            auto result = mbgl::Color::parse(str);
            benchmark::DoNotOptimize(result);
        }
    }
}

}; // namespace

BENCHMARK(ColorParse);
