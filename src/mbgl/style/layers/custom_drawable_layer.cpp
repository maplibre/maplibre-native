#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/style/layers/custom_drawable_layer_impl.hpp>

#include <mbgl/renderer/layers/render_custom_drawable_layer.hpp>
#include <mbgl/style/layer_observer.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/gfx/fill_generator.hpp>
#include <mbgl/shaders/custom_drawable_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/shaders/widevector_ubo.hpp>
#include <mbgl/shaders/custom_geometry_ubo.hpp>
#include <mbgl/util/projection.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/gfx/uniform_buffer.hpp>

#include <cmath>

namespace mbgl {

namespace style {

using namespace shaders;

namespace {
const LayerTypeInfo typeInfoCustomDrawable{.type = "custom-drawable",
                                           .source = LayerTypeInfo::Source::NotRequired,
                                           .pass3d = LayerTypeInfo::Pass3D::NotRequired,
                                           .layout = LayerTypeInfo::Layout::NotRequired,
                                           .fadingTiles = LayerTypeInfo::FadingTiles::NotRequired,
                                           .crossTileIndex = LayerTypeInfo::CrossTileIndex::NotRequired,
                                           .tileKind = LayerTypeInfo::TileKind::NotRequired};
} // namespace

CustomDrawableLayer::CustomDrawableLayer(const std::string& layerID, std::unique_ptr<CustomDrawableLayerHost> host)
    : Layer(makeMutable<Impl>(layerID, std::move(host))) {}

CustomDrawableLayer::~CustomDrawableLayer() = default;

const CustomDrawableLayer::Impl& CustomDrawableLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<CustomDrawableLayer::Impl> CustomDrawableLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> CustomDrawableLayer::cloneRef(const std::string&) const {
    assert(false);
    return nullptr;
}

using namespace conversion;

std::optional<Error> CustomDrawableLayer::setPropertyInternal(const std::string&, const Convertible&) {
    return Error{"layer doesn't support this property"};
}

StyleProperty CustomDrawableLayer::getProperty(const std::string&) const {
    return {};
}

Mutable<Layer::Impl> CustomDrawableLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

// static
const LayerTypeInfo* CustomDrawableLayer::Impl::staticTypeInfo() noexcept {
    return &typeInfoCustomDrawable;
}

// CustomDrawableLayerHost::Interface

class LineDrawableTweaker : public gfx::DrawableTweaker {
public:
    LineDrawableTweaker(const CustomDrawableLayerHost::Interface::LineOptions& options_,
                        CustomDrawableLayerHost::Interface::LineTweakerCallback&& callback_)
        : options(options_),
          callback(callback_) {}

    ~LineDrawableTweaker() override = default;

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        if (callback) {
            callback(drawable, parameters, options);
        }

#if MLN_UBO_CONSOLIDATION
        if (!layerUniforms) {
            layerUniforms = parameters.context.createLayerUniformBufferArray();
        }
#endif

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto zoom = parameters.state.getZoom();

        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, drawable, false);

        const shaders::LineEvaluatedPropsUBO propsUBO = {.color = options.color,
                                                         .blur = options.blur,
                                                         .opacity = options.opacity,
                                                         .gapwidth = options.gapWidth,
                                                         .offset = options.offset,
                                                         .width = options.width,
                                                         /*floorwidth=*/.floorwidth = 0.0f,
                                                         .expressionMask = LineExpressionMask::None,
                                                         .pad1 = 0};

        // We would need to set up `idLineExpressionUBO` if the expression mask isn't empty
        assert(propsUBO.expressionMask == LineExpressionMask::None);

