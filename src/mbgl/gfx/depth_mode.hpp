#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/util/range.hpp>

namespace mbgl {
namespace gfx {

class DepthMode {
public:
    DepthFunctionType func;
    DepthMaskType mask;
#if MLN_LEGACY_RENDERER
    Range<float> range;
#endif

#if MLN_LEGACY_RENDERER
    static DepthMode disabled() { return DepthMode{DepthFunctionType::Always, DepthMaskType::ReadOnly, {0.0, 1.0}}; }
#else
    static DepthMode disabled() { return DepthMode{DepthFunctionType::Always, DepthMaskType::ReadOnly}; }
#endif
};

} // namespace gfx
} // namespace mbgl
