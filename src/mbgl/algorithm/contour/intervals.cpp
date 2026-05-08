#include <mbgl/algorithm/contour/intervals.hpp>

#include <cstddef>

namespace mbgl {
namespace algorithm {
namespace contour {

double intervalForZoom(const IntervalSchedule& schedule, double zoom) {
    const auto& s = schedule.stops;
    if (s.empty() || (s.size() % 2) == 0) {
        return 0.0;
    }
    // Walk the stops left-to-right. Output indices are 0, 2, 4, …; stop
    // indices are 1, 3, 5, …. The final entry is the output above the last
    // stop. Linear scan is fine — schedules in real configs have ≤ ~6 stops.
    for (std::size_t i = 1; i < s.size(); i += 2) {
        if (zoom < s[i]) {
            return s[i - 1];
        }
    }
    return s.back();
}

bool isValid(const IntervalSchedule& schedule) {
    const auto& s = schedule.stops;
    if (s.empty() || (s.size() % 2) == 0) {
        return false;
    }
    if (s[0] <= 0.0) {
        return false;
    }
    for (std::size_t i = 1; i < s.size(); i += 2) {
        // Output above this stop must be > 0.
        if (s[i + 1] <= 0.0) {
            return false;
        }
        // Stops must be strictly increasing.
        if (i + 2 < s.size() && s[i + 2] <= s[i]) {
            return false;
        }
    }
    return true;
}

} // namespace contour
} // namespace algorithm
} // namespace mbgl
