#pragma once

#include <memory>

namespace mbgl {

namespace gfx {

class DrawableData {
public:
    virtual ~DrawableData() = default;
};

using UniqueDrawableData = std::unique_ptr<DrawableData>;

} // namespace gfx
} // namespace mbgl
