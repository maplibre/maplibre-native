#pragma once

#include <string>
#include <vector>

namespace mbgl {
namespace bench {

struct Location {
    const std::string name;
    const double longitude;
    const double latitude;
    const double zoom;
    const double bearing;
};

extern const std::vector<Location> locations;

} // namespace bench
} // namespace mbgl
