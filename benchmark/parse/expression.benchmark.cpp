#include <benchmark/benchmark.h>

#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/util/rapidjson.hpp>

#include <string>

using namespace mln;
using namespace mln::style;

// A chain of "-" calls whose innermost call fails to type-check.
static void Parse_NestedFailingExpression(benchmark::State& state) {
    std::string json = R"("a")";
    for (int64_t i = 0; i < state.range(0); ++i) {
        json = R"(["-", )" + json + ", 1]";
    }
    JSDocument document;
    document.Parse<0>(json.c_str());
    const JSValue* value = &document;

    for (auto _ : state) {
        expression::ParsingContext context;
        benchmark::DoNotOptimize(context.parseExpression(conversion::Convertible(value)));
    }
    state.SetComplexityN(state.range(0));
}

BENCHMARK(Parse_NestedFailingExpression)->DenseRange(4, 16, 4)->Complexity();
