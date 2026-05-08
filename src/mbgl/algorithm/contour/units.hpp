#pragma once

// Unit conversion for contour-source output. The marching-squares algorithm
// operates in the height grid's source unit (metres for terrarium DEMs); the
// contour-tile builder converts threshold + label values to the
// style-configured display unit (`unit: "meters" | "feet" | <number>`) before
// feature emission. Pure C++, no MapLibre deps.

namespace mbgl {
namespace algorithm {
namespace contour {

enum class ContourUnit {
    Meters,
    Feet,
    Custom,
};

struct UnitConfig {
    ContourUnit unit = ContourUnit::Meters;
    // Used only when `unit == Custom`. Interpreted as
    // (display unit) = (metres) * customMultiplier.
    double customMultiplier = 1.0;
};

/// Convert a value in metres (the algorithm's source unit) to the configured
/// display unit. For `Meters` the result equals the input.
double metersToUnit(double meters, const UnitConfig& cfg);

/// Convert a display-unit value back to metres. Inverse of `metersToUnit`,
/// undefined for `Custom` with `customMultiplier == 0`.
double unitToMeters(double value, const UnitConfig& cfg);

} // namespace contour
} // namespace algorithm
} // namespace mbgl
