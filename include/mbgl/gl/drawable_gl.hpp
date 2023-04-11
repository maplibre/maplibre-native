#pragma once

#include <mbgl/gfx/drawable.hpp>

#include <memory>

namespace mbgl {
namespace gl {

class DrawableGL : public gfx::Drawable {
    // is-a/has-a DrawScopeResource?

public:
    DrawableGL();
    virtual ~DrawableGL();

    void draw(const PaintParameters &) const override;

    void setVertData(std::vector<std::uint8_t> data) { vertData = std::move(data); }
    
protected:
    std::vector<std::uint8_t> vertData;

    class Impl;
    const std::unique_ptr<const Impl> impl;

    // For testing only.
    DrawableGL(std::unique_ptr<const Impl>);
};

} // namespace gl
} // namespace mbgl
