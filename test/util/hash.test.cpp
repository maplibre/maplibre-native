#include <mbgl/test/util.hpp>

#include <mbgl/util/hash.hpp>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>

using namespace mbgl;

TEST(OrderIndependentHash, Permutations) {
    // Try collections of up to this many elements
    constexpr int maxSize = 6;
    // Try this many different sets of values for each size
    constexpr int iterations = 10;
    // Use values up to this large
    constexpr size_t maxVal = 10;

    std::seed_seq seed{0xf00dLL};
    std::default_random_engine engine(seed);
    std::uniform_int_distribution<size_t> distribution(0, maxVal);

    for (int curSize = 1; curSize <= maxSize; ++curSize) {
        for (int ii = 0; ii < iterations; ++ii) {
            // Generate `curSize` random values
            std::vector<size_t> values;
            for (int jj = 0; jj < curSize; ++jj) {
                values.push_back(distribution(engine));
            }

            // Generate the hash for each permutation, comparing subsequent values to the first one.
            bool isFirst = true;
            size_t initialValue = 0;
            do {
                const auto curValue = util::order_independent_hash(values.begin(), values.end());
                if (isFirst || curValue != initialValue) {
                    for (auto v : values) {
                        std::cout << v << ",";
                    }
                    std::cout << " - " << std::hex << curValue << std::endl;
                }
                if (isFirst) {
                    initialValue = curValue;
                    isFirst = false;
                } else {
                    EXPECT_EQ(initialValue, curValue);
                }
            } while (std::next_permutation(values.begin(), values.end()));
        }
    }
}