        if (!expressionUniformBuffer) {
            const LineExpressionUBO exprUBO = {
                /* .color = */ .color = nullptr,
                /* .blur = */ .blur = nullptr,
                /* .opacity = */ .opacity = nullptr,
                /* .gapwidth = */ .gapwidth = nullptr,
                /* .offset = */ .offset = nullptr,
                /* .width = */ .width = nullptr,
                /* .floorWidth = */ .floorWidth = nullptr,
            };

            expressionUniformBuffer = parameters.context.createUniformBuffer(&exprUBO, sizeof(exprUBO));
#if MLN_UBO_CONSOLIDATION
            layerUniforms->set(idLineExpressionUBO, expressionUniformBuffer);
#else
            auto& drawableUniforms = drawable.mutableUniformBuffers();
            drawableUniforms.set(idLineExpressionUBO, expressionUniformBuffer);
#endif
        }

#if MLN_UBO_CONSOLIDATION
        shaders::LineDrawableUnionUBO drawableUBO;
        drawableUBO.lineDrawableUBO = {
#else
        const shaders::LineDrawableUBO drawableUBO = {
#endif
            /* .matrix = */ .matrix = util::cast<float>(matrix),
            /* .ratio = */ .ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, zoom),

            /* .color_t = */ .color_t = 0.f,
            /* .blur_t = */ .blur_t = 0.f,
            /* .opacity_t = */ .opacity_t = 0.f,
            /* .gapwidth_t = */ .gapwidth_t = 0.f,
            /* .offset_t = */ .offset_t = 0.f,
            /* .width_t = */ .width_t = 0.f,
            /* .pad1 = */ .pad1 = 0
        };

#if MLN_UBO_CONSOLIDATION
        if (!drawableUniformBuffer) {
            drawableUniformBuffer = parameters.context.createUniformBuffer(
                &drawableUBO, sizeof(drawableUBO), false, true);

            layerUniforms->set(idLineDrawableUBO, drawableUniformBuffer);
            drawable.setUBOIndex(0);
        } else {
            drawableUniformBuffer->update(&drawableUBO, sizeof(drawableUBO));
        }

        layerUniforms->createOrUpdate(idLineEvaluatedPropsUBO, &propsUBO, sizeof(propsUBO), parameters.context);
        layerUniforms->bind(*parameters.renderPass);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idLineDrawableUBO, &drawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idLineEvaluatedPropsUBO, &propsUBO, parameters.context);
#endif
    };

private:
    CustomDrawableLayerHost::Interface::LineOptions options;
    CustomDrawableLayerHost::Interface::LineTweakerCallback callback;

#if MLN_UBO_CONSOLIDATION
    gfx::UniqueUniformBufferArray layerUniforms;
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif

    gfx::UniformBufferPtr expressionUniformBuffer;
};

class WideVectorDrawableTweaker : public gfx::DrawableTweaker {
public:
    WideVectorDrawableTweaker(const CustomDrawableLayerHost::Interface::LineOptions& options_,
                              CustomDrawableLayerHost::Interface::LineTweakerCallback&& callback_)
        : options(options_),
          callback(callback_) {}

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        if (callback) {
            callback(drawable, parameters, options);
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        mat4 tileMatrix;
        parameters.state.matrixFor(/*out*/ tileMatrix, tileID);
        if (const auto& origin{drawable.getOrigin()}; origin.has_value()) {
            matrix::translate(tileMatrix, tileMatrix, origin->x, origin->y, 0);
        }

        mat4 projMatrix = parameters.transformParams.projMatrix;
        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, drawable, false);

        matf4 mvpMatrix, mvpMatrixDiff, mvMatrix, mvMatrixDiff, pMatrix, pMatrixDiff;
        matrix::diffsplit(mvpMatrix, mvpMatrixDiff, matrix);
        matrix::diffsplit(mvMatrix, mvMatrixDiff, tileMatrix);
        matrix::diffsplit(pMatrix, pMatrixDiff, projMatrix);

