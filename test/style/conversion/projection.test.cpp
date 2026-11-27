#include <mln/test/crs.hpp>
#include <mln/test/fixture_log_observer.hpp>
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

TEST(StyleConversion, RegisteredProjection) {
    FixtureLog log;
    Error error;
    auto parseProjection = [&](const std::string& src) {
        return convertJSON<Projection>(src, error);
    };

    for (const std::string name : {"identity", "EPSG:3413"}) {
        auto projection = parseProjection(R"({"type":")" + name + R"("})");
        ASSERT_TRUE(projection) << error.message;
        EXPECT_EQ(name == "identity", projection->impl->registeredProjection != nullptr);
        EXPECT_EQ(StyleProperty::Kind::Constant, projection->getProperty("type").getKind());
        EXPECT_EQ(mln::Value(name), projection->getProperty("type").getValue());

        const mln::JSValue mercator("mercator");
        EXPECT_FALSE(projection->setProperty("type", &mercator));
        EXPECT_EQ(nullptr, projection->impl->registeredProjection);
    }
    EXPECT_EQ(1u,
              log.count({EventSeverity::Warning,
                         Event::General,
                         -1,
                         "Unknown projection name \"EPSG:3413\". Falling back to mercator projection."}));

    EXPECT_FALSE(parseProjection(R"({"type":"equal-earth"})"));
}

TEST(StyleConversion, ProjectionExpressionDrawsAsMercator) {
    FixtureLog log;
    Error error;

    // Without the globe, every projection an expression or a transition can choose draws as Mercator. A registered one
    // only works as a plain name.
    const test::ScopedProjection registration(test::polarStereographic("test:in-a-transition"));
    for (const std::string type : {R"(["step",["zoom"],"identity",10,"mercator"])",
                                   R"(["mercator","test:in-a-transition",0.25])",
                                   R"(["interpolate",["linear"],["zoom"],10,"vertical-perspective",12,"mercator"])"}) {
        auto projection = convertJSON<Projection>(R"({"type":)" + type + "}", error);
        ASSERT_TRUE(projection) << error.message;
        EXPECT_EQ(StyleProperty::Kind::Undefined, projection->getProperty("type").getKind());
        EXPECT_EQ(nullptr, projection->impl->registeredProjection);
    }

    EXPECT_EQ(3u,
              log.count({EventSeverity::Warning,
                         Event::Style,
                         -1,
                         "Projection expressions are not supported yet, so the style's projection draws as Mercator"}));
    EXPECT_EQ(0u, log.uncheckedCount()) << log.unchecked();
}

TEST(StyleConversion, Projection) {
    FixtureLog log;
    Error error;

    auto parseProjection = [&](const std::string& src) {
        return convertJSON<Projection>(src, error);
    };

    {
        auto projection = parseProjection("{}");
        ASSERT_TRUE(projection);
        EXPECT_EQ(StyleProperty::Kind::Undefined, projection->getProperty("type").getKind());
    }

    for (const std::string type : {"mercator", "vertical-perspective", "globe"}) {
        auto projection = parseProjection(R"({"type":")" + type + R"("})");
        ASSERT_TRUE(projection) << error.message;
        EXPECT_EQ(StyleProperty::Kind::Constant, projection->getProperty("type").getKind());
        EXPECT_EQ(mln::Value(type), projection->getProperty("type").getValue());
        EXPECT_EQ(nullptr, projection->impl->registeredProjection);
    }
    EXPECT_EQ(2u,
              log.count({EventSeverity::Warning,
                         Event::Style,
                         -1,
                         "The globe projection is not supported yet, so the style's projection draws as Mercator"}));

    {
        auto projection = parseProjection("{}");
        ASSERT_TRUE(projection);
        const mln::JSValue typeValue("mercator");
        ASSERT_FALSE(projection->setProperty("type", &typeValue));
        ASSERT_TRUE(projection->setProperty("bogus", &typeValue));
        ASSERT_EQ(StyleProperty::Kind::Constant, projection->getProperty("type").getKind());
        ASSERT_EQ(mln::Value("mercator"), projection->getProperty("type").getValue());
    }

    {
        ASSERT_FALSE(parseProjection(R"({"type":"orthographic"})"));
        ASSERT_EQ("projection type must be mercator, vertical-perspective, globe, identity or a name with a colon",
                  error.message);
    }

    {
        ASSERT_FALSE(parseProjection("3"));
        ASSERT_EQ("projection must be an object", error.message);
    }

    {
        ASSERT_FALSE(parseProjection(R"({"type":3})"));
        ASSERT_EQ("value must be a string or an array of [from, to, transition]", error.message);
    }
    EXPECT_EQ(0u, log.uncheckedCount()) << log.unchecked();
}
