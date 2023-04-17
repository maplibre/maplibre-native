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
    // TODO: this should happen a GL-specific part of map initialization
    auto &glContext = static_cast<gl::Context&>(parameters.context);
    auto shader = parameters.shaders.get<gl::ShaderProgramGL>(shaderName);
    if (!shader) {
        constexpr auto vert = R"(
            //#version 300 es
            precision highp float;
            uniform float a;
            uniform int b;
            uniform mat4 c;
            attribute vec3 pos;  //layout (location = 0) in vec3 pos;
            attribute float d;
            //attribute int e;
            attribute mat2 f;
            attribute mat4 g;
            void main() {
                gl_Position = vec4(a, g[0][0], f[0][0], d);
                gl_Position = vec4(pos, 1.0);
            })";
        constexpr auto frag = R"(
            //#version 300 es
            precision highp float;
            //out vec4 color;
            void main() {
                //color = vec4(1.0,0.0,0.0,1.0);
                gl_FragColor = vec4(1.0,0.0,0.0,1.0);
            })";

        try {
            // Compile
            shader = gl::ShaderProgramGL::create(glContext, shaderName, vert, frag);
            if (shader) {
                // Set default values
                if (auto *attr = shader->getVertexAttributes().get("a")) {
                    attr->set(0, 12.3f);
                }
                if (auto *attr = shader->getVertexAttributes().get("b")) {
                    attr->set(0, 123);
                }
                if (auto *attr = shader->getVertexAttributes().get("f")) {
                    attr->set(0, gfx::VertexAttribute::matf2{ 1.0f, 2.0f, 3.0f, 4.0f });
                }

                // Add to the registry
                if (!parameters.shaders.registerShader(shader, shaderName)) {
                    Log::Warning(Event::General, "Shader conflict - " + std::string(shaderName));
                    return;
                }
            } else {
                Log::Warning(Event::General, "Shader create failed - " + std::string(shaderName));
                return;
            }
    } catch (const std::runtime_error& ex) {
            Log::Warning(Event::General, "Shader create exception - " + std::string(ex.what()));
            return;
        }
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
    auto builder = std::make_unique<gl::DrawableGLBuilder>();   // from GL-specific code via virtual method?
    builder->setShaderID(shaderName);
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
