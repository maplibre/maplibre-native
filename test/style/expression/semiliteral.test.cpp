#include <mln/style/conversion_impl.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/style/expression/semiliteral.hpp>
#include <mln/test/util.hpp>
#include <mln/util/rapidjson.hpp>

using namespace mln;
using namespace mln::style;
using namespace mln::style::expression;

namespace {
ParseResult parseSemiliteral(const char* json, ParsingContext& ctx, bool property = false) {
    JSDocument document;
    document.Parse(json);
    const JSValue* value = &document;
    return property ? ctx.parseLayerPropertyExpression(conversion::Convertible(value))
                    : ctx.parseExpression(conversion::Convertible(value));
}
} // namespace

TEST(Semiliteral, ConstantFolding) {
    ParsingContext ctx;
    auto result = parseSemiliteral(R"(["semiliteral", [1, ["+", 2, 3]]])", ctx);
    ASSERT_TRUE(result);
    EXPECT_EQ((*result)->getKind(), Kind::Literal);
    EXPECT_EQ((*result)->getType(), type::Type(type::Array(type::Number, 2)));
    EXPECT_EQ((*result)->evaluate(EvaluationContext{})->get<std::vector<expression::Value>>(),
              (std::vector<expression::Value>{1.0, 5.0}));
}

TEST(Semiliteral, Invalid) {
    for (const auto* json : {R"(["semiliteral"])",
                             R"(["semiliteral", [], []])",
                             R"(["semiliteral", [["unknown"]]])",
                             R"(["semiliteral", [[1, 2]]])",
                             R"(["semiliteral", [{}]])"}) {
        ParsingContext ctx;
        EXPECT_FALSE(parseSemiliteral(json, ctx)) << json;
        EXPECT_FALSE(ctx.getErrors().empty());
    }

    for (const auto* json : {R"(["semiliteral", [["get", "x"], ["get", "y"]]])",
                             R"(["semiliteral", [1]])",
                             R"(["semiliteral", [1, "x"]])",
                             R"(["semiliteral", [["zoom"], 1]])"}) {
        ParsingContext ctx(type::Array(type::Number, 2));
        EXPECT_FALSE(parseSemiliteral(json, ctx, true)) << json;
    }
}

TEST(Semiliteral, FirstError) {
    ParsingContext errors;
    auto failing = parseSemiliteral(R"(["semiliteral", [["number", ["get", "x"]], ["error", "second"]]])", errors);
    ASSERT_TRUE(failing);
    Feature feature;
    auto error = (*failing)->evaluate(0.0f, feature, std::nullopt);
    ASSERT_FALSE(error);
    EXPECT_NE(error.error().message.find("number"), std::string::npos);
    EXPECT_EQ(error.error().message.find("second"), std::string::npos);
}
