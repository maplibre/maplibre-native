#pragma once

#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_light.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/scissor_rect.hpp>
#include <mbgl/util/mat4.hpp>

#include <array>
#include <functional>
#include <iterator>
#include <map>
#include <set>
#include <vector>

namespace mbgl {

class UpdateParameters;
class RenderStaticData;
class TransformState;
class ImageManager;
class LineAtlas;
class PatternAtlas;
class UnwrappedTileID;

namespace gfx {
class Context;
class RendererBackend;
class CommandEncoder;
class RenderPass;
class ShaderRegistry;
} // namespace gfx

class TransformParameters {
public:
    TransformParameters(const TransformState&);
    mat4 projMatrix;
    mat4 alignedProjMatrix;
    mat4 nearClippedProjMatrix;
    const TransformState state;
};

class PaintParameters {
public:
    PaintParameters(gfx::Context&,
                    float pixelRatio,
                    gfx::RendererBackend&,
                    const EvaluatedLight&,
                    MapMode,
                    MapDebugOptions,
                    TimePoint,
                    const TransformParameters&,
                    RenderStaticData&,
                    LineAtlas&,
                    PatternAtlas&,
                    uint64_t frameCount,
                    double tileLodMinRadius,
                    double tileLodScale,
                    double tileLodPitchThreshold,
                    const gfx::ScissorRect&);
    ~PaintParameters();

    gfx::Context& context;
    gfx::RendererBackend& backend;
    std::unique_ptr<gfx::CommandEncoder> encoder;
    std::unique_ptr<gfx::RenderPass> renderPass;

    const TransformParameters& transformParams;
    const TransformState& state;
    const EvaluatedLight& evaluatedLight;

    RenderStaticData& staticData;
    LineAtlas& lineAtlas;
    PatternAtlas& patternAtlas;

    RenderPass pass = RenderPass::Opaque;
    MapMode mapMode;
    MapDebugOptions debugOptions;
    TimePoint timePoint;

    float pixelRatio;
    std::array<float, 2> pixelsToGLUnits;

    gfx::ShaderRegistry& shaders;

    gfx::DepthMode depthModeForSublayer(uint8_t n, gfx::DepthMaskType) const;
    gfx::DepthMode depthModeFor3D() const;
    gfx::ColorMode colorModeForRenderPass() const;

    mat4 matrixForTile(const UnwrappedTileID&, bool aligned = false) const;

    // Stencil handling
public:
    void renderTileClippingMasks(const RenderTiles&);

    /// Clear the stencil buffer, even if there are no tile masks (for 3D)
    void clearStencil();

    /// @brief Get a stencil mode for rendering constrined to the specified tile ID.
    /// The tile ID must have been present in the set previously passed to `renderTileClippingMasks`
    gfx::StencilMode stencilModeForClipping(const UnwrappedTileID&) const;

    /// @brief Initialize a stencil mode for 3D rendering.
    /// @details Clears the tile stencil masks, so `stencilModeForClipping`
    ///          cannot be used until `renderTileClippingMasks` is called again.
    /// @return The stencil mode, each value is unique.
    gfx::StencilMode stencilModeFor3D();

private:
    template <typename TIter>
    using GetTileIDFunc = const UnwrappedTileID& (*)(const typename TIter::value_type&);
    template <typename TIter>
    void renderTileClippingMasks(TIter beg, TIter end, GetTileIDFunc<TIter> unwrap);

    // This needs to be an ordered map so that we have the same order as the renderTiles.
    std::map<UnwrappedTileID, int32_t> tileClippingMaskIDs;
    int32_t nextStencilID = 1;

public:
    uint32_t currentLayer;
    float depthRangeSize;
    uint32_t opaquePassCutoff = 0;
    float symbolFadeChange;
    const uint64_t frameCount;

    static constexpr int numSublayers = 3;
#if MLN_RENDER_BACKEND_OPENGL
    static constexpr float depthEpsilon = 1.0f / (1 << 16);
#else
    static constexpr float depthEpsilon = 1.0f / (1 << 11);
#endif
    static constexpr int maxStencilValue = 255;

    double tileLodMinRadius;
    double tileLodScale;
    double tileLodPitchThreshold;

    gfx::ScissorRect scissorRect;
};

} // namespace mbgl
