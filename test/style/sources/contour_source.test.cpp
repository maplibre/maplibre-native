#include <gtest/gtest.h>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/source.hpp>
#include <mbgl/style/sources/contour_source.hpp>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

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

// --- JSON parser ---------------------------------------------------------
//
// The contour source-spec block (maplibre-style-spec#583) looks like:
//
//   "contours": {
//     "type": "contour",
//     "source": "dem",
//     "intervals": [200, 12, 100, 14, 50, 15, 20],
//     "unit": "feet",         // optional, default "meters"
//     "majorMultiplier": 5,   // optional, default 5
//     "overzoom": 1           // optional, default 0
//   }
//
// `convert<unique_ptr<Source>>(value, error, id)` takes the inner block plus
// the source ID (the JSON object key in the parent `sources`).

namespace {

std::unique_ptr<ContourSource> parseContour(const std::string& json, Error& error, const std::string& id = "contours") {
    auto opt = convertJSON<std::unique_ptr<Source>>(json, error, id);
    if (!opt) return nullptr;
    std::unique_ptr<Source> base = std::move(*opt);
    auto* cs = base->as<ContourSource>();
    if (!cs) return nullptr;
    (void)base.release();
    return std::unique_ptr<ContourSource>(cs);
}

} // namespace

TEST(ContourSourceParse, FullySpecifiedBlockParses) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour",
        "source": "dem",
        "intervals": [200, 12, 100, 14, 50, 15, 20],
        "unit": "feet",
        "majorMultiplier": 5,
        "overzoom": 1
    })JSON",
                            error);
    ASSERT_TRUE(src) << error.message;
    EXPECT_EQ(src->getID(), "contours");
    EXPECT_EQ(src->getDEMSourceID(), "dem");
    EXPECT_EQ(src->getIntervals().stops, (std::vector<double>{200, 12, 100, 14, 50, 15, 20}));
    EXPECT_EQ(src->getUnit().unit, algorithm::contour::ContourUnit::Feet);
    EXPECT_EQ(src->getMajorMultiplier(), 5u);
    EXPECT_EQ(src->getOverzoom(), 1u);
}

TEST(ContourSourceParse, MinimalBlockUsesDefaults) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour",
        "source": "dem",
        "intervals": [100]
    })JSON",
                            error);
    ASSERT_TRUE(src) << error.message;
    EXPECT_EQ(src->getDEMSourceID(), "dem");
    EXPECT_EQ(src->getUnit().unit, algorithm::contour::ContourUnit::Meters);
    EXPECT_EQ(src->getMajorMultiplier(), 5u);
    EXPECT_EQ(src->getOverzoom(), 0u);
}

TEST(ContourSourceParse, MetersUnitParses) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour", "source": "dem", "intervals": [100], "unit": "meters"
    })JSON",
                            error);
    ASSERT_TRUE(src) << error.message;
    EXPECT_EQ(src->getUnit().unit, algorithm::contour::ContourUnit::Meters);
}

TEST(ContourSourceParse, NumericUnitBecomesCustomMultiplier) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour", "source": "dem", "intervals": [100], "unit": 0.5
    })JSON",
                            error);
    ASSERT_TRUE(src) << error.message;
    EXPECT_EQ(src->getUnit().unit, algorithm::contour::ContourUnit::Custom);
    EXPECT_DOUBLE_EQ(src->getUnit().customMultiplier, 0.5);
}

TEST(ContourSourceParse, MissingSourceIsRejected) {
    Error error;
    auto src = parseContour(R"JSON({"type": "contour", "intervals": [100]})JSON", error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("source"), std::string::npos)
        << "expected error to mention `source`, got: " << error.message;
}

TEST(ContourSourceParse, NonStringSourceIsRejected) {
    Error error;
    auto src = parseContour(R"JSON({"type": "contour", "source": 42, "intervals": [100]})JSON", error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("source"), std::string::npos);
}

TEST(ContourSourceParse, MissingIntervalsIsRejected) {
    Error error;
    auto src = parseContour(R"JSON({"type": "contour", "source": "dem"})JSON", error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("intervals"), std::string::npos)
        << "expected error to mention `intervals`, got: " << error.message;
}

TEST(ContourSourceParse, EmptyIntervalsArrayIsRejected) {
    Error error;
    auto src = parseContour(R"JSON({"type": "contour", "source": "dem", "intervals": []})JSON", error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("intervals"), std::string::npos);
}

TEST(ContourSourceParse, EvenLengthIntervalsArrayIsRejected) {
    Error error;
    auto src = parseContour(R"JSON({"type": "contour", "source": "dem", "intervals": [200, 12]})JSON", error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("intervals"), std::string::npos);
}

TEST(ContourSourceParse, NonPositiveIntervalRejected) {
    Error error;
    auto src = parseContour(R"JSON({"type": "contour", "source": "dem", "intervals": [0]})JSON", error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("intervals"), std::string::npos);
}

TEST(ContourSourceParse, BadUnitStringRejected) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour", "source": "dem", "intervals": [100], "unit": "fortnights"
    })JSON",
                            error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("unit"), std::string::npos);
}

TEST(ContourSourceParse, NonPositiveCustomUnitRejected) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour", "source": "dem", "intervals": [100], "unit": 0
    })JSON",
                            error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("unit"), std::string::npos);
}

TEST(ContourSourceParse, NegativeOverzoomRejected) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour", "source": "dem", "intervals": [100], "overzoom": -1
    })JSON",
                            error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("overzoom"), std::string::npos);
}

TEST(ContourSourceParse, ZeroMajorMultiplierRejected) {
    Error error;
    auto src = parseContour(R"JSON({
        "type": "contour", "source": "dem", "intervals": [100], "majorMultiplier": 0
    })JSON",
                            error);
    EXPECT_FALSE(src);
    EXPECT_NE(error.message.find("majorMultiplier"), std::string::npos);
}
