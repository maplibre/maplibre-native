#include <mbgl/test/util.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/tiny_unordered_map.hpp>

#include <algorithm>
#include <iomanip>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <unordered_map>

using namespace mbgl;
using namespace mbgl::util;

template <typename TMapIter, typename TMap2>
void testSetDiff(TMapIter beg, TMapIter end, TMap2& map2) {
    while (beg != end) {
        auto item = *beg++;
        auto i = map2.find(item.first);
        EXPECT_TRUE(i);
        EXPECT_EQ(*i, item.second);
    }
}
// TODO: need key-value iteration
// template <typename TMap1, typename TMap2>
// void testSetDiff(TMap1 map1, TMap2& map2) {
//    testSetDiff(map1.begin(), map1.end(), map2);
//    testSetDiff(map2.begin(), map2.end(), map1);
//}

template <std::size_t Threshold>
void testInit() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    // Construct from separate initializer lists
    const auto map = TinyUnorderedMap<int, int, Threshold>{{1, 2, 3}, {3, 2, 1}};
    EXPECT_EQ(3, map.size());

    // Construct from list of pairs
    const auto map2 = TinyUnorderedMap<int, int, Threshold>{{{1, 3}, {2, 2}, {3, 1}}};
    EXPECT_EQ(map.size(), map2.size());
    // testSetDiff(map, map2);
    EXPECT_TRUE(map[1] && map2[1]);
    EXPECT_TRUE(map[2] && map2[2]);
    EXPECT_TRUE(map[3] && map2[3]);

    // Construct in a different order
    const auto map3 = TinyUnorderedMap<int, int, Threshold>{{2, 3, 1}, {2, 1, 3}};
    EXPECT_EQ(map.size(), map3.size());
    // testSetDiff(map, map3);
    EXPECT_TRUE(map3[1]);
    EXPECT_TRUE(map3[2]);
    EXPECT_TRUE(map3[3]);
}
TEST(TinyMap, Init) {
    testInit<0>();
    testInit<2>();
    testInit<5>();
    testInit<10>();
}

template <std::size_t Threshold>
void testCopy() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    const auto map = TinyUnorderedMap<int, int, Threshold>{{1, 2, 3}, {3, 2, 1}};
    decltype(map) map2(map);

    std::remove_const_t<decltype(map)> map3;
    map3 = map2;

    // testSetDiff(map, map2);
    // testSetDiff(map, map3);
    EXPECT_EQ(map.size(), map2.size());
    EXPECT_EQ(map.size(), map3.size());
    EXPECT_TRUE(map[1] && map2[1] && map3[1]);
    EXPECT_TRUE(map[2] && map2[2] && map3[2]);
    EXPECT_TRUE(map[3] && map2[3] && map3[3]);
}
TEST(TinyMap, Copy) {
    testCopy<0>();
    testCopy<2>();
    testCopy<5>();
    testCopy<10>();
}

template <std::size_t Threshold>
void testGenInit() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    const auto input = std::vector<int>{1, 2, 3};
    const auto map = TinyUnorderedMap<int, int, Threshold>{input.begin(), input.end(), [](const auto& x) {
                                                               return std::make_pair(x, 4 - x);
                                                           }};

    EXPECT_EQ(3, map.size());
    EXPECT_EQ(3, map[1]);
    EXPECT_EQ(2, map[2]);
    EXPECT_EQ(1, map[3]);
}
TEST(TinyMap, GenInit) {
    testGenInit<0>();
    testGenInit<2>();
    testGenInit<5>();
    testGenInit<10>();
}

