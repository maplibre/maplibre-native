#include <gtest/gtest.h>

#include <mbgl/algorithm/contour/units.hpp>

using namespace mbgl::algorithm::contour;

// The contour algorithm operates in the height grid's source unit (metres for
// terrarium DEMs). The contour-tile builder converts threshold + label values
// to the style-configured display unit before emission. These helpers are the
// pure conversion layer.

TEST(ContourUnits, MetersIsIdentity) {
    UnitConfig cfg;
    cfg.unit = ContourUnit::Meters;
    EXPECT_DOUBLE_EQ(metersToUnit(0.0, cfg), 0.0);
    EXPECT_DOUBLE_EQ(metersToUnit(100.0, cfg), 100.0);
    EXPECT_DOUBLE_EQ(metersToUnit(-50.5, cfg), -50.5);
}

TEST(ContourUnits, FeetUsesInternationalFoot) {
    // 1 international foot is defined as exactly 0.3048 m, so 1 m == 1/0.3048
    // feet ≈ 3.28083989501... ft. Use a few well-known values to lock down the
    // conversion; round-trip check below catches drift independently.
    UnitConfig cfg;
    cfg.unit = ContourUnit::Feet;
    EXPECT_NEAR(metersToUnit(1.0, cfg), 3.2808398950131, 1e-9);
    EXPECT_NEAR(metersToUnit(100.0, cfg), 328.0839895013, 1e-6);
    EXPECT_DOUBLE_EQ(metersToUnit(0.0, cfg), 0.0);
}

TEST(ContourUnits, CustomMultiplierScalesMeters) {
    UnitConfig cfg;
    cfg.unit = ContourUnit::Custom;
    cfg.customMultiplier = 0.5;
    EXPECT_DOUBLE_EQ(metersToUnit(100.0, cfg), 50.0);
    EXPECT_DOUBLE_EQ(metersToUnit(-200.0, cfg), -100.0);
}

TEST(ContourUnits, CustomMultiplierZeroProducesZero) {
    UnitConfig cfg;
    cfg.unit = ContourUnit::Custom;
    cfg.customMultiplier = 0.0;
    EXPECT_DOUBLE_EQ(metersToUnit(123.0, cfg), 0.0);
    // Zero-multiplier round-trip is undefined (division by zero in
    // unitToMeters), so don't try to invert it.
}

TEST(ContourUnits, RoundTripMetersFeetMeters) {
    UnitConfig cfg;
    cfg.unit = ContourUnit::Feet;
    for (double m : {0.0, 1.0, -1.0, 100.0, 9999.0, -500.5}) {
        const double feet = metersToUnit(m, cfg);
        const double back = unitToMeters(feet, cfg);
        EXPECT_NEAR(back, m, 1e-9) << "round-trip failed for " << m;
    }
}

TEST(ContourUnits, RoundTripCustomMultiplier) {
    UnitConfig cfg;
    cfg.unit = ContourUnit::Custom;
    cfg.customMultiplier = 2.5;
    for (double m : {0.0, 4.0, -16.0, 99.5}) {
        const double scaled = metersToUnit(m, cfg);
        const double back = unitToMeters(scaled, cfg);
        EXPECT_NEAR(back, m, 1e-12) << "round-trip failed for " << m;
    }
}