        const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();
        shaders::WideVectorUniformsUBO uniform = {
            /* .mvpMatrix = */ .mvpMatrix = mvpMatrix,
            /* .mvpMatrixDiff = */ .mvpMatrixDiff = mvpMatrixDiff,
            /* .mvMatrix = */ .mvMatrix = mvMatrix,
            /* .mvMatrixDiff = */ .mvMatrixDiff = mvMatrixDiff,
            /* .pMatrix = */ .pMatrix = pMatrix,
            /* .pMatrixDiff = */ .pMatrixDiff = pMatrixDiff,
            /* .frameSize = */ .frameSize = {(float)renderableSize.width, (float)renderableSize.height},
            /* .pad1 = */ .pad1 = 0,
            /* .pad2 = */ .pad2 = 0};

        shaders::WideVectorUniformWideVecUBO wideVec = {
            /* .color = */ .color = options.color,
            /* .w2 = */ .w2 = options.width,
            /* .offset = */ .offset = options.offset,
            /* .edge = */ .edge = 0.0f,           // TODO: MLN does not provide a value. Analyze impact.
            /* .texRepeat = */ .texRepeat = 0.0f, // N/A
            /* .texOffset = */ .texOffset = {},   // N/A
            /* .miterLimit = */ .miterLimit = options.geometry.miterLimit,
            /* .join = */ .join = static_cast<int32_t>(options.geometry.joinType),
            /* .cap = */ .cap = static_cast<int32_t>(options.geometry.beginCap), // TODO: MLN option for endCap to be
                                                                                 // implemented in the shader!
            /* .hasExp = */ .hasExp = false,                                     // N/A
            /* .interClipLimit = */ .interClipLimit = 0.0f,                      // N/A
            /* .pad1 = */ .pad1 = 0};

        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idWideVectorUniformsUBO, &uniform, parameters.context);
        drawableUniforms.createOrUpdate(idWideVectorUniformWideVecUBO, &wideVec, parameters.context);
    };

private:
    CustomDrawableLayerHost::Interface::LineOptions options;
    CustomDrawableLayerHost::Interface::LineTweakerCallback callback;
};

class FillDrawableTweaker : public gfx::DrawableTweaker {
public:
    FillDrawableTweaker(const CustomDrawableLayerHost::Interface::FillOptions& options_,
                        CustomDrawableLayerHost::Interface::FillTweakerCallback&& callback_)
        : options(options_),
          callback(callback_) {}

    ~FillDrawableTweaker() override = default;

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        if (callback) {
            callback(drawable, parameters, options);
        }

#if MLN_UBO_CONSOLIDATION
        if (!layerUniforms) {
            layerUniforms = parameters.context.createLayerUniformBufferArray();
        }
#endif

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, drawable, false);

        const shaders::FillEvaluatedPropsUBO propsUBO = {/* .color = */ .color = options.color,
                                                         /* .outline_color = */ .outline_color = Color::white(),
                                                         /* .opacity = */ .opacity = options.opacity,
                                                         /* .fade = */ .fade = 0.f,
                                                         /* .from_scale = */ .from_scale = 0.f,
                                                         /* .to_scale = */ .to_scale = 0.f};

#if MLN_UBO_CONSOLIDATION
        FillDrawableUnionUBO drawableUBO;
        drawableUBO.fillDrawableUBO = {
#else
        const shaders::FillDrawableUBO drawableUBO = {
#endif
            /* .matrix = */ .matrix = util::cast<float>(matrix),

            /* .color_t = */ .color_t = 0.f,
            /* .opacity_t = */ .opacity_t = 0.f,
            /* .pad1 = */ .pad1 = 0,
            /* .pad2 = */ .pad2 = 0
        };

#if MLN_UBO_CONSOLIDATION
        if (!drawableUniformBuffer) {
            drawableUniformBuffer = parameters.context.createUniformBuffer(
                &drawableUBO, sizeof(drawableUBO), false, true);

            layerUniforms->set(idFillDrawableUBO, drawableUniformBuffer);
            drawable.setUBOIndex(0);
        } else {
            drawableUniformBuffer->update(&drawableUBO, sizeof(drawableUBO));
        }

        layerUniforms->createOrUpdate(idFillEvaluatedPropsUBO, &propsUBO, sizeof(propsUBO), parameters.context);
        layerUniforms->bind(*parameters.renderPass);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idFillEvaluatedPropsUBO, &propsUBO, parameters.context);
