#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/util/rapidjson.hpp>

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
