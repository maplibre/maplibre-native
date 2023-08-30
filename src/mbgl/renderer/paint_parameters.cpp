#include <mbgl/renderer/paint_parameters.hpp>

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/update_parameters.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
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

void PaintParameters::clearStencil() {
    nextStencilID = 1;
    tileClippingMaskIDs.clear();

#if MLN_RENDER_BACKEND_METAL
    // Metal doesn't have an equivalent of `glClear`, so we clear the buffer by drawing zero to (0:0,0)
    std::set<UnwrappedTileID> ids { { 0, 0, 0 } };
    auto f = [](const auto& ii) -> const UnwrappedTileID& { return ii; };
    renderTileClippingMasks(ids.cbegin(), ids.cend(), std::move(f), /*clear=*/true);
#else
    context.clearStencilBuffer(0b00000000);
#endif

}

namespace {

template <typename TIter>
using GetTileIDFunc = std::function<const UnwrappedTileID&(const typename TIter::value_type&)>;
using TileMaskIDMap = std::map<UnwrappedTileID, int32_t>;

// Detects a difference in keys of renderTiles and tileClippingMaskIDs
template <typename TIter>
bool tileIDsIdentical(TIter beg, TIter end, GetTileIDFunc<TIter>& f, const TileMaskIDMap& idMap) {
    if (static_cast<std::size_t>(std::distance(beg, end)) != idMap.size()) {
        return false;
    }
    assert(std::is_sorted(beg, end, [&f](const auto& a, const auto& b) { return f(a) < f(b); }));
    return std::equal(beg, end, idMap.cbegin(), [&f](const auto& ii, const auto& pair) { return f(ii) == pair.first; });
}

} // namespace

void PaintParameters::renderTileClippingMasks(const std::set<UnwrappedTileID>& tileIDs) {
    auto f = [](const auto& ii) -> const UnwrappedTileID& { return ii; };
    renderTileClippingMasks(tileIDs.cbegin(), tileIDs.cend(), std::move(f), /*clear=*/false);
}

void PaintParameters::renderTileClippingMasks(const RenderTiles& renderTiles) {
    auto f = [](const auto& ii) -> const UnwrappedTileID& { return ii.get().id; };
    renderTileClippingMasks((*renderTiles).cbegin(), (*renderTiles).cend(), std::move(f), /*clear=*/false);
}

void PaintParameters::clearTileClippingMasks() {
    if (!tileClippingMaskIDs.empty()) {
        clearStencil();
    }
}

