#pragma once

#include <mbgl/util/size.hpp>

#include <native_buffer/native_buffer.h>
#include <native_window/external_window.h>

#include <cstdint>
#include <limits>

namespace mbgl {
namespace ohos {

inline bool setNativeWindowBufferGeometry(OHNativeWindow* window, Size size) {
    if (window == nullptr || size.isEmpty() ||
        size.width > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()) ||
        size.height > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
        return false;
    }

    return OH_NativeWindow_NativeWindowHandleOpt(window,
                                                SET_BUFFER_GEOMETRY,
                                                static_cast<std::int32_t>(size.width),
                                                static_cast<std::int32_t>(size.height)) == 0;
}

} // namespace ohos
} // namespace mbgl
