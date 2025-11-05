#include <mbgl/renderer/paint_parameters.hpp>

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

#if MLN_RENDER_BACKEND_OPENGL
#include <mbgl/shaders/gl/legacy/clipping_mask_program.hpp>
#endif

#if MLN_RENDER_BACKEND_WEBGPU
#include <mbgl/webgpu/context.hpp>
#include <mbgl/shaders/webgpu/clipping_mask.hpp>
#endif

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/mtl/context.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>
#endif // MLN_RENDER_BACKEND_METAL

#if MLN_RENDER_BACKEND_VULKAN
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/shaders/vulkan/clipping_mask.hpp>
#include <mbgl/vulkan/context.hpp>
#endif // MLN_RENDER_BACKEND_VULKAN

namespace mbgl {

TransformParameters::TransformParameters(const TransformState& state_)
    : state(state_) {
    // Update the default matrices to the current viewport dimensions.
    state.getProjMatrix(projMatrix);

    // Also compute a projection matrix that aligns with the current pixel grid,
    // taking into account odd viewport sizes.
    state.getProjMatrix(alignedProjMatrix, 1, true);

    // Calculate a second projection matrix with the near plane moved further,
    // to a tenth of the far value, so as not to waste depth buffer precision on
    // very close empty space, for layer types (fill-extrusion) that use the
    // depth buffer to emulate real-world space.
    state.getProjMatrix(nearClippedProjMatrix, static_cast<uint16_t>(0.1 * state.getCameraToCenterDistance()));
}

PaintParameters::PaintParameters(gfx::Context& context_,
                                 float pixelRatio_,
                                 gfx::RendererBackend& backend_,
                                 const EvaluatedLight& evaluatedLight_,
                                 MapMode mode_,
                                 MapDebugOptions debugOptions_,
                                 TimePoint timePoint_,
                                 const TransformParameters& transformParams_,
                                 RenderStaticData& staticData_,
                                 LineAtlas& lineAtlas_,
                                 PatternAtlas& patternAtlas_,
                                 uint64_t frameCount_,
                                 double tileLodMinRadius_,
                                 double tileLodScale_,
                                 double tileLodPitchThreshold_,
                                 const gfx::ScissorRect& scissorRect_)
    : context(context_),
      backend(backend_),
      encoder(context.createCommandEncoder()),
      transformParams(transformParams_),
      state(transformParams_.state),
      evaluatedLight(evaluatedLight_),
      staticData(staticData_),
      lineAtlas(lineAtlas_),
      patternAtlas(patternAtlas_),
      mapMode(mode_),
      debugOptions(debugOptions_),
      timePoint(timePoint_),
      pixelRatio(pixelRatio_),
      shaders(*staticData_.shaders),
      frameCount(frameCount_),
      tileLodMinRadius(tileLodMinRadius_),
      tileLodScale(tileLodScale_),
      tileLodPitchThreshold(tileLodPitchThreshold_),
      scissorRect(scissorRect_) {
    pixelsToGLUnits = {{2.0f / state.getSize().width, -2.0f / state.getSize().height}};

    if (state.getViewportMode() == ViewportMode::FlippedY) {
        pixelsToGLUnits[1] *= -1;
    }
}

PaintParameters::~PaintParameters() = default;

mat4 PaintParameters::matrixForTile(const UnwrappedTileID& tileID, bool aligned) const {
    mat4 matrix;
    state.matrixFor(matrix, tileID);
    matrix::multiply(matrix, aligned ? transformParams.alignedProjMatrix : transformParams.projMatrix, matrix);
    return matrix;
}

gfx::DepthMode PaintParameters::depthModeForSublayer([[maybe_unused]] uint8_t n, gfx::DepthMaskType mask) const {
    if (currentLayer < opaquePassCutoff) {
        return gfx::DepthMode::disabled();
    }

#if MLN_RENDER_BACKEND_OPENGL
    float depth = depthRangeSize + ((1 + currentLayer) * numSublayers + n) * depthEpsilon;
    return gfx::DepthMode{gfx::DepthFunctionType::LessEqual, mask, {depth, depth}};
#else
    return gfx::DepthMode{gfx::DepthFunctionType::LessEqual, mask};
#endif
}

gfx::DepthMode PaintParameters::depthModeFor3D() const {
#if MLN_RENDER_BACKEND_OPENGL
    return gfx::DepthMode{gfx::DepthFunctionType::LessEqual, gfx::DepthMaskType::ReadWrite, {0.0, depthRangeSize}};
#else
    return gfx::DepthMode{gfx::DepthFunctionType::LessEqual, gfx::DepthMaskType::ReadWrite};
#endif
}

namespace {

template <typename TIter>
using GetTileIDFunc = const UnwrappedTileID& (*)(const typename TIter::value_type&);

using TileMaskIDMap = std::map<UnwrappedTileID, int32_t>;

// Check whether we can reuse a clip mask for a new set of tiles
bool tileIDsCovered(const RenderTiles& tiles, const TileMaskIDMap& idMap) {
    return idMap.size() == tiles->size() &&
           std::equal(idMap.cbegin(), idMap.cend(), tiles->cbegin(), tiles->cend(), [=](const auto& a, const auto& b) {
               return a.first == b.get().id;
           });
}

} // namespace

void PaintParameters::clearStencil() {
    nextStencilID = 1;
    tileClippingMaskIDs.clear();

#if MLN_RENDER_BACKEND_METAL
    auto& mtlContext = static_cast<mtl::Context&>(context);

    // Metal doesn't have an equivalent of `glClear`, so we clear the buffer by drawing zero to (0:0,0)
#if !defined(NDEBUG)
    const auto debugGroup = renderPass->createDebugGroup("tile-clip-mask-clear");
#endif

    const std::vector<shaders::ClipUBO> tileUBO = {
        shaders::ClipUBO{/* .matrix = */ util::cast<float>(matrixForTile({0, 0, 0})),
                         /* .stencil_ref = */ 0,
                         /* .pad1 = */ 0,
                         /* .pad2 = */ 0,
                         /* .pad3 = */ 0}};
    mtlContext.renderTileClippingMasks(*renderPass, staticData, tileUBO);
    context.renderingStats().stencilClears++;
#elif MLN_RENDER_BACKEND_VULKAN
    const auto& vulkanRenderPass = static_cast<vulkan::RenderPass&>(*renderPass);
    vulkanRenderPass.clearStencil();

    context.renderingStats().stencilClears++;
#elif MLN_RENDER_BACKEND_WEBGPU
    // WebGPU clears stencil through render pass descriptor
    context.renderingStats().stencilClears++;
#elif MLN_RENDER_BACKEND_OPENGL
    context.clearStencilBuffer(0b00000000);
#endif
}

void PaintParameters::renderTileClippingMasks(const RenderTiles& renderTiles) {
    // We can avoid updating the mask if it already contains the same set of tiles.
    if (!renderTiles || !renderPass || tileIDsCovered(renderTiles, tileClippingMaskIDs)) {
        return;
    }

    tileClippingMaskIDs.clear();

    // If the stencil value will overflow, clear the target to ensure ensure that none of the new
    // values remain set somewhere in it. Otherwise we can continue to overwrite it incrementally.
    const auto count = renderTiles->size();
    if (nextStencilID + count > maxStencilValue) {
        clearStencil();
    }

#if MLN_RENDER_BACKEND_WEBGPU
    std::vector<shaders::ClipUBO> tileUBOs;
    for (const auto& tileRef : *renderTiles) {
        const auto& tileID = tileRef.get().id;

        const int32_t stencilID = nextStencilID;
        const auto result = tileClippingMaskIDs.insert(std::make_pair(tileID, stencilID));
        if (result.second) {
            nextStencilID++;
        } else {
            continue;
        }

        if (tileUBOs.empty()) {
            tileUBOs.reserve(count);
        }

        tileUBOs.emplace_back(shaders::ClipUBO{/* .matrix = */ util::cast<float>(matrixForTile(tileID)),
                                               /* .stencil_ref = */ static_cast<uint32_t>(stencilID),
                                               /* .pad1 = */ 0,
                                               /* .pad2 = */ 0,
                                               /* .pad3 = */ 0});
    }

    if (!tileUBOs.empty()) {
#if !defined(NDEBUG)
        const auto debugGroup = renderPass->createDebugGroup("tile-clip-masks");
#endif

        auto& webgpuContext = static_cast<webgpu::Context&>(context);
        webgpuContext.renderTileClippingMasks(*renderPass, staticData, tileUBOs);
        webgpuContext.renderingStats().stencilUpdates++;
    }
#elif MLN_RENDER_BACKEND_METAL
    // Assign a stencil ID and build a UBO for each tile in the set
    std::vector<shaders::ClipUBO> tileUBOs;
    for (const auto& tileRef : *renderTiles) {
        const auto& tileID = tileRef.get().id;

        const int32_t stencilID = nextStencilID;
        const auto result = tileClippingMaskIDs.insert(std::make_pair(tileID, stencilID));
        if (result.second) {
            // inserted
            nextStencilID++;
        } else {
            // already present
            continue;
        }

        if (tileUBOs.empty()) {
            tileUBOs.reserve(count);
        }

        tileUBOs.emplace_back(shaders::ClipUBO{/* .matrix = */ util::cast<float>(matrixForTile(tileID)),
                                               /* .stencil_ref = */ static_cast<uint32_t>(stencilID),
                                               /* .pad1 = */ 0,
                                               /* .pad2 = */ 0,
                                               /* .pad3 = */ 0});
    }

    if (!tileUBOs.empty()) {
#if !defined(NDEBUG)
        const auto debugGroup = renderPass->createDebugGroup("tile-clip-masks");
#endif

        auto& mtlContext = static_cast<mtl::Context&>(context);
        mtlContext.renderTileClippingMasks(*renderPass, staticData, tileUBOs);

        mtlContext.renderingStats().stencilUpdates++;
    }

#elif MLN_RENDER_BACKEND_VULKAN

    std::vector<shaders::ClipUBO> tileUBOs;
    for (const auto& tileRef : *renderTiles) {
        const auto& tileID = tileRef.get().id;

        const uint32_t stencilID = nextStencilID;
        const auto result = tileClippingMaskIDs.insert(std::make_pair(tileID, stencilID));
        if (result.second) {
            // inserted
            nextStencilID++;
        } else {
            // already present
            continue;
        }

        if (tileUBOs.empty()) {
            tileUBOs.reserve(count);
        }

        tileUBOs.emplace_back(shaders::ClipUBO{matrixForTile(tileID), stencilID});
    }

    if (!tileUBOs.empty()) {
#if !defined(NDEBUG)
        const auto debugGroup = renderPass->createDebugGroup("tile-clip-masks");
#endif

        auto& vulkanContext = static_cast<vulkan::Context&>(context);
        vulkanContext.renderTileClippingMasks(*renderPass, staticData, tileUBOs);
        vulkanContext.renderingStats().stencilUpdates++;
    }

#elif MLN_RENDER_BACKEND_OPENGL
    auto program = staticData.shaders->getLegacyGroup().get<ClippingMaskProgram>();

    if (!program) {
        return;
    }

    static_cast<gl::Context&>(context).renderingStats().stencilUpdates++;

    const style::Properties<>::PossiblyEvaluated properties{};
    const ClippingMaskProgram::Binders paintAttributeData(properties, 0);

    for (const auto& tileRef : *renderTiles) {
        const auto& tileID = tileRef.get().id;

        const int32_t stencilID = nextStencilID;
        const auto result = tileClippingMaskIDs.insert(std::make_pair(tileID, stencilID));
        if (result.second) {
            // inserted
            nextStencilID++;
        } else {
            // already present
            continue;
        }

        program->draw(context,
                      *renderPass,
                      gfx::Triangles(),
                      gfx::DepthMode::disabled(),
                      gfx::StencilMode{gfx::StencilMode::Always{},
                                       stencilID,
                                       0b11111111,
                                       gfx::StencilOpType::Keep,
                                       gfx::StencilOpType::Keep,
                                       gfx::StencilOpType::Replace},
                      gfx::ColorMode::disabled(),
                      gfx::CullFaceMode::disabled(),
                      *staticData.quadTriangleIndexBuffer,
                      staticData.clippingMaskSegments,
                      ClippingMaskProgram::computeAllUniformValues(
                          ClippingMaskProgram::LayoutUniformValues{
                              uniforms::matrix::Value(matrixForTile(tileID)),
                          },
                          paintAttributeData,
                          properties,
                          static_cast<float>(state.getZoom())),
                      ClippingMaskProgram::computeAllAttributeBindings(
                          *staticData.tileVertexBuffer, paintAttributeData, properties),
                      "clipping/" + util::toString(stencilID));
    }
#endif // MLN_RENDER_BACKEND_OPENGL
}

gfx::StencilMode PaintParameters::stencilModeForClipping(const UnwrappedTileID& tileID) const {
    auto it = tileClippingMaskIDs.find(tileID);
    assert(it != tileClippingMaskIDs.end());
    const int32_t id = it != tileClippingMaskIDs.end() ? it->second : 0b00000000;
    return gfx::StencilMode{gfx::StencilMode::Equal{0b11111111},
                            id,
                            0b00000000,
                            gfx::StencilOpType::Keep,
                            gfx::StencilOpType::Keep,
                            gfx::StencilOpType::Replace};
}

gfx::StencilMode PaintParameters::stencilModeFor3D() {
    if (nextStencilID + 1 > maxStencilValue) {
        clearStencil();
    }

    // We're potentially destroying the stencil clipping mask in this pass. That
    // means we'll have to recreate it for the next source if any.
    tileClippingMaskIDs.clear();

    const int32_t id = nextStencilID++;
    return gfx::StencilMode{gfx::StencilMode::NotEqual{0b11111111},
                            id,
                            0b11111111,
                            gfx::StencilOpType::Keep,
                            gfx::StencilOpType::Keep,
                            gfx::StencilOpType::Replace};
}

gfx::ColorMode PaintParameters::colorModeForRenderPass() const {
    if (debugOptions & MapDebugOptions::Overdraw) {
        constexpr float overdraw = 1.0f / 8.0f;
        return gfx::ColorMode{
            gfx::ColorMode::Add{gfx::ColorBlendFactorType::ConstantColor, gfx::ColorBlendFactorType::One},
            Color{overdraw, overdraw, overdraw, 0.0f},
            gfx::ColorMode::Mask{true, true, true, true}};
    } else if (pass == RenderPass::Translucent) {
        return gfx::ColorMode::alphaBlended();
    } else {
        return gfx::ColorMode::unblended();
    }
}

} // namespace mbgl
