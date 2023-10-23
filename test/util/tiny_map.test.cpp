#include <mbgl/test/util.hpp>

#include <mbgl/util/tiny_map.hpp>

using namespace mbgl::util;

TEST(TinyMap, Init) {
    // Construct from separate initializer lists
    const auto map = TinyMap<int, int>{{1, 2, 3}, {3, 2, 1}};
    EXPECT_EQ(3, map.size());
    EXPECT_LT(1 << 16, map.max_size());

    // Construct from list of pairs
    const auto map2 = TinyMap<int, int>{{1, 3}, {2, 2}, {3, 1}};
    EXPECT_EQ(map, map2);

    // Construct in a different order
    const auto map3 = TinyMap<int, int>{{2, 3, 1}, {2, 1, 3}};
    EXPECT_EQ(map, map3);
}

TEST(TinyMap, Copy) {
    const auto map = TinyMap<int, int>{{1, 2, 3}, {3, 2, 1}};
    decltype(map) map2(map);

    std::remove_const_t<decltype(map)> map3;
    map3 = map2;

    EXPECT_EQ(map, map2);
    EXPECT_EQ(map, map3);
}

TEST(TinyMap, Move) {
    const auto map = TinyMap<int, int>{{1, 2, 3}, {3, 2, 1}};
    std::remove_const_t<decltype(map)> map2(map);

    // move constructor
    decltype(map2) map3(std::move(map2));
    EXPECT_TRUE(map2.empty());
    EXPECT_EQ(map, map3);

    // move assignment
    decltype(map2) map4;
    map4 = std::move(map3);
    EXPECT_TRUE(map3.empty());
    EXPECT_EQ(map, map4);
}

template <typename T>
void testLookup(T& map) {
    EXPECT_EQ(3, map.find(2)->second);
    EXPECT_EQ(2, map.find(4)->second);
    EXPECT_EQ(1, map.find(6)->second);

    EXPECT_EQ(map.begin(), map.lower_bound(1));
    EXPECT_EQ(map.begin(), map.lower_bound(2));
    EXPECT_EQ(--map.end(), map.upper_bound(5));
    EXPECT_EQ(map.end()--, map.upper_bound(6));

    auto range = map.equal_range(4);
    EXPECT_EQ(1, std::distance(range.first, range.second));
    EXPECT_EQ(4, range.first->first);
    EXPECT_EQ(2, range.first->second);

    range = map.equal_range(999);
    EXPECT_EQ(range.first, range.second);
    EXPECT_EQ(0, std::distance(range.first, range.second));
}

TEST(TinyMap, ConstLookup) {
    const auto map = TinyMap<int, int>{{2, 4, 6}, {3, 2, 1}};
    testLookup(map);
}

TEST(TinyMap, MutableLookup) {
    auto map = TinyMap<int, int>{{2, 4, 6}, {3, 2, 1}};
    testLookup(map);

    // `[]` inserts a default value
    EXPECT_EQ(0, map[7]);
    EXPECT_EQ(0, map[5]);
    EXPECT_EQ(0, map[0]);
    EXPECT_EQ(0, map[3]);

    // Lookups still work
    EXPECT_EQ(3, map[2]);
    EXPECT_EQ(2, map[4]);
    EXPECT_EQ(1, map[6]);

    map.erase(0);
    map.erase(3);
    map.erase(5);
    map.erase(7);

    testLookup(map);
}

template <typename A, typename B, typename C, typename D>
bool equal(const std::pair<A, B>& a, const std::pair<C, D>& b) {
    return a.first == b.first && a.second == b.second;
}
const auto IntPairEq = equal<const int&, const int&, int, int>; // Why isn't this inferred?

TEST(TinyMap, Iterate) {
    // Iterate as map
    const auto map = TinyMap<int, int>{{2, 4, 6}, {3, 2, 1}};
    const auto map2 = std::map<int, int>{{2, 3}, {4, 2}, {6, 1}};
    EXPECT_TRUE(std::equal(map.begin(), map.end(), map2.begin(), map2.end(), IntPairEq));
    EXPECT_TRUE(std::equal(map.rbegin(), map.rend(), map2.rbegin(), map2.rend(), IntPairEq));
}

TEST(TinyMap, IterateMutable) {
    // Iterate as map
    auto map = TinyMap<int, int>{{2, 4, 6}, {3, 2, 1}};
    auto map2 = std::map<int, int>{{2, 3}, {4, 2}, {6, 1}};
    EXPECT_TRUE(std::equal(map.begin(), map.end(), map2.begin(), map2.end(), IntPairEq));
    EXPECT_TRUE(std::equal(map.rbegin(), map.rend(), map2.rbegin(), map2.rend(), IntPairEq));
}

TEST(TinyMap, Erase) {
    auto map = TinyMap<int, int>{{2, 4, 6, 8, 10, 12}, {1, 2, 3, 4, 5, 6}};

    map.erase(6);
    EXPECT_EQ(5, map.size());

    map.erase(map.find(2));
    EXPECT_EQ(4, map.size());

    map.erase(map.lower_bound(10), map.upper_bound(12));
    EXPECT_EQ(2, map.size());

    map.erase(map.equal_range(8).first, map.equal_range(8).second);
    EXPECT_EQ(1, map.size());

    EXPECT_EQ(2, map[4]);

    EXPECT_FALSE(map.empty());
    map.clear();
    EXPECT_TRUE(map.empty());
}

TEST(TinyMap, StringKey) {
    auto map = TinyMap<std::string, int>{{"a", "b", "c"}, {1, 2, 3}};
    EXPECT_EQ(2, map["b"]);
}
