#include <mln/util/grid_index.hpp>

#include <mln/test/util.hpp>

#include <algorithm>
#include <functional>
#include <optional>
#include <vector>

using namespace mln;

TEST(GridIndex, IndexesFeatures) {
    GridIndex<int16_t> grid(100, 100, 10);
    grid.insert(0, {{4, 10}, {6, 30}});
    grid.insert(1, {{4, 10}, {30, 12}});
    grid.insert(2, {{-10, 30}, {5, 35}});

    EXPECT_EQ(grid.query({{4, 10}, {5, 11}}), (std::vector<int16_t>{0, 1}));
    EXPECT_EQ(grid.query({{24, 10}, {25, 11}}), (std::vector<int16_t>{1}));
    EXPECT_EQ(grid.query({{40, 40}, {100, 100}}), (std::vector<int16_t>{}));
    EXPECT_EQ(grid.query({{-6, 0}, {3, 100}}), (std::vector<int16_t>{2}));
    EXPECT_EQ(grid.query({{-1000, -1000}, {1000, 1000}}), (std::vector<int16_t>{0, 1, 2}));
}

TEST(GridIndex, DuplicateKeys) {
    GridIndex<int16_t> grid(100, 100, 10);
#define KEY 123
    grid.insert(KEY, {{3, 4}, {4, 4}});
    grid.insert(KEY, {{13, 13}, {14, 14}});
    grid.insert(KEY, {{23, 23}, {24, 24}});

    EXPECT_EQ(grid.query({{0, 0}, {30, 30}}), (std::vector<int16_t>{KEY, KEY, KEY}));
}

TEST(GridIndex, CircleCircle) {
    GridIndex<int16_t> grid(100, 100, 10);
    grid.insert(0, {{50, 50}, 10});
    grid.insert(1, {{60, 60}, 15});
    grid.insert(2, {{-10, 110}, 20});

    EXPECT_TRUE(grid.hitTest({{55, 55}, 2}));
    EXPECT_FALSE(grid.hitTest({{10, 10}, 10}));
    EXPECT_TRUE(grid.hitTest({{0, 100}, 10}));
    EXPECT_TRUE(grid.hitTest({{80, 60}, 10}));
}

TEST(GridIndex, CircleBox) {
    GridIndex<int16_t> grid(100, 100, 10);
    grid.insert(0, {{50, 50}, 10});
    grid.insert(1, {{60, 60}, 15});
    grid.insert(2, {{-10, 110}, 20});

    EXPECT_EQ(grid.query({{45, 45}, {55, 55}}), (std::vector<int16_t>{0, 1}));
    EXPECT_EQ(grid.query({{0, 0}, {30, 30}}), (std::vector<int16_t>{}));
    EXPECT_EQ(grid.query({{0, 80}, {20, 100}}), (std::vector<int16_t>{2}));
}

TEST(GridIndex, IndexesFeaturesOverflow) {
    GridIndex<int16_t> grid(5000, 5000, 25);
    grid.insert(0, {{4500, 4500}, {4900, 4900}});
    EXPECT_EQ(grid.query({{4000, 4000}, {5000, 5000}}), (std::vector<int16_t>{0}));
}

namespace {

using Box = GridIndex<uint32_t>::BBox;
using Circle = GridIndex<uint32_t>::BCircle;

bool collide(const Box& a, const Box& b) {
    return a.min.x <= b.max.x && a.min.y <= b.max.y && a.max.x >= b.min.x && a.max.y >= b.min.y;
}

// Small deterministic generator, so the test does not depend on the standard library's distributions.
struct Random {
    uint32_t state = 12345;
    float next(float min, float max) {
        state = state * 1664525u + 1013904223u;
        return min + (max - min) * (static_cast<float>(state >> 8) / static_cast<float>(1u << 24));
    }
};

} // namespace

