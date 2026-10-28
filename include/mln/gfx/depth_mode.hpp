#pragma once

#include <mln/gfx/types.hpp>
#include <mln/util/range.hpp>

namespace mln {
namespace gfx {

class DepthMode {
public:
    DepthFunctionType func;
    DepthMaskType mask;
    Range<float> range;

    static DepthMode disabled() { return DepthMode{DepthFunctionType::Always, DepthMaskType::ReadOnly, {0.0, 1.0}}; }
};

} // namespace gfx
} // namespace mln
