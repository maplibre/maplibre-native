#pragma once

#include <mbgl/map/camera.hpp>

#include <optional>

namespace mbgl {
namespace ohos {

struct CameraBoundsOptions {
    LatLngBounds bounds;
    EdgeInsets padding;
    std::optional<double> bearing;
    std::optional<double> pitch;
};

} // namespace ohos
} // namespace mbgl