#endif
    };

private:
    CustomDrawableLayerHost::Interface::FillOptions options;
    CustomDrawableLayerHost::Interface::FillTweakerCallback callback;

#if MLN_UBO_CONSOLIDATION
    gfx::UniqueUniformBufferArray layerUniforms;
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

class SymbolDrawableTweaker : public gfx::DrawableTweaker {
public:
    SymbolDrawableTweaker(const CustomDrawableLayerHost::Interface::SymbolOptions& options_,
                          CustomDrawableLayerHost::Interface::SymbolTweakerCallback&& callback_)
        : options(options_),
          callback(callback_) {}
    ~SymbolDrawableTweaker() override = default;

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        if (callback) {
            callback(drawable, parameters, options);
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, drawable, false);

        const auto pixelsToTileUnits = tileID.pixelsToTileUnits(
            1.0f, options.scaleWithMap ? tileID.canonical.z : parameters.state.getZoom());
        const float factor = options.scaleWithMap
                                 ? static_cast<float>(std::pow(2.f, parameters.state.getZoom() - tileID.canonical.z))
                                 : 1.0f;
        const auto extrudeScale = options.pitchWithMap ? std::array<float, 2>{pixelsToTileUnits, pixelsToTileUnits}
                                                       : std::array<float, 2>{parameters.pixelsToGLUnits[0] * factor,
                                                                              parameters.pixelsToGLUnits[1] * factor};

        const shaders::CustomSymbolIconDrawableUBO drawableUBO = {
            /* .matrix = */ .matrix = util::cast<float>(matrix),
            /* .extrude_scale = */ .extrude_scale = {extrudeScale[0] * options.size.width,
                                                     extrudeScale[1] * options.size.height},
            /* .anchor = */ .anchor = options.anchor,
            /* .angle_degrees = */ .angle_degrees = options.angleDegrees,
            /* .scale_with_map = */ .scale_with_map = options.scaleWithMap,
            /* .pitch_with_map = */ .pitch_with_map = options.pitchWithMap,
            /* .camera_to_center_distance = */ .camera_to_center_distance =
                parameters.state.getCameraToCenterDistance(),
            /* .aspect_ratio = */ .aspect_ratio = parameters.pixelsToGLUnits[0] / parameters.pixelsToGLUnits[1],
            /* .pad1 = */ .pad1 = 0,
            /* .pad2 = */ .pad2 = 0,
            /* .pad3 = */ .pad3 = 0};

        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idCustomSymbolDrawableUBO, &drawableUBO, parameters.context);
    };

private:
    CustomDrawableLayerHost::Interface::SymbolOptions options;
    CustomDrawableLayerHost::Interface::SymbolTweakerCallback callback;
};

class GeometryDrawableTweaker : public gfx::DrawableTweaker {
public:
    GeometryDrawableTweaker(const CustomDrawableLayerHost::Interface::GeometryOptions& options_,
                            CustomDrawableLayerHost::Interface::GeometryTweakerCallback&& callback_)
        : options(options_),
          callback(callback_) {}
    ~GeometryDrawableTweaker() override = default;

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        if (callback) {
            callback(drawable, parameters, options);
        }

        CustomGeometryDrawableUBO drawableUBO = {/* .matrix = */ .matrix = util::cast<float>(options.matrix),
                                                 /* .color = */ .color = options.color};

        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idCustomGeometryDrawableUBO, &drawableUBO, parameters.context);
    };

private:
    CustomDrawableLayerHost::Interface::GeometryOptions options;
    CustomDrawableLayerHost::Interface::GeometryTweakerCallback callback;
};

