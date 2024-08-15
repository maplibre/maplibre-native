#include <mbgl/util/lru_cache.hpp>

#include <gtest/gtest.h>

#include <stdexcept>

using namespace mbgl;

TEST(LRU, LRU) {
    LRU<int> lru;
    EXPECT_TRUE(lru.empty());

    lru.touch(1);
    lru.touch(2);
    lru.touch(3);
    EXPECT_FALSE(lru.empty());
    EXPECT_EQ(lru.size(), 3);

    lru.remove(2);
    lru.remove(77);
    EXPECT_EQ(lru.evict(), 1);
    EXPECT_FALSE(lru.empty());
    EXPECT_EQ(lru.size(), 1);

    lru.touch(4);
    EXPECT_EQ(lru.evict(), 3);
    EXPECT_FALSE(lru.empty());
    EXPECT_EQ(lru.size(), 1);

    EXPECT_FALSE(lru.isHit(3));
    EXPECT_FALSE(lru.isHit(33));
    EXPECT_TRUE(lru.isHit(4));

    EXPECT_EQ(lru.evict(), 4);
    EXPECT_TRUE(lru.empty());
    EXPECT_FALSE(lru.isHit(4));
    EXPECT_THROW(lru.evict(), std::runtime_error);
}
