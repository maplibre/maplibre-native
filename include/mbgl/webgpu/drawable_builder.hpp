#pragma once

#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <memory>
#include <string>

namespace mbgl {
namespace webgpu {

class Context;

class DrawableBuilder : public gfx::DrawableBuilder {
public:
    explicit DrawableBuilder(std::string name);
    ~DrawableBuilder() override;

protected:
    std::unique_ptr<gfx::Drawable> createDrawable() const override;
    std::unique_ptr<gfx::Drawable::DrawSegment> createSegment(gfx::DrawMode, SegmentBase&&) override;
    void init() override;

private:
    // WebGPU-specific drawable configuration is handled in the drawable implementation
    // Pipeline, buffers, and textures are managed by the Context and Drawable classes
};

} // namespace webgpu
} // namespace mbgl
