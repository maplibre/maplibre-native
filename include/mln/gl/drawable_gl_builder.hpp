#pragma once

#include <mln/gfx/drawable_builder.hpp>
#include <mln/gl/vertex_attribute_gl.hpp>

namespace mln {
namespace gl {

/**
    Base class for OpenGL-specific drawable builders.
 */
class DrawableGLBuilder final : public gfx::DrawableBuilder {
public:
    DrawableGLBuilder(std::string name_)
        : gfx::DrawableBuilder(std::move(name_)) {}
    ~DrawableGLBuilder() override = default;

    using DrawSegment = gfx::Drawable::DrawSegment;
    std::unique_ptr<DrawSegment> createSegment(gfx::DrawMode, SegmentBase&&) override;

protected:
    gfx::UniqueDrawable createDrawable() const override;

    /// Setup the SDK-specific aspects after all the values are present
    void init() override;
};

} // namespace gl
} // namespace mln
