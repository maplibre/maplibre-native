#include <mbgl/test/util.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/tiny_map.hpp>

#include <algorithm>
#include <iomanip>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <unordered_map>

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
    auto map = util::TinyMap<std::string, int>{GetParam(), {"c", "b", "a"}, {1, 2, 3}};
    EXPECT_EQ(2, map["b"]);
}

TEST_P(TinyMap, StringVal) {
    auto map = util::TinyMap<int, std::string>{GetParam(), {1, 2, 3}, {"a", "b", "c"}};

    std::string s = "d";
    map.insert(5, std::move(s));
    EXPECT_TRUE(s.empty());

    s = "f";
    map[4] = std::move(s);

    EXPECT_EQ("d", map[5]);
    EXPECT_EQ("f", map[4]);
}

TEST_P(TinyMap, TileIDKey) {
    const bool sorted = GetParam();
    const std::vector<OverscaledTileID> keys = {
        {2, 0, {2, 0, 0}},
        {2, 0, {2, 1, 0}},
        {2, 0, {2, 0, 1}},
        {2, 0, {2, 1, 1}},
        {2, 0, {2, 2, 1}},
        {2, 0, {2, 3, 1}},
        {2, 0, {2, 2, 2}},
        {2, 0, {2, 3, 2}},

        {1, 0, {1, 0, 0}},
        {1, 0, {1, 1, 0}},
        {1, 0, {1, 0, 1}},
        {1, 0, {1, 1, 1}},

        {0, 0, {0, 0, 0}},
    };
    const std::vector<std::size_t> values(keys.size(), 0);
    const auto map = util::TinyMap<OverscaledTileID, int>{
        sorted, keys.begin(), keys.end(), values.begin(), values.end()};

    for (const auto& k : keys) {
        EXPECT_EQ(1, map.count(k));
        EXPECT_EQ(0, map.count({3, k.wrap, k.canonical}));
        EXPECT_EQ(0, map.count({k.overscaledZ, 1, k.canonical}));
    }

    EXPECT_TRUE(!sorted ||
                std::is_sorted(map.begin(), map.end(), [](const auto& a, const auto& b) { return a.first < b.first; }));
}

TEST_P(TinyMap, CustomComp) {
    const bool sorted = GetParam();
    const auto map = util::TinyMap<int, int>{sorted, {1, 2, 3}, {3, 2, 1}};
    const auto map2 = util::TinyMap<int, int, std::greater<int>>{sorted, {2, 3, 1}, {2, 1, 3}};
    testSetDiff(map, map2);

    // First key should be the greatest if we're sorted, first-inserted otherwise
    EXPECT_EQ(GetParam() ? 3 : 2, map2.begin()->first);
}

TEST_P(TinyMap, ChangeSort) {
    const bool sorted = GetParam();
    auto map = util::TinyMap<int, int>{sorted, {3, 2, 1}, {1, 2, 3}};
    auto map2 = util::TinyMap<int, int>{!sorted, {2, 3, 1}, {2, 1, 3}};
    testSetDiff(map, map2);
    EXPECT_EQ(sorted ? 1 : 3, map.begin()->first);
    EXPECT_EQ(sorted ? 2 : 1, map2.begin()->first);

    map.setSorted(!sorted);
    map2.setSorted(sorted);
    testSetDiff(map, map2);

    EXPECT_EQ(1, map.begin()->first);
    EXPECT_EQ(1, map2.begin()->first);
}

template <typename T>
using TIter = typename std::vector<T>::const_iterator;
template <typename TMap, typename TKey>
using TMakeMap = std::function<TMap(TIter<TKey>, TIter<TKey>, TIter<std::size_t>, TIter<std::size_t>)>;
using ssize_t = std::ptrdiff_t;

