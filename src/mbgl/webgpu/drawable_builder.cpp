#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/drawable_impl.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

DrawableBuilder::DrawableBuilder(std::string name)
    : gfx::DrawableBuilder(std::move(name)) {
}

DrawableBuilder::~DrawableBuilder() = default;

std::unique_ptr<gfx::Drawable> DrawableBuilder::createDrawable() const {
    return std::make_unique<Drawable>(name);
}

std::unique_ptr<gfx::Drawable::DrawSegment> DrawableBuilder::createSegment(gfx::DrawMode drawMode, SegmentBase&& segment) {
    return std::make_unique<Drawable::DrawSegment>(drawMode, std::move(segment));
}

void DrawableBuilder::init() {
    // WebGPU resources are initialized lazily when the drawable is created
    // Pipeline creation happens when the drawable is first drawn
}

} // namespace webgpu
} // namespace mbgl