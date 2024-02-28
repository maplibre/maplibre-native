#include <mbgl/style/expression/dsl.hpp>
#include <mbgl/style/expression/format_section_override.hpp>
#include <mbgl/test/util.hpp>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::expression;
using namespace mbgl::style::expression::dsl;

using namespace std::string_literals;

namespace std {
using ::mbgl::operator<<;
}

template <typename T>
auto createOverride(expression::type::Type exprType,
                    PossiblyEvaluatedPropertyValue<T> propValue,
                    std::string propName) {
    return std::make_unique<FormatSectionOverride<T>>(std::move(exprType), std::move(propValue), std::move(propName));
}

TEST(ExpressionDependencies, Get) {
    EXPECT_EQ(Dependency::Feature, number(get("property"))->dependencies);
}

TEST(ExpressionDependencies, Id) {
    EXPECT_EQ(Dependency::Feature, id()->dependencies);
}

TEST(ExpressionDependencies, Zoom) {
    EXPECT_EQ(Dependency::Zoom, zoom()->dependencies);
}

TEST(ExpressionDependencies, Convert) {
    EXPECT_EQ(Dependency::Feature, toColor(string(get("color")))->dependencies);
    EXPECT_EQ(Dependency::Feature, toString(number(string(get("color"))))->dependencies);
    EXPECT_EQ(Dependency::Feature, boolean(string(get("color")))->dependencies);
    EXPECT_EQ(Dependency::Feature, toFormatted(toColor(string(get("color"))))->dependencies);
    EXPECT_EQ(Dependency::Feature | Dependency::Image, toImage(string(get("color")))->dependencies);
}

TEST(ExpressionDependencies, Compare) {
    EXPECT_EQ(Dependency::Feature, eq(string(get("color")), string(get("color")))->dependencies);
    EXPECT_EQ(Dependency::Feature, lt(string(get("color")), string(get("color")))->dependencies);
    EXPECT_EQ(Dependency::Feature, gt(string(get("color")), string(get("color")))->dependencies);
    EXPECT_EQ(Dependency::Feature, eq(string(get("color")), string(get("color")))->dependencies);
}

TEST(ExpressionDependencies, Interpolate) {
    EXPECT_EQ(Dependency::Feature, step(number(id()), literal(0.0), 0.0, literal(0.0))->dependencies);
    EXPECT_EQ(Dependency::Feature, step(literal(0.0), number(id()), 0.0, literal(0.0))->dependencies);
    EXPECT_EQ(Dependency::Feature, step(literal(0.0), literal(0.0), 0.0, number(id()))->dependencies);

    interpolate(linear(), number(id()), 0.0, literal(0.0), 0.0, literal(0.0), 0.0, literal(0.0));
    interpolate(linear(), literal(0.0), 0.0, number(id()), 0.0, literal(0.0), 0.0, literal(0.0));
    interpolate(linear(), literal(0.0), 0.0, literal(0.0), 0.0, number(id()), 0.0, literal(0.0));
    interpolate(linear(), literal(0.0), 0.0, literal(0.0), 0.0, literal(0.0), 0.0, number(id()));
}

TEST(ExpressionDependencies, Image) {
    EXPECT_EQ(Dependency::Image, image(literal("airport-11"))->dependencies);
    EXPECT_EQ(Dependency::Image | Dependency::Feature, image(get(literal("image_name"s)))->dependencies);
}

TEST(ExpressionDependencies, Distance) {
    const std::string pointGeoSource = R"({
         "type": "Point",
         "coordinates":[24.938492774963375,60.16980226227959]})";
    const auto exprStr = R"(["distance", )" + pointGeoSource + R"( ])";
    const auto expression = createExpression(exprStr.c_str());
    EXPECT_EQ(Dependency::Feature, expression->dependencies);
}