// hitTest takes only a callable that tells whether an element counts.
static_assert(GridIndexPredicate<std::function<bool(const uint32_t&)>, uint32_t>);
static_assert(!GridIndexPredicate<int, uint32_t>);
static_assert(!GridIndexPredicate<std::function<void(const uint32_t&)>, uint32_t>);

// Boxes that span many cells, reach outside the grid or cover it completely must be reported exactly once.
TEST(GridIndex, EveryCollidingBoxIsReportedOnce) {
    GridIndex<uint32_t> grid(200, 100, 10);
    Random random;
    std::vector<Box> boxes;
    for (uint32_t i = 0; i < 300; ++i) {
        const float x = random.next(-40, 220);
        const float y = random.next(-40, 120);
        const Box box{{x, y}, {x + random.next(0, 80), y + random.next(0, 80)}};
        boxes.push_back(box);
        grid.insert(uint32_t(i), box);
    }
    boxes.push_back({{-10, -10}, {210, 110}});
    grid.insert(300u, boxes.back());

    for (int i = 0; i < 200; ++i) {
        const float x = random.next(-20, 200);
        const float y = random.next(-20, 100);
        const Box query = i == 0 ? Box{{-5, -5}, {205, 105}}
                                 : Box{{x, y}, {x + random.next(0, 90), y + random.next(0, 90)}};

        // A query that covers the grid returns everything; one outside of it returns nothing.
        std::vector<uint32_t> expected;
        if (!(query.max.x < 0 || query.min.x >= 200 || query.max.y < 0 || query.min.y >= 100)) {
            for (uint32_t id = 0; id < boxes.size(); ++id) {
                if (i == 0 || collide(query, boxes[id])) expected.push_back(id);
            }
        }
        auto actual = grid.query(query);
        std::sort(actual.begin(), actual.end());
        EXPECT_EQ(expected, actual) << "query " << i;

        const std::optional<std::function<bool(const uint32_t&)>> odd = [](const uint32_t& id) {
            return id % 2 == 1;
        };
        const bool expectedOdd = std::any_of(expected.begin(), expected.end(), [](uint32_t id) { return id % 2 == 1; });
        EXPECT_EQ(expectedOdd, grid.hitTest(query, odd)) << "query " << i;
        EXPECT_EQ(!expected.empty(), grid.hitTest(query)) << "query " << i;
    }
}

// A circle that covers the whole grid looks at every element once.
TEST(GridIndex, CoveringCircleTestsEveryElementOnce) {
    GridIndex<uint32_t> grid(100, 100, 10);
    grid.insert(0u, Box{{10, 10}, {20, 20}});
    grid.insert(1u, Box{{60, 60}, {95, 95}});
    grid.insert(2u, Circle{{50, 50}, 10});

    std::vector<uint32_t> tested;
    const std::optional<std::function<bool(const uint32_t&)>> record = [&](const uint32_t& id) {
        tested.push_back(id);
        return false;
    };
    EXPECT_FALSE(grid.hitTest(Circle{{50, 50}, 500}, record));
    std::sort(tested.begin(), tested.end());
    EXPECT_EQ((std::vector<uint32_t>{0, 1, 2}), tested);
}

// A circle that spans several cells finds boxes and circles there, each tested once.
TEST(GridIndex, CircleTestsEveryCollidingElementOnce) {
    GridIndex<uint32_t> grid(100, 100, 10);
    grid.insert(0u, Box{{10, 10}, {60, 60}});
    grid.insert(1u, Circle{{55, 55}, 25});
    grid.insert(2u, Box{{90, 90}, {95, 95}});

    std::vector<uint32_t> tested;
    const std::optional<std::function<bool(const uint32_t&)>> record = [&](const uint32_t& id) {
        tested.push_back(id);
        return false;
    };
    EXPECT_FALSE(grid.hitTest(Circle{{40, 40}, 22}, record));
    std::sort(tested.begin(), tested.end());
    EXPECT_EQ((std::vector<uint32_t>{0, 1}), tested);
}
