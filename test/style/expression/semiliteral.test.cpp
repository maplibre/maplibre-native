#include <mln/style/conversion_impl.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/style/expression/dsl.hpp>
#include <mln/style/expression/is_constant.hpp>
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

TEST(Semiliteral, Constants) {
    for (const auto* json : {R"(["semiliteral", []])",
                             R"(["semiliteral", [1, ["+", 2, 3]]])",
                             R"(["semiliteral", ["x", true, null]])",
                             R"(["semiliteral", "x"])",
                             R"(["semiliteral", {"x": ["get", "x"]}])"}) {
        ParsingContext ctx;
        auto result = parseSemiliteral(json, ctx);
        ASSERT_TRUE(result) << ctx.getCombinedErrors();
        EXPECT_EQ((*result)->getKind(), Kind::Literal);
    }
    ParsingContext ctx;
    auto result = parseSemiliteral(R"(["semiliteral", [1, ["+", 2, 3]]])", ctx);
    ASSERT_TRUE(result);
    EXPECT_EQ((*result)->getType(), type::Type(type::Array(type::Number, 2)));
    EXPECT_EQ((*result)->evaluate(EvaluationContext{})->get<std::vector<expression::Value>>(),
              (std::vector<expression::Value>{1.0, 5.0}));
}

TEST(Semiliteral, EvaluationAndRoundTrip) {
    const char* json = R"(["semiliteral", [["number", ["get", "x"]], ["*", 2, ["get", "y"]]]])";
    ParsingContext ctx(type::Array(type::Number, 2));
    auto result = parseSemiliteral(json, ctx, true);
    ASSERT_TRUE(result) << ctx.getCombinedErrors();
    EXPECT_EQ((*result)->getKind(), Kind::Semiliteral);
    EXPECT_TRUE((*result)->has(Dependency::Feature));
    EXPECT_FALSE(isFeatureConstant(**result));
    EXPECT_TRUE(isZoomConstant(**result));
    Feature feature;
    feature.properties = {{"x", 3.0}, {"y", 4.0}};
    auto evaluated = (*result)->evaluate(0.0f, feature, std::nullopt);
    ASSERT_TRUE(evaluated);
    EXPECT_EQ(evaluated->get<std::vector<expression::Value>>(), (std::vector<expression::Value>{3.0, 8.0}));
    auto serialized = (*result)->serialize();
    const auto& array = serialized.get<std::vector<mln::Value>>();
    ASSERT_EQ(array.size(), 2u);
    ASSERT_EQ(array[1].get<std::vector<mln::Value>>().size(), 2u);
    ParsingContext roundTrip;
    auto reparsed = parseSemiliteral(stringify(toExpressionValue(serialized)).c_str(), roundTrip);
    ASSERT_TRUE(reparsed) << roundTrip.getCombinedErrors();
    EXPECT_EQ(**result, **reparsed);
    feature.properties["x"] = "wrong";
    EXPECT_FALSE((*result)->evaluate(0.0f, feature, std::nullopt));
}

TEST(Semiliteral, NestedAndBindings) {
    ParsingContext ctx;
    auto result = parseSemiliteral(
        R"(["let", "x", ["get", "x"], ["semiliteral", ["top", ["semiliteral", [["var", "x"], 2]], ["literal", ["get", "x"]]]]])",
        ctx);
    ASSERT_TRUE(result) << ctx.getCombinedErrors();
    Feature feature;
    feature.properties = {{"x", 3.0}};
    auto value = (*result)->evaluate(0.0f, feature, std::nullopt);
    ASSERT_TRUE(value);
    const auto& array = value->get<std::vector<expression::Value>>();
    EXPECT_EQ(array[0], expression::Value(std::string("top")));
    EXPECT_EQ(array[1].get<std::vector<expression::Value>>(), (std::vector<expression::Value>{3.0, 2.0}));
    EXPECT_EQ(array[2].get<std::vector<expression::Value>>(),
              (std::vector<expression::Value>{std::string("get"), std::string("x")}));
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

TEST(Semiliteral, DependenciesAndEquality) {
    ParsingContext ctx;
    auto result = parseSemiliteral(R"(["semiliteral", [["feature-state", "x"], ["zoom"]]])", ctx);
    ASSERT_TRUE(result);
    EXPECT_TRUE((*result)->has(Dependency::Feature));
    EXPECT_TRUE((*result)->has(Dependency::Zoom));
    EXPECT_FALSE(isZoomConstant(**result));
    ParsingContext otherCtx;
    auto other = parseSemiliteral(R"(["semiliteral", [["zoom"], ["feature-state", "x"]]])", otherCtx);
    ASSERT_TRUE(other);
    EXPECT_NE(**result, **other);
    EXPECT_NE(**result, *dsl::literal(0.0));
}

TEST(Semiliteral, LiteralObjectAndFirstError) {
    ParsingContext ctx;
    auto object = parseSemiliteral(R"(["semiliteral", {"x": ["get", "x"]}])", ctx);
    ASSERT_TRUE(object);
    auto evaluated = (*object)->evaluate(EvaluationContext{});
    ASSERT_TRUE(evaluated);
    EXPECT_EQ((evaluated->get<std::unordered_map<std::string, expression::Value>>().at("x")),
              expression::Value(std::vector<expression::Value>{std::string("get"), std::string("x")}));

    ParsingContext errors;
    auto failing = parseSemiliteral(R"(["semiliteral", [["number", ["get", "x"]], ["error", "second"]]])", errors);
    ASSERT_TRUE(failing);
    Feature feature;
    auto error = (*failing)->evaluate(0.0f, feature, std::nullopt);
    ASSERT_FALSE(error);
    EXPECT_NE(error.error().message.find("number"), std::string::npos);
    EXPECT_EQ(error.error().message.find("second"), std::string::npos);
}