template <std::size_t Threshold>
void testMove() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    const auto map = TinyUnorderedMap<int, int, Threshold>{{1, 2, 3}, {3, 2, 1}};
    std::remove_const_t<decltype(map)> map2(map);

    // move constructor
    decltype(map2) map3(std::move(map2));
    EXPECT_TRUE(map2.empty());
    // testSetDiff(map, map3);
    EXPECT_TRUE(map3[1]);
    EXPECT_TRUE(map3[2]);
    EXPECT_TRUE(map3[3]);

    // move assignment
    decltype(map2) map4;
    map4 = std::move(map3);
    EXPECT_TRUE(map3.empty());
    // testSetDiff(map, map4);
    EXPECT_TRUE(map4[1]);
    EXPECT_TRUE(map4[2]);
    EXPECT_TRUE(map4[3]);

    map4.clear();
    EXPECT_TRUE(map4.empty());
}
TEST(TinyMap, Move) {
    testMove<0>();
    testMove<2>();
    testMove<5>();
    testMove<10>();
}

template <std::size_t Threshold>
void testConstLookup() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    const auto map = TinyUnorderedMap<int, int, Threshold>{{2, 4, 6}, {3, 2, 1}};

    EXPECT_EQ(3, *map.find(2));
    EXPECT_EQ(2, *map.find(4));
    EXPECT_EQ(1, *map.find(6));
}
TEST(TinyMap, ConstLookup) {
    testConstLookup<0>();
    testConstLookup<2>();
    testConstLookup<5>();
    testConstLookup<10>();
}

template <std::size_t Threshold>
void testMutableLookup() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    auto map = TinyUnorderedMap<int, int, Threshold>{{2, 4, 6}, {3, 2, 1}};

    EXPECT_EQ(3, *map.find(2));
    EXPECT_EQ(2, *map.find(4));
    EXPECT_EQ(1, *map.find(6));

    // operator[] doesn't insert... for now
    EXPECT_FALSE(map[7]);
    EXPECT_FALSE(map[5]);
    EXPECT_FALSE(map[0]);
    EXPECT_FALSE(map[3]);
    EXPECT_EQ(3, map.size());

    EXPECT_EQ(3, map[2]);
    EXPECT_EQ(2, map[4]);
    EXPECT_EQ(1, map[6]);
}
TEST(TinyMap, MutableLookup) {
    testMutableLookup<0>();
    testMutableLookup<2>();
    testMutableLookup<5>();
    testMutableLookup<10>();
}

template <std::size_t Threshold>
void testTileIDKey() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    const std::vector<OverscaledTileID> keys = {
        {1, 0, {1, 0, 0}},
        {1, 0, {1, 1, 0}},
        {1, 0, {1, 0, 1}},
        {1, 0, {1, 1, 1}},

        {2, 0, {2, 0, 0}},
        {2, 0, {2, 1, 0}},
        {2, 0, {2, 0, 1}},
        {2, 0, {2, 1, 1}},
        {2, 0, {2, 2, 1}},
        {2, 0, {2, 3, 1}},
        {2, 0, {2, 2, 2}},
        {2, 0, {2, 3, 2}},

        {0, 0, {0, 0, 0}},
    };
    const std::vector<std::size_t> values(keys.size(), 0);
    const auto map = TinyUnorderedMap<OverscaledTileID, std::size_t, Threshold>{
        keys.begin(), keys.end(), values.begin(), values.end()};

    for (const auto& k : keys) {
        EXPECT_TRUE(map.find(k));
        EXPECT_FALSE(map.find({3, k.wrap, k.canonical}));
        EXPECT_FALSE(map.find({k.overscaledZ, 1, k.canonical}));
    }
}
TEST(TinyMap, TileIDKey) {
    testTileIDKey<0>();
    testTileIDKey<2>();
    testTileIDKey<5>();
    testTileIDKey<10>();
}

template <std::size_t Threshold>
void testCustomComp() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    const auto map = TinyUnorderedMap<int, int, Threshold>{{1, 2, 3}, {3, 2, 1}};
    const auto map2 = TinyUnorderedMap<int, int, Threshold, std::hash<int>, std::equal_to<>>{{2, 3, 1}, {2, 1, 3}};
    // testSetDiff(map, map2);
    EXPECT_EQ(map.size(), map2.size());
    EXPECT_TRUE(map2[1]);
    EXPECT_TRUE(map2[2]);
    EXPECT_TRUE(map2[3]);
}
TEST(TinyMap, CustomComp) {
    testCustomComp<0>();
    testCustomComp<2>();
    testCustomComp<5>();
    testCustomComp<10>();
}

