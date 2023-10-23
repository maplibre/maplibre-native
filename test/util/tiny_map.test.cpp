#include <mbgl/test/util.hpp>

#include <mbgl/util/tiny_map.hpp>

using namespace mbgl;

struct TinyMap : public testing::TestWithParam<bool> {};

template <typename TMapIter, typename TMap2>
void testSetDiff(TMapIter beg, TMapIter end, TMap2& map2) {
    while (beg != end) {
        auto item = *beg++;
        auto i = map2.find(item.first);
        EXPECT_NE(i, map2.end());
        EXPECT_EQ(i->second, item.second);
    }
}
template <typename TMap1, typename TMap2>
void testSetDiff(TMap1 map1, TMap2& map2) {
    testSetDiff(map1.begin(), map1.end(), map2);
    testSetDiff(map2.begin(), map2.end(), map1);
}

TEST_P(TinyMap, Init) {
    // Construct from separate initializer lists
    const auto map = util::TinyMap<int, int>{GetParam(), {1, 2, 3}, {3, 2, 1}};
    EXPECT_EQ(3, map.size());
    EXPECT_LT(1 << 16, map.max_size());

    // Construct from list of pairs
    const auto map2 = util::TinyMap<int, int>{GetParam(), {{1, 3}, {2, 2}, {3, 1}}};
    EXPECT_EQ(map.size(), map2.size());
    testSetDiff(map, map2);

    // Construct in a different order
    const auto map3 = util::TinyMap<int, int>{GetParam(), {2, 3, 1}, {2, 1, 3}};
    testSetDiff(map, map3);
}

TEST_P(TinyMap, Copy) {
    const auto map = util::TinyMap<int, int>{GetParam(), {1, 2, 3}, {3, 2, 1}};
    decltype(map) map2(map);

    std::remove_const_t<decltype(map)> map3(GetParam());
    map3 = map2;

    testSetDiff(map, map2);
    testSetDiff(map, map3);
}

TEST_P(TinyMap, Move) {
    const auto map = util::TinyMap<int, int>{GetParam(), {1, 2, 3}, {3, 2, 1}};
    std::remove_const_t<decltype(map)> map2(map);

    // move constructor
    decltype(map2) map3(std::move(map2));
    EXPECT_TRUE(map2.empty());
    testSetDiff(map, map3);

    // move assignment
    decltype(map2) map4(GetParam());
    map4 = std::move(map3);
    EXPECT_TRUE(map3.empty());
    testSetDiff(map, map4);
}

TEST_P(TinyMap, ConstLookup) {
    const auto map = util::TinyMap<int, int>{GetParam(), {2, 4, 6}, {3, 2, 1}};

    EXPECT_EQ(3, map.find(2)->second);
    EXPECT_EQ(2, map.find(4)->second);
    EXPECT_EQ(1, map.find(6)->second);
}

TEST_P(TinyMap, MutableLookup) {
    auto map = util::TinyMap<int, int>{GetParam(), {2, 4, 6}, {3, 2, 1}};

    EXPECT_EQ(3, map.find(2)->second);
    EXPECT_EQ(2, map.find(4)->second);
    EXPECT_EQ(1, map.find(6)->second);

    // `[]` inserts a default value
    EXPECT_EQ(0, map[7]);
    EXPECT_EQ(0, map[5]);
    EXPECT_EQ(0, map[0]);
    EXPECT_EQ(0, map[3]);

    // Lookups still work
    EXPECT_EQ(3, map[2]);
    EXPECT_EQ(2, map[4]);
    EXPECT_EQ(1, map[6]);
}

// Check that symmetric set difference is empty using const, mutable, and reverse iterators
TEST_P(TinyMap, Iterate) {
    // Iterate as map
    auto map = util::TinyMap<int, int>{GetParam(), {2, 4, 6}, {3, 2, 1}};
    auto map2 = std::map<int, int>{{2, 3}, {4, 2}, {6, 1}};

    testSetDiff(map.begin(), map.end(), map2);
    testSetDiff(map2.begin(), map2.end(), map);
    testSetDiff(map.rbegin(), map.rend(), map2);
    testSetDiff(map2.rbegin(), map2.rend(), map);

    const auto& cmap = map;
    const auto& cmap2 = map2;
    testSetDiff(cmap.begin(), cmap.end(), cmap2);
    testSetDiff(cmap2.begin(), cmap2.end(), cmap);
    testSetDiff(cmap.rbegin(), cmap.rend(), cmap2);
    testSetDiff(cmap2.rbegin(), cmap2.rend(), cmap);
}

TEST_P(TinyMap, Erase) {
    auto map = util::TinyMap<int, int>{GetParam(), {2, 4, 6, 8, 10, 12}, {1, 2, 3, 4, 5, 6}};

    map.erase(6);
    EXPECT_EQ(5, map.size());

    map.erase(map.find(2));
    EXPECT_EQ(4, map.size());

    EXPECT_EQ(2, map[4]);

    EXPECT_FALSE(map.empty());
    map.clear();
    EXPECT_TRUE(map.empty());
}

TEST_P(TinyMap, StringKey) {
    auto map = util::TinyMap<std::string, int>{GetParam(), {"a", "b", "c"}, {1, 2, 3}};
    EXPECT_EQ(2, map["b"]);
}

TEST_P(TinyMap, StringVal) {
    auto map = util::TinyMap<int, std::string>{GetParam(), {1, 2, 3}, {"a", "b", "c"}};

    std::string s = "d";
    map.insert(4, std::move(s));
    EXPECT_TRUE(s.empty());
    EXPECT_EQ("d", map[4]);
}

INSTANTIATE_TEST_SUITE_P(Sort, TinyMap, testing::Values(true, false));
