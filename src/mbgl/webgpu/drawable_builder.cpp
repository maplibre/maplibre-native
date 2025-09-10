#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/webgpu/drawable.hpp>
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

} // namespace webgpu
} // namespace mbgl