#include <gtest/gtest.h>

#include <mbgl/style/sources/contour_source.hpp>

using namespace mbgl;
using namespace mbgl::style;

// ContourSource is a "source-on-source" — it doesn't fetch tiles itself but
// derives contour-line features from another source (a `raster-dem`) by
// reference. T2.1 establishes the type with full configuration storage; the
// JSON parser (T2.3) and the render-source integration (Task 3) come later.

namespace {

ContourSourceOptions fullConfig() {
    ContourSourceOptions opts;
    opts.sourceID = "dem";
    opts.intervals.stops = {200.0, 12.0, 100.0, 14.0, 50.0, 15.0, 20.0};
    opts.unit.unit = algorithm::contour::ContourUnit::Feet;
    opts.unit.customMultiplier = 1.0;
    opts.majorMultiplier = 5;
    opts.overzoom = 1;
    return opts;
}

} // namespace

TEST(ContourSource, ConstructorStoresIDAndType) {
    ContourSource source("contours", fullConfig());
    EXPECT_EQ(source.getID(), "contours");
    EXPECT_EQ(source.getType(), SourceType::Contour);
    EXPECT_TRUE(source.is<ContourSource>());
}

TEST(ContourSource, GettersReturnConfiguredValues) {
    const auto cfg = fullConfig();
    ContourSource source("contours", cfg);
    EXPECT_EQ(source.getDEMSourceID(), "dem");
    EXPECT_EQ(source.getIntervals().stops, cfg.intervals.stops);
    EXPECT_EQ(source.getUnit().unit, algorithm::contour::ContourUnit::Feet);
    EXPECT_EQ(source.getMajorMultiplier(), 5);
    EXPECT_EQ(source.getOverzoom(), 1);
}

TEST(ContourSource, DefaultOptionsReflectSpecDefaults) {
    // Spec defaults: unit "meters", majorMultiplier 5, overzoom 0.
    ContourSourceOptions opts;
    opts.sourceID = "dem";
    opts.intervals.stops = {100.0};
    ContourSource source("contours", opts);
    EXPECT_EQ(source.getUnit().unit, algorithm::contour::ContourUnit::Meters);
    EXPECT_EQ(source.getMajorMultiplier(), 5);
    EXPECT_EQ(source.getOverzoom(), 0);
}

TEST(ContourSource, IsCanCastDownFromBase) {
    ContourSource source("c", fullConfig());
    Source& base = source;
    ASSERT_NE(base.as<ContourSource>(), nullptr);
    EXPECT_EQ(base.as<ContourSource>()->getDEMSourceID(), "dem");
}

TEST(ContourSource, AttributionIsEmpty) {
    // ContourSource emits derived features rather than fetching tiles, so it
    // has no source-of-its-own attribution; any DEM attribution comes from
    // the upstream raster-dem source.
    ContourSource source("c", fullConfig());
    EXPECT_FALSE(source.getAttribution().has_value());
}
