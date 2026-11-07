#include <mln/storage/resource_options.hpp>
#include <mln/util/chrono.hpp>

#include <gtest/gtest.h>

using namespace mln;

TEST(ResourceOptions, ImplicitCacheLifetime) {
    const Duration lifetime = std::chrono::hours(24);
    const Duration oneHour = std::chrono::hours(1);

    EXPECT_EQ(Duration::zero(), ResourceOptions().implicitCacheLifetime());
    EXPECT_EQ(Duration::zero(), ResourceOptions::Default().implicitCacheLifetime());

    ResourceOptions options;
    EXPECT_EQ(&options, &options.withApiKey("token").withCachePath("cache.db").withImplicitCacheLifetime(lifetime));
    EXPECT_EQ(lifetime, options.implicitCacheLifetime());
    EXPECT_EQ("token", options.apiKey());
    EXPECT_EQ("cache.db", options.cachePath());

    ResourceOptions cloned = options.clone();
    EXPECT_EQ(lifetime, cloned.implicitCacheLifetime());
    EXPECT_EQ(options.apiKey(), cloned.apiKey());
    EXPECT_EQ(options.cachePath(), cloned.cachePath());

    cloned.withImplicitCacheLifetime(oneHour);
    EXPECT_EQ(lifetime, options.implicitCacheLifetime());
    EXPECT_EQ(oneHour, cloned.implicitCacheLifetime());

    ResourceOptions assigned;
    assigned.withImplicitCacheLifetime(std::chrono::hours(3)).withApiKey("replaced");
    assigned = options;
    EXPECT_EQ(lifetime, assigned.implicitCacheLifetime());
    EXPECT_EQ(options.apiKey(), assigned.apiKey());
    EXPECT_EQ(options.cachePath(), assigned.cachePath());

    assigned.withImplicitCacheLifetime(Duration::zero());
    EXPECT_EQ(Duration::zero(), assigned.implicitCacheLifetime());

    const Duration negative = std::chrono::seconds(-5);
    options.withImplicitCacheLifetime(negative);
    EXPECT_EQ(negative, options.implicitCacheLifetime());
}
