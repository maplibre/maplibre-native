#include <mbgl/style/layers/background_layer_impl.hpp>

#include <mbgl/gl/drawable_gl_builder.hpp>
#include <mbgl/gl/drawable_gl_tweaker.hpp>
#include <mbgl/renderer/change_request.hpp>

namespace mbgl {
namespace style {

bool BackgroundLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

void BackgroundLayer::Impl::update(UniqueChangeRequestVec& changes) const {
    if (!drawable /* && visible/enabled */) {
        auto builder = std::make_unique<gl::DrawableGLBuilder>();   // from GL-specific code via virtual method?
        builder->setShaderID("background_generic");
        builder->addTweaker(std::make_shared<gl::DrawableGLTweaker>()); // generally shared across drawables
        builder->addQuad(0, 0, util::EXTENT, util::EXTENT);
        builder->flush();

        changes.reserve(builder->getDrawables().size());
        for (auto &draw : builder->clearDrawables()) {
            drawable = draw;
            changes.emplace_back(std::make_unique<AddDrawableRequest>(std::move(draw)));
        }
    //} else if (drawable && !visible/enabled) {
    //    return { std::make_unique<RemoveDrawableRequest>(std::move(drawable)) };
    }
}

} // namespace style
} // namespace mbgl
