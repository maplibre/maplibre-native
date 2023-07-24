#include <mbgl/gl/render_target_gl.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/framebuffer.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/util/logging.hpp>

#include <cassert>

namespace mbgl {
namespace gl {

using namespace platform;

RenderTargetGL::RenderTargetGL(Context& context, const Size size, const gfx::TextureChannelDataType type)
    : glContext(context) {
    texture = glContext.createTexture2D();
    texture->setSize(size);
    texture->setFormat(gfx::TexturePixelType::RGBA, type);
    texture->setSamplerConfiguration(
        {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
}

RenderTargetGL::~RenderTargetGL() {
    if (framebuffer) {
        MBGL_CHECK_ERROR(glDeleteFramebuffers(1, &framebuffer->get()));
        framebuffer.reset();
    }
}

void RenderTargetGL::upload(gfx::UploadPass& uploadPass) {
    visitLayerGroups(([&](LayerGroupBase& layerGroup) { layerGroup.upload(uploadPass); }));
}

void RenderTargetGL::render(RenderOrchestrator& orchestrator,
                            const RenderTree& renderTree,
                            PaintParameters& parameters) {
    texture->create();
    if (!framebuffer) {
        framebuffer = std::make_shared<UniqueFramebuffer>(glContext.createFramebuffer(*texture));
    }

    glContext.bindFramebuffer = *framebuffer;
    glContext.activeTextureUnit = 0;
    glContext.scissorTest = false;
    glContext.viewport = {0, 0, texture->getSize()};
    glContext.clear(Color{0.0f, 0.0f, 0.0f, 1.0f}, {}, {});

    // Run layer tweakers to update any dynamic elements
    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        if (layerGroup.getLayerTweaker()) {
            layerGroup.getLayerTweaker()->execute(layerGroup, renderTree, parameters);
        }
    });

    // draw layer groups, opaque pass
    parameters.pass = RenderPass::Opaque;
    parameters.currentLayer = 0;
    parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * parameters.numSublayers * PaintParameters::depthEpsilon;

    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        parameters.currentLayer++;
    });

    // draw layer groups, translucent pass
    parameters.pass = RenderPass::Translucent;
    parameters.currentLayer = static_cast<int32_t>(numLayerGroups()) - 1;
    parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * parameters.numSublayers * PaintParameters::depthEpsilon;

    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        if (parameters.currentLayer != 0) {
            parameters.currentLayer--;
        }
    });
}

} // namespace gl
} // namespace mbgl
