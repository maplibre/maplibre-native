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

private:
    // TODO: Implement WebGPU-specific drawable configuration
    // Context* context = nullptr;
    // void* pipeline = nullptr;
    // void* vertexBuffer = nullptr;
    // void* indexBuffer = nullptr;
    // void* uniformBuffer = nullptr;
    // std::vector<void*> textures;
};

} // namespace webgpu
} // namespace mbgl