template <typename T>
using TIter = typename std::vector<T>::const_iterator;
template <typename TMap, typename TKey>
using TMakeMap = std::function<TMap(TIter<TKey>, TIter<TKey>, TIter<std::size_t>, TIter<std::size_t>)>;
using ssize_t = std::ptrdiff_t;

void incTileID(OverscaledTileID& tid) {
    tid.canonical.y++;
    if (tid.canonical.y == 1U << tid.canonical.z) {
        tid.canonical.y = 0;
        tid.canonical.x++;
    }
    if (tid.canonical.x == 1U << tid.canonical.z) {
        tid.canonical.x = 0;
        tid.canonical.z++;
    }
}
// return a function that generate successive tile IDs
std::function<OverscaledTileID()> makeGenTileID() {
    auto tid = std::make_shared<OverscaledTileID>(0, 0, 0, 0, 0);
    return [=] {
        auto v = *tid;
        incTileID(*tid);
        return v;
    };
}

std::function<std::size_t()> makeGenIntID() {
    static std::size_t n = 0;
    return [=] {
        return ++n;
    };
}

std::function<std::string()> makeGenStringID() {
    static int n = 0;
    return [=] {
        return "someprefix_" + std::to_string(++n);
    };
}

static volatile int do_not_optimize_away = 0;

template <typename TKey, // key type
          typename TMap, // map type
          std::size_t Threshold = 0>
void benchmark(const std::string_view label,
               const std::size_t max,     // largest number of elements to consider
               const std::size_t lookups, // number of searches for each item (~ read/write ratio)
               const std::size_t reports, // number of items reported from `max` (should divide evenly)
               const TMakeMap<TMap, TKey> make,
               const std::function<TKey()> generate,
               const std::size_t seed = 0xf00dLL) {
    if ((max / reports) * reports != max) {
        assert(false);
        return;
    }

    std::seed_seq seed_seq{seed};
    std::default_random_engine engine(seed_seq);

    // build keys
    std::vector<TKey> keys;
    keys.reserve(max);
    std::generate_n(std::back_inserter(keys), max, std::move(generate));

    // build values
    std::vector<size_t> values(keys.size());
    std::iota(values.begin(), values.end(), 0);

    using SteadyClock = std::chrono::steady_clock;
    using Usec = std::chrono::microseconds;
    std::vector<double> times(reports);

    constexpr std::size_t timingIterations = 50;

    // test each size from 1 to max
    for (std::size_t i = 1; i <= max; ++i) {
        // Build and lookup keys in different random orders
        std::vector<TKey> buildKeys(keys.begin(), keys.begin() + i);
        std::vector<TKey> testKeys(keys.begin(), keys.begin() + i);
        std::shuffle(buildKeys.begin(), buildKeys.end(), engine);
        std::shuffle(testKeys.begin(), testKeys.end(), engine);

        // (build a map and look up the keys many times) many times
        const auto startTime = SteadyClock::now();
        for (std::size_t tt = 0; tt < timingIterations; ++tt) {
            const auto map = make(buildKeys.begin(), buildKeys.end(), values.begin(), values.begin() + i);
            for (std::size_t j = 0; j < lookups; ++j) {
                for (const auto& k : testKeys) {
                    if (map.count(k)) {
                        do_not_optimize_away = do_not_optimize_away + 1;
                    }
                }
            }
        }

        if ((i % (max / reports)) == 0) {
            const auto totalElapsed = std::chrono::duration_cast<Usec>(SteadyClock::now() - startTime);
            const auto elapsed = static_cast<double>(totalElapsed.count()) / timingIterations;
            times[i / (max / reports) - 1] = elapsed;
        }
    }

    std::stringstream ss;
    ss << std::setw(10) << std::left << label;
    ss << " threshold=" << std::setw(3) << std::setfill('0') << std::right << Threshold << std::setfill(' ');
    ss << " (x" << std::setw(2) << (max / reports) << "): ";
    for (std::size_t i = 0; i < reports; ++i) {
        ss << std::setw(6) << std::round(times[i] * 10);
    }
    Log::Info(Event::Timing, ss.str());
}

