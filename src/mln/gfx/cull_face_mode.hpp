#pragma once

#include <mln/gfx/types.hpp>

namespace mln {
namespace gfx {

class CullFaceMode {
public:
    bool enabled;
    CullFaceSideType side;
    CullFaceWindingType winding;

    static CullFaceMode disabled() {
        return {.enabled = false, .side = CullFaceSideType::Back, .winding = CullFaceWindingType::CounterClockwise};
    }

    static CullFaceMode backCCW() {
        return {.enabled = true, .side = CullFaceSideType::Back, .winding = CullFaceWindingType::CounterClockwise};
    }
};

} // namespace gfx
} // namespace mln
