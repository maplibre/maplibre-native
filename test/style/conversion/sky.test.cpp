#include <mln/test/util.hpp>

#include <mln/renderer/property_evaluation_parameters.hpp>
#include <mln/renderer/render_sky.hpp>
#include <mln/renderer/transition_parameters.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/sky.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/sky_impl.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/color.hpp>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

namespace {

std::optional<Sky> parseSky(const std::string& source, Error& error) {
    return convertJSON<Sky>(source, error);
}

} // namespace

TEST(StyleConversion, SkyDefaults) {
    Sky sky;
    RenderSky renderSky(sky.impl);
    renderSky.evaluate(PropertyEvaluationParameters(0.0f));

    EXPECT_EQ((Color{136.0f / 255.0f, 198.0f / 255.0f, 252.0f / 255.0f, 1.0f}), Sky::getDefaultSkyColor());
    EXPECT_EQ(Color::white(), Sky::getDefaultHorizonColor());
    EXPECT_EQ(Color::white(), Sky::getDefaultFogColor());
    EXPECT_FLOAT_EQ(0.5f, Sky::getDefaultFogGroundBlend());
    EXPECT_FLOAT_EQ(0.8f, Sky::getDefaultHorizonFogBlend());
    EXPECT_FLOAT_EQ(0.8f, Sky::getDefaultSkyHorizonBlend());
    EXPECT_FLOAT_EQ(0.8f, Sky::getDefaultAtmosphereBlend());

    const auto& evaluated = renderSky.getEvaluated();
    EXPECT_EQ(Sky::getDefaultSkyColor(), evaluated.get<SkyColor>());
    EXPECT_EQ(Color::white(), evaluated.get<SkyHorizonColor>());
    EXPECT_EQ(Color::white(), evaluated.get<SkyFogColor>());
    EXPECT_FLOAT_EQ(0.5f, evaluated.get<SkyFogGroundBlend>());
    EXPECT_FLOAT_EQ(0.8f, evaluated.get<SkyHorizonFogBlend>());
    EXPECT_FLOAT_EQ(0.8f, evaluated.get<SkyHorizonBlend>());
    EXPECT_FLOAT_EQ(0.8f, evaluated.get<SkyAtmosphereBlend>());
}

TEST(StyleConversion, SkySetAndGet) {
    Sky sky;
    sky.setSkyColor(Color::red());
    sky.setHorizonColor(Color::green());
    sky.setFogColor(Color::blue());
    sky.setFogGroundBlend(0.1f);
    sky.setHorizonFogBlend(0.2f);
    sky.setSkyHorizonBlend(0.3f);
    sky.setAtmosphereBlend(0.4f);

    EXPECT_EQ(Color::red(), sky.getSkyColor().asConstant());
    EXPECT_EQ(Color::green(), sky.getHorizonColor().asConstant());
    EXPECT_EQ(Color::blue(), sky.getFogColor().asConstant());
    EXPECT_FLOAT_EQ(0.1f, sky.getFogGroundBlend().asConstant());
    EXPECT_FLOAT_EQ(0.2f, sky.getHorizonFogBlend().asConstant());
    EXPECT_FLOAT_EQ(0.3f, sky.getSkyHorizonBlend().asConstant());
    EXPECT_FLOAT_EQ(0.4f, sky.getAtmosphereBlend().asConstant());
    EXPECT_EQ(StyleProperty::Kind::Expression, sky.getProperty("sky-color").getKind());
    EXPECT_EQ(StyleProperty::Kind::Undefined, sky.getProperty("unknown").getKind());
}

TEST(StyleConversion, SkyConstantsAndTransitions) {
    Error error;
    auto sky = parseSky(R"({
        "sky-color": "#123456",
        "horizon-color": "red",
        "fog-color": "blue",
        "fog-ground-blend": 0.1,
        "horizon-fog-blend": 0.2,
        "sky-horizon-blend": 0.3,
        "atmosphere-blend": 0.4,
        "sky-color-transition": {"duration": 1000, "delay": 20}
    })",
                        error);
    ASSERT_TRUE(sky);

    EXPECT_EQ(Color::red(), sky->getHorizonColor().asConstant());
    EXPECT_EQ(Color::blue(), sky->getFogColor().asConstant());
    EXPECT_FLOAT_EQ(0.1f, sky->getFogGroundBlend().asConstant());
    EXPECT_FLOAT_EQ(0.2f, sky->getHorizonFogBlend().asConstant());
    EXPECT_FLOAT_EQ(0.3f, sky->getSkyHorizonBlend().asConstant());
    EXPECT_FLOAT_EQ(0.4f, sky->getAtmosphereBlend().asConstant());
    EXPECT_EQ(Milliseconds(1000), sky->getSkyColorTransition().duration);
    EXPECT_EQ(Milliseconds(20), sky->getSkyColorTransition().delay);

    Sky transitionSource;
    transitionSource.setAtmosphereBlend(0.4f);
    RenderSky transitioning(transitionSource.impl);
    auto changed = transitionSource.mutableImpl();
    changed->properties.get<SkyAtmosphereBlend>().value = 0.9f;
    transitioning.impl = std::move(changed);
    const auto start = TimePoint(Duration::zero());
    transitioning.transition({start, {Milliseconds(100)}});
    EXPECT_TRUE(transitioning.hasTransition());

    auto evaluation = PropertyEvaluationParameters(0.0f);
    evaluation.now = start;
    transitioning.evaluate(evaluation);
    EXPECT_FLOAT_EQ(0.4f, transitioning.getEvaluated().get<SkyAtmosphereBlend>());

    evaluation.now = start + Milliseconds(100);
    transitioning.evaluate(evaluation);
    EXPECT_FLOAT_EQ(0.9f, transitioning.getEvaluated().get<SkyAtmosphereBlend>());
    EXPECT_FALSE(transitioning.hasTransition());
}

TEST(StyleConversion, SkyZoomExpressions) {
    Error error;
    auto sky = parseSky(R"({
        "atmosphere-blend": ["interpolate", ["linear"], ["zoom"], 0, 0, 10, 1],
        "sky-color": ["interpolate", ["linear"], ["zoom"], 0, "black", 10, "white"]
    })",
                        error);
    ASSERT_TRUE(sky) << error.message;
    ASSERT_TRUE(sky->getAtmosphereBlend().isExpression());
    ASSERT_FALSE(sky->getAtmosphereBlend().asExpression().isZoomConstant());

    RenderSky renderSky(sky->impl);
    renderSky.evaluate(PropertyEvaluationParameters(5.0f));
    EXPECT_FLOAT_EQ(0.5f, renderSky.getEvaluated().get<SkyAtmosphereBlend>());
    EXPECT_EQ((Color{0.5f, 0.5f, 0.5f, 1.0f}), renderSky.getEvaluated().get<SkyColor>());

    renderSky.evaluate(PropertyEvaluationParameters(10.0f));
    EXPECT_FLOAT_EQ(1.0f, renderSky.getEvaluated().get<SkyAtmosphereBlend>());
    EXPECT_EQ(Color::white(), renderSky.getEvaluated().get<SkyColor>());
}

TEST(StyleConversion, InvalidSky) {
    Error error;

    EXPECT_FALSE(parseSky("[]", error));
    EXPECT_EQ("sky must be an object", error.message);

    EXPECT_FALSE(parseSky(R"({"atmosphere-blend":"opaque"})", error));
    EXPECT_EQ("value must be a number", error.message);

    EXPECT_FALSE(parseSky(R"({"sky-color":5})", error));
    EXPECT_EQ("value must be a string", error.message);
}
