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

RenderTargetGL::RenderTargetGL(Context& context)
    : glContext(context) {}

RenderTargetGL::~RenderTargetGL() {
    if (id) {
        MBGL_CHECK_ERROR(glDeleteFramebuffers(1, &id));
        id = 0;
    }
}

void RenderTargetGL::render(RenderOrchestrator& orchestrator, const RenderTree& renderTree, PaintParameters& parameters) {
    
    const auto& viewportSize = parameters.staticData.backendSize;
    const auto size = Size{viewportSize.width / 4, viewportSize.height / 4};
    
    if (!texture) {
        texture = glContext.createTexture2D();
        texture->setSize(size);
        texture->setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::HalfFloat);
        texture->setSamplerConfiguration({gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        texture->create();
    }
    
    if (!id) {
        id = glContext.createFramebuffer(*texture);
    }
    
    glContext.bindFramebuffer = id;
    
    // Run layer tweakers to update any dynamic elements
    observeLayerGroups([&](LayerGroupBase& layerGroup) {
        if (layerGroup.getLayerTweaker()) {
            layerGroup.getLayerTweaker()->execute(layerGroup, renderTree, parameters);
        }
    });
    
    // draw layer groups, opaque pass
    parameters.pass = RenderPass::Opaque;
    parameters.currentLayer = 0;
    parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * parameters.numSublayers * PaintParameters::depthEpsilon;

    observeLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        parameters.currentLayer++;
    });
    
    // draw layer groups, translucent pass
    parameters.pass = RenderPass::Translucent;
    parameters.currentLayer = static_cast<int32_t>(numLayerGroups()) - 1;
    parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * parameters.numSublayers * PaintParameters::depthEpsilon;

    observeLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        if (parameters.currentLayer != 0) {
            parameters.currentLayer--;
        }
    });
}

} // namespace gl
} // namespace mbgl