CustomDrawableLayerHost::Interface::Interface(RenderLayer& layer_,
                                              LayerGroupBasePtr& layerGroup_,
                                              gfx::ShaderRegistry& shaders_,
                                              gfx::Context& context_,
                                              const TransformState& state_,
                                              const std::shared_ptr<UpdateParameters>& updateParameters_,
                                              const RenderTree& renderTree_,
                                              UniqueChangeRequestVec& changes_)
    : layer(layer_),
      layerGroup(layerGroup_),
      shaders(shaders_),
      context(context_),
      state(state_),
      updateParameters(updateParameters_),
      renderTree(renderTree_),
      changes(changes_) {
    // ensure we have a default layer group set up
    if (!layerGroup) {
        if (auto aLayerGroup = context.createTileLayerGroup(
                /*layerIndex*/ layer.getLayerIndex(), /*initialCapacity=*/64, layer.getID())) {
            changes.emplace_back(std::make_unique<AddLayerGroupRequest>(aLayerGroup));
            layerGroup = std::move(aLayerGroup);
        }
    }
}

std::size_t CustomDrawableLayerHost::Interface::getDrawableCount() const {
    return layerGroup->getDrawableCount();
}

void CustomDrawableLayerHost::Interface::setTileID(OverscaledTileID tileID_) {
    if (tileID != tileID_) finish();
    tileID = tileID_;
}

void CustomDrawableLayerHost::Interface::setLineOptions(const LineOptions& options) {
    finish();
    lineOptions = options;
}

void CustomDrawableLayerHost::Interface::setFillOptions(const FillOptions& options) {
    finish();
    fillOptions = options;
}

void CustomDrawableLayerHost::Interface::setSymbolOptions(const SymbolOptions& options) {
    finish();
    symbolOptions = options;
}

void CustomDrawableLayerHost::Interface::setGeometryOptions(const GeometryOptions& options) {
    finish();
    geometryOptions = options;
}

bool CustomDrawableLayerHost::Interface::updateBuilder(BuilderType type,
                                                       const std::string& name,
                                                       gfx::ShaderPtr shader) {
    if (!shader) return false;
    if (type != builderType || !builder || builder->getShader() != shader) {
        finish();
        builder = createBuilder(name, shader);
        builderType = type;
    }
    assert(builder);
    assert(builder->getShader() == shader);
    return true;
};

util::SimpleIdentity CustomDrawableLayerHost::Interface::addPolyline(const LineString<double>& coordinates,
                                                                     LineShaderType shaderType) {
#if !MLN_RENDER_BACKEND_METAL
    shaderType = LineShaderType::Classic;
#endif

    switch (shaderType) {
        case LineShaderType::Classic: {
            // build classic polyline with Geo coordinates
            if (!updateBuilder(BuilderType::LineClassic, "custom-lines", lineShaderDefault())) {
                return util::SimpleIdentity::Empty;
            }

            // geographic coordinates require tile {0, 0, 0}
            setTileID({0, 0, 0});

            constexpr int32_t zoom = 0;
            GeometryCoordinates tileCoordinates;
            for (const auto& coord : coordinates) {
                const auto point = Projection::project(LatLng(coord.y, coord.x), zoom);
                tileCoordinates.push_back(Point<int16_t>(static_cast<int16_t>(point.x * mbgl::util::EXTENT),
                                                         static_cast<int16_t>(point.y * mbgl::util::EXTENT)));
            }

            builder->addPolyline(tileCoordinates, lineOptions.geometry);
        } break;

        case LineShaderType::WideVector: {
            // build wide vector polyline with Geo coordinates
            if (!updateBuilder(BuilderType::LineWideVector, "custom-lines-widevector", lineShaderWideVector()))
                return util::SimpleIdentity::Empty;

            // geographic coordinates require tile {0, 0, 0}
            setTileID({0, 0, 0});

            builder->addWideVectorPolylineGlobal(coordinates, lineOptions.geometry);
        } break;
    }

    return builder->getCurrentDrawable(true)->getID();
}

