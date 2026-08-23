#pragma once

#include <memory>

namespace mln {

namespace gfx {

class DrawableData {
public:
    virtual ~DrawableData() = default;
};

using UniqueDrawableData = std::unique_ptr<DrawableData>;

} // namespace gfx
} // namespace mln