void incTileID(OverscaledTileID& tid) {
    tid.canonical.y++;
    if (tid.canonical.y == 1 << tid.canonical.z) {
        tid.canonical.y = 0;
        tid.canonical.x++;
    }
    if (tid.canonical.x == 1 << tid.canonical.z) {
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

static volatile int do_not_optimize_away = 0;

template <typename TKey,       // key type
          typename TMap,       // map type
          std::size_t max,     // largest number of elements to consider
          std::size_t lookups, // number of searches for each item (~ read/write ratio)
          std::size_t reports  // number of items reported from `max` (should divide evenly)
          >
void benchmark(bool sorted,
               std::string_view label,
               TMakeMap<TMap, TKey> make,
               std::function<TKey()> generate,
               std::size_t seed = 0xf00dLL) {
    static_assert((max / reports) * reports == max);

    std::seed_seq seed_seq{0xf00dLL};
    std::default_random_engine engine(seed_seq);

    // build keys
    std::vector<TKey> keys;
    keys.reserve(max);
    std::generate_n(std::back_inserter(keys), max, std::move(generate));

    // build values
    std::vector<size_t> values(keys.size());
    std::iota(values.begin(), values.end(), 0);

    using Clock = std::chrono::steady_clock;
    using Usec = std::chrono::microseconds;
    std::vector<double> times(reports);

    constexpr std::size_t timingIterations = 50;

    // test each size from 1 to max
    int x = 0;
    for (std::size_t i = 1; i <= max; ++i) {
        // Build and lookup keys in different random orders
        std::vector<TKey> buildKeys(keys.begin(), keys.begin() + i);
        std::vector<TKey> testKeys(keys.begin(), keys.begin() + i);
        std::shuffle(buildKeys.begin(), buildKeys.end(), engine);
        std::shuffle(testKeys.begin(), testKeys.end(), engine);

        // (build a map and look up the keys many times) many times
        const auto startTime = Clock::now();
        for (std::size_t tt = 0; tt < timingIterations; ++tt) {
            const auto map = make(buildKeys.begin(), buildKeys.end(), values.begin(), values.begin() + i);
            for (std::size_t j = 0; j < lookups; ++j) {
                for (const auto& k : testKeys) {
                    if (map.find(k) != map.end()) {
                        do_not_optimize_away++;
                    }
                }
            }
        }

        if ((i % (max / reports)) == 0) {
            const auto totalElapsed = std::chrono::duration_cast<Usec>(Clock::now() - startTime);
            const auto elapsed = static_cast<double>(totalElapsed.count()) / timingIterations;
            times[i / (max / reports) - 1] = elapsed;
        }
    }

    std::stringstream ss;
    ss << std::setw(15) << label << " keysize=" << std::setw(2) << sizeof(TKey);
    ss << " sort=" << (sorted ? "T" : "F");
    ss << " ratio=" << lookups;
    ss << " (x" << (max / reports) << "): ";
    for (std::size_t i = 0; i < reports; ++i) {
        ss << std::setw(6) << std::round(times[i] * 10);
    }
    Log::Info(Event::Timing, ss.str());
}

TEST_P(TinyMap, TEST_REQUIRES_ACCURATE_TIMING(Benchmark)) {
#if defined(DEBUG)
    Log::Info(Event::General, "Build Type: Debug");
#elif defined(NDEBUG)
    Log::Info(Event::General, "Build Type: Release");
#else
    Log::Info(Event::General, "Build Type: ?");
#endif

    const bool sorted = GetParam();
    constexpr std::size_t lookups = 100;

    std::size_t n = 0;
    const auto genInts = [&] {
        return n += 3821; // arbitrary value
    };

    // TinyMap with size_t keys
    benchmark<std::size_t, util::TinyMap<std::size_t, size_t>, 60, lookups, 20>(
        sorted,
        "TinyMap",
        [&](auto kb, auto ke, auto vb, auto ve) {
            return util::TinyMap<std::size_t, size_t>{sorted, kb, ke, vb, ve};
        },
        genInts);

    if (sorted) {
        // std::map with size_t keys
        n = 0;
        benchmark<std::size_t, std::map<std::size_t, size_t>, 60, lookups, 20>(
            sorted,
            "map",
            [&](auto kb, auto ke, auto vb, auto ve) {
                std::map<std::size_t, size_t> m;
                while (kb != ke) m.insert(std::make_pair(*kb++, *vb++));
                return m;
            },
            genInts);
    } else {
        // std::unordered_map with size_t keys
        n = 0;
        benchmark<std::size_t, std::unordered_map<std::size_t, size_t>, 60, lookups, 20>(
            sorted,
            "unordered_map",
            [&](auto kb, auto ke, auto vb, auto ve) {
                std::unordered_map<std::size_t, size_t> m;
                m.reserve(std::distance(kb, ke));
                while (kb != ke) m.insert(std::make_pair(*kb++, *vb++));
                return m;
            },
            genInts);
    }

    benchmark<OverscaledTileID, util::TinyMap<OverscaledTileID, size_t>, 20, lookups, 20>(
        sorted,
        "TinyMap",
        [&](auto kb, auto ke, auto vb, auto ve) {
            return util::TinyMap<OverscaledTileID, size_t>{sorted, kb, ke, vb, ve};
        },
        makeGenTileID());

    if (sorted) {
        benchmark<OverscaledTileID, std::map<OverscaledTileID, size_t>, 20, lookups, 20>(
            sorted,
            "map",
            [&](auto kb, auto ke, auto vb, auto ve) {
                std::map<OverscaledTileID, size_t> m;
                while (kb != ke) m.insert(std::make_pair(*kb++, *vb++));
                return m;
            },
            makeGenTileID());
    } else {
        benchmark<OverscaledTileID, std::unordered_map<OverscaledTileID, size_t>, 20, lookups, 20>(
            sorted,
            "unordered_map",
            [&](auto kb, auto ke, auto vb, auto ve) {
                std::unordered_map<OverscaledTileID, size_t> m;
                m.reserve(std::distance(kb, ke));
                while (kb != ke) m.insert(std::make_pair(*kb++, *vb++));
                return m;
            },
            makeGenTileID());
    }
}

// Use `testing::GTEST_FLAG(filter) = "Sort/TinyMap.*/*"` to run these alone
INSTANTIATE_TEST_SUITE_P(Sort, TinyMap, testing::Values(false, true));