util::SimpleIdentity CustomDrawableLayerHost::Interface::addPolyline(const GeometryCoordinates& coordinates,
                                                                     LineShaderType shaderType) {
#if !MLN_RENDER_BACKEND_METAL
    shaderType = LineShaderType::Classic;
#endif

    switch (shaderType) {
        case LineShaderType::Classic: {
            // build classic polyline with Tile coordinates
            if (!updateBuilder(BuilderType::LineClassic, "custom-lines", lineShaderDefault())) {
                return util::SimpleIdentity::Empty;
            }

            builder->addPolyline(coordinates, lineOptions.geometry);
        } break;

        case LineShaderType::WideVector: {
            // build wide vector polyline
            if (!updateBuilder(BuilderType::LineWideVector, "custom-lines-widevector", lineShaderWideVector()))
                return util::SimpleIdentity::Empty;

            builder->addWideVectorPolylineLocal(coordinates, lineOptions.geometry);
        } break;
    }

    return builder->getCurrentDrawable(true)->getID();
}

util::SimpleIdentity CustomDrawableLayerHost::Interface::addFill(const GeometryCollection& geometry) {
    // build fill
    if (!updateBuilder(BuilderType::Fill, "custom-fill", fillShaderDefault())) {
        return util::SimpleIdentity::Empty;
    }

    // provision buffers for fill vertices, indexes and segments
    using VertexVector = gfx::VertexVector<FillLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector triangleSegments;

    // generate fill geometry into buffers
    gfx::generateFillBuffers(geometry, vertices, triangles, triangleSegments);

    // add to builder
    auto attrs = context.createVertexAttributeArray();
    if (const auto& attr = attrs->set(idFillPosVertexAttribute)) {
        attr->setSharedRawData(sharedVertices,
                               offsetof(FillLayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(FillLayoutVertex),
                               gfx::AttributeDataType::Short2);
    }
    builder->setVertexAttributes(std::move(attrs));
    builder->setRawVertices({}, vertices.elements(), gfx::AttributeDataType::Short2);
    builder->setSegments(gfx::Triangles(), sharedTriangles, triangleSegments.data(), triangleSegments.size());

    const auto& id = builder->getCurrentDrawable(true)->getID();

    // flush current builder drawable
    builder->flush(context);

    return id;
}

util::SimpleIdentity CustomDrawableLayerHost::Interface::addSymbol(
    const GeometryCoordinate& point, const std::array<std::array<float, 2>, 2>& textureCoordinates) {
    // build symbol
    if (!updateBuilder(BuilderType::Symbol, "custom-symbol", symbolShaderDefault())) {
        return util::SimpleIdentity::Empty;
    }

    // temporary: buffers
    struct CustomSymbolIcon {
        std::array<float, 2> a_pos;
        std::array<float, 2> a_tex;
    };

    // vertices
    using VertexVector = gfx::VertexVector<CustomSymbolIcon>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    // encode center and extrude direction into vertices
    for (int y = 0; y <= 1; ++y) {
        for (int x = 0; x <= 1; ++x) {
            vertices.emplace_back(
                CustomSymbolIcon{.a_pos = {static_cast<float>(point.x * 2 + x), static_cast<float>(point.y * 2 + y)},
                                 .a_tex = {textureCoordinates[x][0], textureCoordinates[y][1]}});
        }
    }

    // indexes
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    triangles.emplace_back(0, 1, 2, 1, 2, 3);

    SegmentVector triangleSegments;
    triangleSegments.emplace_back(0, 0, 4, 6);

    // add to builder
    auto attrs = context.createVertexAttributeArray();
    if (const auto& attr = attrs->set(idCustomSymbolPosVertexAttribute)) {
        attr->setSharedRawData(sharedVertices,
                               offsetof(CustomSymbolIcon, a_pos),
                               /*vertexOffset=*/0,
                               sizeof(CustomSymbolIcon),
                               gfx::AttributeDataType::Float2);
    }
    if (const auto& attr = attrs->set(idCustomSymbolTexVertexAttribute)) {
        attr->setSharedRawData(sharedVertices,
                               offsetof(CustomSymbolIcon, a_tex),
                               /*vertexOffset=*/0,
                               sizeof(CustomSymbolIcon),
                               gfx::AttributeDataType::Float2);
    }
    builder->setVertexAttributes(std::move(attrs));
    builder->setRawVertices({}, vertices.elements(), gfx::AttributeDataType::Float2);
    builder->setSegments(gfx::Triangles(), sharedTriangles, triangleSegments.data(), triangleSegments.size());

    // texture
    if (symbolOptions.texture) {
        builder->setTexture(symbolOptions.texture, idCustomSymbolImageTexture);
    }

    // create symbol tweaker
    auto tweaker = std::make_shared<SymbolDrawableTweaker>(symbolOptions, std::move(symbolTweakerCallback));
    builder->addTweaker(tweaker);

    const auto& id = builder->getCurrentDrawable(true)->getID();

    // flush current builder drawable
    builder->flush(context);

    return id;
}

util::SimpleIdentity CustomDrawableLayerHost::Interface::addGeometry(
    std::shared_ptr<gfx::VertexVector<GeometryVertex>> vertices,
    std::shared_ptr<gfx::IndexVector<gfx::Triangles>> indices,
    bool is3D) {
    if (!vertices || !indices) {
        return util::SimpleIdentity::Empty;
    }

    if (!updateBuilder(BuilderType::Geometry, "custom-geometry", geometryShaderDefault())) {
        return util::SimpleIdentity::Empty;
    }

    // geographic coordinates require tile {0, 0, 0}
    setTileID({0, 0, 0});

    SegmentVector triangleSegments;
    triangleSegments.emplace_back(0, 0, vertices->elements(), indices->elements());

    // add to builder
    auto attrs = context.createVertexAttributeArray();
    if (const auto& attr = attrs->set(idCustomGeometryPosVertexAttribute)) {
        attr->setSharedRawData(vertices,
                               offsetof(GeometryVertex, position),
                               /*vertexOffset=*/0,
                               sizeof(GeometryVertex),
                               gfx::AttributeDataType::Float3);
    }

    if (const auto& attr = attrs->set(idCustomGeometryTexVertexAttribute)) {
        attr->setSharedRawData(vertices,
                               offsetof(GeometryVertex, texcoords),
                               /*vertexOffset=*/0,
                               sizeof(GeometryVertex),
                               gfx::AttributeDataType::Float2);
    }

    builder->setVertexAttributes(std::move(attrs));
    builder->setRawVertices({}, vertices->elements(), gfx::AttributeDataType::Float3);
    builder->setSegments(gfx::Triangles(), indices, triangleSegments.data(), triangleSegments.size());

    builder->setEnableDepth(true);

    if (is3D) {
        builder->setDepthType(gfx::DepthMaskType::ReadWrite);
        builder->setIs3D(true);
    }

    // white texture
    if (!geometryOptions.texture) {
        auto image = std::make_shared<PremultipliedImage>(mbgl::Size(2, 2));
        image->fill(255);

        geometryOptions.texture = context.createTexture2D();
        geometryOptions.texture->setImage(std::move(image));
    }

    builder->setTexture(geometryOptions.texture, shaders::idCustomGeometryTexture);

    builder->addTweaker(std::make_shared<GeometryDrawableTweaker>(geometryOptions, std::move(geometryTweakerCallback)));

    const auto& id = builder->getCurrentDrawable(true)->getID();

    // flush current builder drawable
    builder->flush(context);

    return id;
}

void CustomDrawableLayerHost::Interface::finish() {
    if (builder && !builder->empty()) {
        // flush current builder drawable
        builder->flush(context);

        // finish function
        const auto finish_ = [this](gfx::DrawableTweakerPtr tweaker) {
            builder->flush(context);
            for (auto& drawable : builder->clearDrawables()) {
                assert(tileID.has_value());
                drawable->setTileID(tileID.value());
                if (tweaker) {
                    drawable->addTweaker(tweaker);
                }

                TileLayerGroup* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
                tileLayerGroup->addDrawable(RenderPass::Translucent, tileID.value(), std::move(drawable));
            }
        };

        // what were we building?
        switch (builderType) {
            case BuilderType::LineClassic: {
                // finish building classic lines

                // create line tweaker
                auto tweaker = std::make_shared<LineDrawableTweaker>(lineOptions, std::move(lineTweakerCallback));

                // finish drawables
                finish_(tweaker);
            } break;
            case BuilderType::LineWideVector: {
                // finish building wide vector lines

                // create line tweaker
                auto tweaker = std::make_shared<WideVectorDrawableTweaker>(lineOptions, std::move(lineTweakerCallback));

                // finish drawables
                finish_(tweaker);
            } break;
            case BuilderType::Fill: {
                // finish building fills

                // create fill tweaker
                auto tweaker = std::make_shared<FillDrawableTweaker>(fillOptions, std::move(fillTweakerCallback));

                // finish drawables
                finish_(tweaker);
            } break;
            case BuilderType::Symbol:
                // finish building symbols
                finish_(nullptr);
                break;

            case BuilderType::Geometry:
                finish_(nullptr);
                break;
            default:
                break;
        }
    }
}

void CustomDrawableLayerHost::Interface::removeDrawable(const util::SimpleIdentity& id) {
    TileLayerGroup* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
    tileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) { return drawable.getID() == id; });
}

