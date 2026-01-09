#include "mapbox/compatibility/value.hpp"

#include <gtest/gtest.h>

template <typename T>
void unused(T&&) {}

TEST(Compatibility, Dummy) {
    mapbox::base::Value v;
    EXPECT_FALSE(bool(v));

    mapbox::base::ValueArray va;
    EXPECT_EQ(va.size(), 0);

    mapbox::base::ValueObject vo;
    EXPECT_EQ(vo.size(), 0);

    mapbox::base::NullValue nv;
    unused(nv);
}
