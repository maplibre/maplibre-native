#include <mbgl/test/util.hpp>
#include <mbgl/util/std.hpp>

#include <numeric>
#include <random>
#include <ranges>
#include <vector>

using namespace mbgl::util;

TEST(STD, orderedInsert) {
    std::mt19937 generator(42);
    std::vector<int> values(1000);
    std::iota(values.begin(), values.end(), 0);
    std::ranges::shuffle(values, generator);

    std::vector<int> result;
    std::ranges::copy(values, make_ordered_inserter(result));
    EXPECT_TRUE(std::ranges::is_sorted(result));

    result.clear();
    std::ranges::copy(values, make_ordered_inserter(result, std::greater<int>()));
    EXPECT_TRUE(std::ranges::is_sorted(result, std::greater<int>()));
}
