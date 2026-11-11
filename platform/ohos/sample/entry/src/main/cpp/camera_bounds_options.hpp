#pragma once

#include <mln/map/camera.hpp>

#include <optional>

namespace mln {
namespace ohos {

struct CameraBoundsOptions {
    LatLngBounds bounds;
    EdgeInsets padding;
    std::optional<double> bearing;
    std::optional<double> pitch;
};

} // namespace ohos
} // namespace mln
