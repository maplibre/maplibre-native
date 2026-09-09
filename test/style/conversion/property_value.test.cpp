#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/util/rapidjson.hpp>
#include <mln/test/stub_geometry_tile_feature.hpp>
#include <mln/style/variable_anchor_offset_collection.hpp>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

TEST(StyleConversion, PropertyValue) {
    // PropertyValue<T> accepts a constant expression:
    // https://github.com/mapbox/mapbox-gl-native/issues/11940
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"(["literal", [1, 2]])");
    auto expected = std::array<float, 2>{{1, 2}};
    auto result = convert<PropertyValue<std::array<float, 2>>>(doc, error, false, false);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->isConstant());
    ASSERT_EQ(result->asConstant(), expected);
}

TEST(StyleConversion, SemiliteralOffsets) {
    Error error;
    JSDocument doc;
    doc.Parse(R"(["semiliteral", [["number", ["get", "x"]], ["*", 2, ["get", "y"]]]])");
    auto result = convert<PropertyValue<std::array<float, 2>>>(doc, error, true, false);
    ASSERT_TRUE(result) << error.message;
    ASSERT_TRUE(result->isExpression());
    StubGeometryTileFeature feature{PropertyMap{{"x", 3.0}, {"y", 4.0}}};
    EXPECT_EQ(result->asExpression().evaluate(feature, {}), (std::array<float, 2>{3, 8}));

    doc.Parse(
        R"(["semiliteral", ["top", ["semiliteral", [["number", ["get", "x"]], 2]], "bottom", ["literal", [0, 1]]]])");
    auto anchors = convert<PropertyValue<VariableAnchorOffsetCollection>>(doc, error, true, false);
    ASSERT_TRUE(anchors) << error.message;
    ASSERT_TRUE(anchors->isExpression());
    auto evaluated = anchors->asExpression().evaluate(feature, {});
    ASSERT_EQ(evaluated.size(), 2u);
    EXPECT_EQ(evaluated[0].anchorType, SymbolAnchorType::Top);
    EXPECT_EQ(evaluated[0].offset, (std::array<float, 2>{3, 2}));
    EXPECT_EQ(evaluated[1].anchorType, SymbolAnchorType::Bottom);
    EXPECT_EQ(evaluated[1].offset, (std::array<float, 2>{0, 1}));
}
