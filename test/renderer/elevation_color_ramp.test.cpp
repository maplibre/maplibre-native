#include <mln/test/util.hpp>

#include <mln/renderer/layers/fill_extrusion_layer_tweaker.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/property_value.hpp>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

namespace {

PropertyExpression<Color> parseColor(const char* json) {
    Error error;
    auto value = convertJSON<PropertyValue<Color>>(json, error, false, false, true);
    EXPECT_TRUE(value && value->isExpression()) << error.message;
    return value->asExpression();
}

const char* const ramp = R"(["interpolate", ["linear"], ["elevation"], 0, "black", 10, "white"])";

} // namespace

TEST(ElevationColorRamp, ZoomConstant) {
    const auto color = parseColor(ramp);
    const auto atZoom14 = elevationColorRampZoom(color, 14);
    EXPECT_EQ(0.0f, atZoom14.t);
    // Zoom alone never requires re-sampling
    EXPECT_TRUE(atZoom14.sameRows(elevationColorRampZoom(color, 17.5f)));
}

TEST(ElevationColorRamp, ZoomInterpolate) {
    const auto color = parseColor(
        R"(["interpolate", ["linear"], ["zoom"], 14, ["interpolate", ["linear"], ["elevation"], 0, "black", 10, "white"],
                                                 18, ["interpolate", ["linear"], ["elevation"], 0, "red", 10, "blue"]])");

    // Between the stops: rows at 14 and 18, blended on the GPU
    const auto atZoom16 = elevationColorRampZoom(color, 16);
    EXPECT_EQ(14.0f, atZoom16.rows.min);
    EXPECT_EQ(18.0f, atZoom16.rows.max);
    EXPECT_FLOAT_EQ(0.5f, atZoom16.t);
    // ... so zooming between them only changes the blend
    const auto atZoom17 = elevationColorRampZoom(color, 17);
    EXPECT_TRUE(atZoom16.sameRows(atZoom17));
    EXPECT_FLOAT_EQ(0.75f, atZoom17.t);

    // Outside the stops the ramp is clamped to the nearest one
    const auto atZoom12 = elevationColorRampZoom(color, 12);
    EXPECT_EQ(14.0f, atZoom12.rows.min);
    EXPECT_EQ(0.0f, atZoom12.t);
    EXPECT_FALSE(atZoom12.sameRows(atZoom16));
    const auto atZoom20 = elevationColorRampZoom(color, 20);
    EXPECT_EQ(18.0f, atZoom20.rows.min);
    EXPECT_EQ(0.0f, atZoom20.t);
}

TEST(ElevationColorRamp, ZoomStep) {
    const auto color = parseColor(
        R"(["step", ["zoom"], ["interpolate", ["linear"], ["elevation"], 0, "black", 10, "white"],
                    16, ["interpolate", ["linear"], ["elevation"], 0, "red", 10, "blue"]])");

    // Constant between stops: sampled at the current zoom, re-sampled only when crossing a stop
    const auto atZoom14 = elevationColorRampZoom(color, 14);
    EXPECT_EQ(14.0f, atZoom14.rows.min);
    EXPECT_EQ(0.0f, atZoom14.t);
    EXPECT_TRUE(atZoom14.sameRows(elevationColorRampZoom(color, 15.5f)));
    EXPECT_FALSE(atZoom14.sameRows(elevationColorRampZoom(color, 16.5f)));
    EXPECT_TRUE(elevationColorRampZoom(color, 16.5f).sameRows(elevationColorRampZoom(color, 19)));
}
