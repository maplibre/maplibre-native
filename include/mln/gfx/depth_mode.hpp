#pragma once

#include <mln/gfx/types.hpp>
#include <mln/util/range.hpp>

namespace mln {
namespace gfx {

class DepthMode {
public:
    DepthFunctionType func;
    DepthMaskType mask;
#if MLN_RENDER_BACKEND_OPENGL
    Range<float> range;
#endif

#if MLN_RENDER_BACKEND_OPENGL
    static DepthMode disabled() { return DepthMode{DepthFunctionType::Always, DepthMaskType::ReadOnly, {0.0, 1.0}}; }
#else
    static DepthMode disabled() {
        return DepthMode{.func = DepthFunctionType::Always, .mask = DepthMaskType::ReadOnly};
    }
#endif
};

} // namespace gfx
} // namespace mln