template <typename TIter>
void PaintParameters::renderTileClippingMasks(TIter beg, TIter end, GetTileIDFunc<TIter>&& f, bool clear) {
    if (tileIDsIdentical(beg, end, f, tileClippingMaskIDs)) {
        // The current stencil mask is for this source already; no need to draw another one.
        return;
    }

    const auto count = std::distance(beg, end);
    if (!clear && nextStencilID + count > maxStencilValue) {
        // we'll run out of fresh IDs so we need to clear and start from scratch
        clearStencil();
    }

    if (!renderPass) {
        assert(false);
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = renderPass->createDebugGroup("tile-clip-masks");
#endif

#if MLN_RENDER_BACKEND_METAL
    using ShaderClass = shaders::ShaderSource<shaders::BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>;
    const auto group = staticData.shaders->getShaderGroup(std::string(MLN_STRINGIZE(ClippingMaskProgram)));
    if (!group) {
        return;
    }
    const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(group->getOrCreateShader(context, {}));
    if (!shader) {
        return;
    }

    const auto& mtlContext = static_cast<mtl::Context&>(context);
    const auto& mtlShader = static_cast<const mtl::ShaderProgram&>(*shader);
    const auto& mtlRenderPass = static_cast<mtl::RenderPass&>(*renderPass);
    const auto& encoder = mtlRenderPass.getMetalEncoder();
    const auto colorMode = gfx::ColorMode::disabled();

    gfx::IndexVector<gfx::Triangles> indexes;
    std::optional<mtl::BufferResource> indexRes;

    // TODO: refactor to use `Context::makeDepthStencilState`
    const auto init = [&]() {
        // TODO: a lot of this can be cached
        // Create a vertex buffer from the fixed tile coordinates
        const auto vertices = RenderStaticData::tileVertices();
        constexpr auto vertexSize = sizeof(decltype(vertices)::Vertex::a1);
        auto vertexRes = mtlContext.createBuffer(vertices.data(), vertices.bytes(), gfx::BufferUsageType::StaticDraw);
        if (!vertexRes) {
            return;
        }
        encoder->setVertexBuffer(vertexRes.getMetalBuffer().get(), /*offset=*/0, ShaderClass::attributes[0].index);

        // A vertex descriptor tells Metal what's in the vertex buffer
        auto vertDesc = NS::RetainPtr(MTL::VertexDescriptor::vertexDescriptor());
        auto attribDesc = NS::TransferPtr(MTL::VertexAttributeDescriptor::alloc()->init());
        auto layoutDesc = NS::TransferPtr(MTL::VertexBufferLayoutDescriptor::alloc()->init());
        if (!vertDesc || !attribDesc || !layoutDesc) {
            return;
        }
        attribDesc->setBufferIndex(ShaderClass::attributes[0].index);
        attribDesc->setOffset(0);
        attribDesc->setFormat(MTL::VertexFormatShort2);
        layoutDesc->setStride(static_cast<NS::UInteger>(vertexSize));
        layoutDesc->setStepFunction(MTL::VertexStepFunctionPerVertex);
        layoutDesc->setStepRate(1);
        vertDesc->attributes()->setObject(attribDesc.get(), 0);
        vertDesc->layouts()->setObject(layoutDesc.get(), 0);

        // Create a buffer from the fixed tile indexes
        indexes = RenderStaticData::quadTriangleIndices();
        indexRes.emplace(mtlContext.createBuffer(indexes.data(), indexes.bytes(), gfx::BufferUsageType::StaticDraw));
        if (!indexRes || !*indexRes) {
            return;
        }

        // gfx::CullFaceMode::disabled();
        encoder->setCullMode(MTL::CullModeNone);

        auto stencilDescriptor = NS::TransferPtr(MTL::StencilDescriptor::alloc()->init());
        if (!stencilDescriptor) {
            return;
        }
        stencilDescriptor->setStencilCompareFunction(MTL::CompareFunction::CompareFunctionAlways);
        stencilDescriptor->setStencilFailureOperation(MTL::StencilOperation::StencilOperationKeep);
        stencilDescriptor->setDepthFailureOperation(MTL::StencilOperation::StencilOperationKeep);
        stencilDescriptor->setDepthStencilPassOperation(MTL::StencilOperation::StencilOperationReplace);
        stencilDescriptor->setReadMask(0);
        stencilDescriptor->setWriteMask(0b11111111);

        auto depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
        if (!depthStencilDescriptor) {
            return;
        }
        depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionAlways);
        depthStencilDescriptor->setDepthWriteEnabled(false);
        depthStencilDescriptor->setFrontFaceStencil(stencilDescriptor.get());
        depthStencilDescriptor->setBackFaceStencil(stencilDescriptor.get());

        auto& device = mtlContext.getBackend().getDevice();
        auto depthStencilState = NS::TransferPtr(device->newDepthStencilState(depthStencilDescriptor.get()));
        if (!depthStencilState) {
            return;
        }
        encoder->setDepthStencilState(depthStencilState.get());

        // Create a render pipeline state, telling Metal how to render the primitives
        const auto& renderPassDescriptor = mtlRenderPass.getDescriptor();
        if (auto state = mtlShader.getRenderPipelineState(renderPassDescriptor, vertDesc, colorMode)) {
            encoder->setRenderPipelineState(state.get());
        } else {
            return;
        }
    };

    // For each tile in the set...
    for (auto i = beg; i != end; ++i) {
        const auto& tileID = f(*i);

        const int32_t stencilID = clear ? 0 : nextStencilID;
        if (!clear) {
            const auto result = tileClippingMaskIDs.insert(std::make_pair(tileID, stencilID));
            if (result.second) {
                // inserted
                nextStencilID++;
            } else {
                // already present
                continue;
            }
        }

        if (!indexRes) {
            init();
        }

        encoder->setStencilReferenceValue(stencilID);

        const auto ubo = shaders::ClipUBO{/* .matrix = */ util::cast<float>(matrixForTile(tileID))};
        // Create a buffer for the per-tile UBO data
        if (auto uboRes = mtlContext.createBuffer(&ubo, sizeof(ubo), gfx::BufferUsageType::StaticDraw)) {
            encoder->setVertexBuffer(uboRes.getMetalBuffer().get(), /*offset=*/0, ShaderClass::uniforms[0].index);
        } else {
            break;
        }

        encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                       indexes.elements(),
                                       MTL::IndexType::IndexTypeUInt16,
                                       indexRes->getMetalBuffer().get(),
                                       /*indexOffset=*/0,
                                       /*instanceCount=*/1,
                                       /*baseVertex=*/0,
                                       /*baseInstance=*/0);
    }
#else  // !MLN_RENDER_BACKEND_METAL
    auto program = staticData.shaders->getLegacyGroup().get<ClippingMaskProgram>();

    if (!program) {
        return;
    }

    const style::Properties<>::PossiblyEvaluated properties{};
    const ClippingMaskProgram::Binders paintAttributeData(properties, 0);

    for (auto i = beg; i != end; ++i) {
        const auto& tileID = f(*i);

        const int32_t stencilID = nextStencilID + 1;
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