gfx::ShaderPtr CustomDrawableLayerHost::Interface::lineShaderDefault() const {
    gfx::ShaderGroupPtr shaderGroup = shaders.getShaderGroup("LineShader");
    assert(shaderGroup);
    if (!shaderGroup) return gfx::ShaderPtr();

    const StringIDSetsPair propertiesAsUniforms{{"a_color", "a_blur", "a_opacity", "a_gapwidth", "a_offset", "a_width"},
                                                {idLineColorVertexAttribute,
                                                 idLineBlurVertexAttribute,
                                                 idLineOpacityVertexAttribute,
                                                 idLineGapWidthVertexAttribute,
                                                 idLineOffsetVertexAttribute,
                                                 idLineWidthVertexAttribute}};

    return shaderGroup->getOrCreateShader(context, propertiesAsUniforms);
}

gfx::ShaderPtr CustomDrawableLayerHost::Interface::lineShaderWideVector() const {
    gfx::ShaderGroupPtr shaderGroup = shaders.getShaderGroup("WideVectorShader");
    if (!shaderGroup) return gfx::ShaderPtr();

    return shaderGroup->getOrCreateShader(context, {});
}

gfx::ShaderPtr CustomDrawableLayerHost::Interface::fillShaderDefault() const {
    gfx::ShaderGroupPtr shaderGroup = shaders.getShaderGroup("FillShader");

    const StringIDSetsPair propertiesAsUniforms{{"a_color", "a_opacity"},
                                                {idFillColorVertexAttribute, idFillOpacityVertexAttribute}};

    return shaderGroup->getOrCreateShader(context, propertiesAsUniforms);
}

gfx::ShaderPtr CustomDrawableLayerHost::Interface::symbolShaderDefault() const {
    return context.getGenericShader(shaders, "CustomSymbolIconShader");
}

gfx::ShaderPtr CustomDrawableLayerHost::Interface::geometryShaderDefault() const {
    return context.getGenericShader(shaders, "CustomGeometryShader");
}

std::unique_ptr<gfx::DrawableBuilder> CustomDrawableLayerHost::Interface::createBuilder(const std::string& name,
                                                                                        gfx::ShaderPtr shader) const {
    std::unique_ptr<gfx::DrawableBuilder> builder_ = context.createDrawableBuilder(name);
    builder_->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
    builder_->setSubLayerIndex(0);
    builder_->setEnableDepth(false);
    builder_->setColorMode(gfx::ColorMode::alphaBlended());
    builder_->setCullFaceMode(gfx::CullFaceMode::disabled());
    builder_->setRenderPass(RenderPass::Translucent);

    return builder_;
}

} // namespace style
} // namespace mbgl