constexpr std::size_t lookups = 100;

TEST(TinyMap, TEST_REQUIRES_ACCURATE_TIMING(BenchmarkRef)) {
#if defined(DEBUG)
    Log::Info(Event::General, "Build Type: Debug");
#elif defined(NDEBUG)
    Log::Info(Event::General, "Build Type: Release");
#else
    Log::Info(Event::General, "Build Type: ?");
#endif

    // std::unordered_map with size_t keys
    benchmark<std::size_t, std::unordered_map<std::size_t, size_t>>(
        "int:stl",
        20,
        lookups,
        10,
        [&](auto kb, auto ke, auto vb, auto) {
            std::unordered_map<std::size_t, size_t> m;
            m.reserve(std::distance(kb, ke));
            while (kb != ke) m.insert(std::make_pair(*kb++, *vb++));
            return m;
        },
        makeGenIntID());

    // std::unordered_map with string keys
    benchmark<std::string, std::unordered_map<std::string, size_t>>(
        "str:stl",
        10,
        lookups,
        10,
        [&](auto kb, auto ke, auto vb, auto) {
            std::unordered_map<std::string, size_t> m;
            m.reserve(std::distance(kb, ke));
            while (kb != ke) m.insert(std::make_pair(*kb++, *vb++));
            return m;
        },
        makeGenStringID());

    benchmark<OverscaledTileID, std::unordered_map<OverscaledTileID, size_t>>(
        "tile:stl",
        20,
        lookups,
        10,
        [&](auto kb, auto ke, auto vb, auto) {
            std::unordered_map<OverscaledTileID, size_t> m;
            m.reserve(std::distance(kb, ke));
            while (kb != ke) m.insert(std::make_pair(*kb++, *vb++));
            return m;
        },
        makeGenTileID());
}

template <std::size_t Threshold>
void testBenchmarkTinyMap() {
    testing::ScopedTrace trace(__FILE__, __LINE__, Threshold);

    // TinyMap with int keys
    benchmark<std::size_t, TinyUnorderedMap<std::size_t, size_t, Threshold>, Threshold>(
        "int:tiny",
        20,
        lookups,
        10,
        [&](auto kb, auto ke, auto vb, auto ve) {
            return TinyUnorderedMap<std::size_t, size_t, Threshold>{kb, ke, vb, ve};
        },
        makeGenIntID());

    // TinyMap with string keys
    benchmark<std::string, TinyUnorderedMap<std::string, size_t, Threshold>, Threshold>(
        "str:tiny",
        10,
        lookups,
        10,
        [&](auto kb, auto ke, auto vb, auto ve) {
            return TinyUnorderedMap<std::string, size_t, Threshold>{kb, ke, vb, ve};
        },
        makeGenStringID());

    benchmark<OverscaledTileID, TinyUnorderedMap<OverscaledTileID, size_t, Threshold>, Threshold>(
        "tile:tiny",
        20,
        lookups,
        10,
        [&](auto kb, auto ke, auto vb, auto ve) {
            return TinyUnorderedMap<OverscaledTileID, size_t, Threshold>{kb, ke, vb, ve};
        },
        makeGenTileID());
}
TEST(TinyMap, TEST_REQUIRES_ACCURATE_TIMING(BenchmarkTinyMap)) {
    testBenchmarkTinyMap<4>();
    testBenchmarkTinyMap<8>();
    testBenchmarkTinyMap<16>();
}
