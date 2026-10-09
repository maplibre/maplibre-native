#include <mln/test/fixture_log_observer.hpp>
#include <mln/test/util.hpp>

#include <mln/renderer/buckets/fill_bucket.hpp>

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

TEST(FillBucket, WarnsOnceForMixedPatternTypesPerBucket) {
    FixtureLog log;
    FillBucket::PossiblyEvaluatedLayoutProperties layout;
    FillBucket firstBucket{layout, {}, 5.0f, 1};
    FillBucket secondBucket{layout, {}, 5.0f, 1};

    firstBucket.recordSDFPattern("mixed-layer", true);
    firstBucket.recordSDFPattern("mixed-layer", false);
    firstBucket.recordSDFPattern("mixed-layer", false);
    secondBucket.recordSDFPattern("mixed-layer", true);
    secondBucket.recordSDFPattern("mixed-layer", false);

    const FixtureLog::Message warning{
        EventSeverity::Warning,
        Event::Style,
        -1,
        "Style sheet warning: Cannot mix SDF and non-SDF fill patterns in layer \"mixed-layer\""};
    EXPECT_EQ(2u, log.count(warning));
}
