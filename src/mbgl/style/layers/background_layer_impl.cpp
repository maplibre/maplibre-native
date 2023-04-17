#include <mbgl/style/layers/background_layer_impl.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/drawable_gl_builder.hpp>
#include <mbgl/gl/drawable_gl_tweaker.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace style {

bool BackgroundLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

constexpr auto shaderName = "background_generic";

void BackgroundLayer::Impl::layerAdded(PaintParameters& parameters, UniqueChangeRequestVec& changes) const {
    if (!shader) {
        shader = parameters.shaders.get<gl::ShaderProgramGL>(shaderName);
    }

    buildDrawables(changes);
}

void BackgroundLayer::Impl::layerRemoved(PaintParameters&, UniqueChangeRequestVec& changes) const {
    if (drawable) {
        changes.emplace_back(std::make_unique<RemoveDrawableRequest>(drawable->getId()));
        drawable.reset();
    }
}

void BackgroundLayer::Impl::buildDrawables(UniqueChangeRequestVec& changes) const {

    if (!shader) {
        return;
    }

    auto builder = std::make_unique<gl::DrawableGLBuilder>();   // from GL-specific code via virtual method?
    builder->setShader(shader);
    builder->addTweaker(std::make_shared<gl::DrawableGLTweaker>()); // generally shared across drawables
    builder->addQuad(0, 0, util::EXTENT, util::EXTENT);
    builder->flush();

    changes.reserve(builder->getDrawables().size());
    for (auto &draw : builder->clearDrawables()) {
        drawable = draw;
        changes.emplace_back(std::make_unique<AddDrawableRequest>(std::move(draw)));
    }
}

void BackgroundLayer::Impl::update(UniqueChangeRequestVec& changes) const {
    if (!drawable /* && visible/enabled */) {
        buildDrawables(changes);
//    } else if (drawable && !visible/enabled) {
//        changes.emplace_back(std::make_unique<RemoveDrawableRequest>(drawable->getId()));
//        drawable.reset();
    }
}

} // namespace style
} // namespace mbgl
