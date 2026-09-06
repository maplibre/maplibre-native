#include <mln/test/fixture_log_observer.hpp>
#include <mln/test/util.hpp>

#include <mln/renderer/buckets/fill_bucket.hpp>

#include <cstddef>
#include <string>

using namespace mln;

TEST(FillBucket, TracksSDFPatternsPerLayer) {
    FixtureLog log;
    FillBucket::PossiblyEvaluatedLayoutProperties layout;
    FillBucket bucket{layout, {}, 5.0f, 1};

    bucket.recordSDFPattern("sdf-layer", true);
    bucket.recordSDFPattern("rgba-layer", false);

    EXPECT_TRUE(bucket.isSDFPattern("sdf-layer"));
    EXPECT_FALSE(bucket.isSDFPattern("rgba-layer"));
    EXPECT_FALSE(bucket.isSDFPattern("missing-layer"));
    EXPECT_TRUE(log.empty());
}

TEST(FillBucket, WarnsOnceForMixedPatternTypesAcrossBuckets) {
    static std::size_t invocation = 0;
    const std::string layerID = "mixed-layer-" + std::to_string(invocation++);

    FixtureLog log;
    FillBucket::PossiblyEvaluatedLayoutProperties layout;
    FillBucket firstBucket{layout, {}, 5.0f, 1};
    FillBucket secondBucket{layout, {}, 5.0f, 1};

    firstBucket.recordSDFPattern(layerID, true);
    firstBucket.recordSDFPattern(layerID, false);
    secondBucket.recordSDFPattern(layerID, true);
    secondBucket.recordSDFPattern(layerID, false);

    const FixtureLog::Message warning{
        EventSeverity::Warning,
        Event::Style,
        -1,
        "Style sheet warning: Cannot mix SDF and non-SDF fill patterns in layer \"" + layerID + "\""};
    EXPECT_EQ(1u, log.count(warning));
}
