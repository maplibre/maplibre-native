#include <mbgl/algorithm/contour/units.hpp>

namespace mbgl {
namespace algorithm {
namespace contour {

namespace {
// 1 international foot = 0.3048 m (exact, by definition since 1959).
constexpr double kMetersPerFoot = 0.3048;
} // namespace

double metersToUnit(double meters, const UnitConfig& cfg) {
    switch (cfg.unit) {
        case ContourUnit::Meters:
            return meters;
        case ContourUnit::Feet:
            return meters / kMetersPerFoot;
        case ContourUnit::Custom:
            return meters * cfg.customMultiplier;
    }
    return meters;
}

double unitToMeters(double value, const UnitConfig& cfg) {
    switch (cfg.unit) {
        case ContourUnit::Meters:
            return value;
        case ContourUnit::Feet:
            return value * kMetersPerFoot;
        case ContourUnit::Custom:
            return value / cfg.customMultiplier;
    }
    return value;
}

} // namespace contour
} // namespace algorithm
} // namespace mbgl
