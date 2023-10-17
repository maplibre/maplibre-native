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

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/mtl/context.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>
#endif // MLN_RENDER_BACKEND_METAL

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
                                 uint64_t frameCount_)
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
#ifndef NDEBUG
      programs((debugOptions & MapDebugOptions::Overdraw) ? staticData_.overdrawPrograms : staticData_.programs),
#else
      programs(staticData_.programs),
#endif
      shaders(*staticData_.shaders),
      frameCount(frameCount_) {
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

gfx::DepthMode PaintParameters::depthModeForSublayer(uint8_t n, gfx::DepthMaskType mask) const {
    if (currentLayer < opaquePassCutoff) {
        return gfx::DepthMode::disabled();
    }
    float depth = depthRangeSize + ((1 + currentLayer) * numSublayers + n) * depthEpsilon;
    return gfx::DepthMode{gfx::DepthFunctionType::LessEqual, mask, {depth, depth}};
}

gfx::DepthMode PaintParameters::depthModeFor3D() const {
    return gfx::DepthMode{gfx::DepthFunctionType::LessEqual, gfx::DepthMaskType::ReadWrite, {0.0, depthRangeSize}};
}

namespace {

template <typename TIter>
using GetTileIDFunc = const UnwrappedTileID& (*)(const typename TIter::value_type&);

using TileMaskIDMap = std::map<UnwrappedTileID, int32_t>;

// `std::includes` adapted to extract tile IDs from arbitrary collection iterators
template <class I1, class I2>
bool includes(I1 beg1, const I1 end1, I2 beg2, const I2 end2, const GetTileIDFunc<I2>& unwrap) {
    for (; beg2 != end2; ++beg1) {
        if (beg1 == end1) {
            return false;
        }
        const auto id2 = unwrap(*beg2);
        if (id2 < beg1->first) {
            return false;
        } else if (!(beg1->first < id2)) {
            ++beg2;
        }
    }
    return true;
}

// Check whether the given set of tile IDs is a subset of the ones already rendered
template <typename TIter>
bool tileIDsCovered(TIter beg, TIter end, GetTileIDFunc<TIter>& unwrap, const TileMaskIDMap& idMap) {
    if (idMap.size() < static_cast<std::size_t>(std::distance(beg, end))) {
        return false;
    }
    assert(std::is_sorted(beg, end, [=](const auto& a, const auto& b) { return unwrap(a) < unwrap(b); }));
    return includes(idMap.cbegin(), idMap.cend(), beg, end, unwrap);
}

#if MLN_LEGACY_RENDERER
const UnwrappedTileID& unwrapRenderTiles(const RenderTiles::element_type::value_type& iter) {
    return iter.get().id;
}
#else  // !MLN_LEGACY_RENDERER
const UnwrappedTileID& unwrapIdentity(const UnwrappedTileID& value) {
    return value;
}
#endif // MLN_LEGACY_RENDERER
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
        shaders::ClipUBO{/*.matrix=*/util::cast<float>(matrixForTile({0, 0, 0})),
                         /*.stencil_ref=*/0,
                         /*.pad=*/0,
                         0,
                         0}};
    mtlContext.renderTileClippingMasks(*renderPass, staticData, tileUBO);
#else // !MLN_RENDER_BACKEND_METAL
    context.clearStencilBuffer(0b00000000);
#endif
}

#if MLN_LEGACY_RENDERER
void PaintParameters::renderTileClippingMasks(const RenderTiles& renderTiles) {
    renderTileClippingMasks((*renderTiles).cbegin(), (*renderTiles).cend(), &unwrapRenderTiles);
}
#else  // !MLN_LEGACY_RENDERER
void PaintParameters::renderTileClippingMasks(const std::set<UnwrappedTileID>& tileIDs) {
    renderTileClippingMasks(tileIDs.cbegin(), tileIDs.cend(), &unwrapIdentity);
}
#endif // MLN_LEGACY_RENDERER

void PaintParameters::clearTileClippingMasks() {
    if (!tileClippingMaskIDs.empty()) {
        clearStencil();
    }
}

template <typename TIter>
void PaintParameters::renderTileClippingMasks(TIter beg, TIter end, GetTileIDFunc<TIter> unwrap) {
    if (!renderPass) {
        assert(false);
        return;
    }

    if (tileIDsCovered(beg, end, unwrap, tileClippingMaskIDs)) {
        // The current stencil mask is for this source already; no need to draw another one.
        return;
    }

    const auto count = std::distance(beg, end);
    if (nextStencilID + count > maxStencilValue) {
        // we'll run out of fresh IDs so we need to clear and start from scratch
        clearStencil();
    }

#if MLN_RENDER_BACKEND_METAL
    // Assign a stencil ID and build a UBO for each tile in the set
    std::vector<shaders::ClipUBO> tileUBOs;
    for (auto i = beg; i != end; ++i) {
        const auto& tileID = unwrap(*i);

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
            tileUBOs.reserve(std::distance(beg, end));
        }

        tileUBOs.emplace_back(shaders::ClipUBO{/*.matrix=*/util::cast<float>(matrixForTile(tileID)),
                                               /*.stencil_ref=*/stencilID,
                                               /*.pad=*/0,
                                               0,
                                               0});
    }

    if (!tileUBOs.empty()) {
#if !defined(NDEBUG)
        const auto debugGroup = renderPass->createDebugGroup("tile-clip-masks");
#endif

        auto& mtlContext = static_cast<mtl::Context&>(context);
        mtlContext.renderTileClippingMasks(*renderPass, staticData, tileUBOs);
    }

#else  // !MLN_RENDER_BACKEND_METAL
    auto program = staticData.shaders->getLegacyGroup().get<ClippingMaskProgram>();

    if (!program) {
        return;
    }

    static_cast<gl::Context&>(context).renderingStats().stencilUpdates++;

    const style::Properties<>::PossiblyEvaluated properties{};
    const ClippingMaskProgram::Binders paintAttributeData(properties, 0);

    for (auto i = beg; i != end; ++i) {
        const auto& tileID = unwrap(*i);

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
                      ClippingMaskProgram::TextureBindings{},
                      "clipping/" + util::toString(stencilID));
    }
#endif // MLN_RENDER_BACKEND_METAL
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
