#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/projection.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/projection_impl.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/util/rapidjson.hpp>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

TEST(StyleConversion, Projection) {
    Error error;

    auto parseProjection = [&](const std::string& src) {
        return convertJSON<Projection>(src, error);
    };

    {
        auto projection = parseProjection("{}");
        ASSERT_TRUE(projection);
        ASSERT_TRUE(projection->getType().isUndefined());
        ASSERT_EQ(ProjectionDefinition("mercator"), projection->impl->evaluate(3.f));
    }

    {
        auto projection = parseProjection(R"({"type":"globe"})");
        ASSERT_TRUE(projection);
        ASSERT_TRUE(projection->getType().isConstant());
        ASSERT_EQ(ProjectionDefinition("globe"), projection->getType().asConstant());
        ASSERT_EQ(ProjectionDefinition("globe", "globe", 1), projection->impl->evaluate(3.f));
    }

    {
        auto projection = parseProjection(R"({"type":["vertical-perspective","mercator",0.25]})");
        ASSERT_TRUE(projection);
        ASSERT_TRUE(projection->getType().isConstant());
        ASSERT_EQ(ProjectionDefinition("vertical-perspective", "mercator", 0.25), projection->getType().asConstant());
    }

    {
        auto projection = parseProjection(
            R"({"type":["interpolate",["linear"],["zoom"],10,"vertical-perspective",12,"mercator"]})");
        ASSERT_TRUE(projection);
        ASSERT_TRUE(projection->getType().isExpression());
        ASSERT_EQ(ProjectionDefinition("vertical-perspective", "vertical-perspective", 1),
                  projection->impl->evaluate(9.f));
        ASSERT_EQ(ProjectionDefinition("vertical-perspective", "mercator", 0.5), projection->impl->evaluate(11.f));
        ASSERT_EQ(ProjectionDefinition("mercator", "mercator", 1), projection->impl->evaluate(13.f));
    }

    {
        auto projection = parseProjection("{}");
        ASSERT_TRUE(projection);
        const mln::JSValue typeValue("globe");
        ASSERT_FALSE(projection->setProperty("type", &typeValue));
        ASSERT_EQ(ProjectionDefinition("globe"), projection->getType().asConstant());
        ASSERT_TRUE(projection->setProperty("bogus", &typeValue));
        ASSERT_EQ(StyleProperty::Kind::Constant, projection->getProperty("type").getKind());
        ASSERT_EQ(mln::Value("globe"), projection->getProperty("type").getValue());
    }

    {
        ASSERT_FALSE(parseProjection("3"));
        ASSERT_EQ("projection must be an object", error.message);
    }

    {
        ASSERT_FALSE(parseProjection(R"({"type":3})"));
        ASSERT_EQ("value must be a string or an array of [from, to, transition]", error.message);
    }
}
