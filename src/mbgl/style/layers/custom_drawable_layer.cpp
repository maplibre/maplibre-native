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
#include <mbgl/programs/fill_program.hpp>
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
#include <mbgl/util/projection.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/renderer/render_tile.hpp>

#include <cmath>

namespace mbgl {

namespace style {

using namespace shaders;

namespace {
const LayerTypeInfo typeInfoCustomDrawable{"custom-drawable",
                                           LayerTypeInfo::Source::NotRequired,
                                           LayerTypeInfo::Pass3D::NotRequired,
                                           LayerTypeInfo::Layout::NotRequired,
                                           LayerTypeInfo::FadingTiles::NotRequired,
                                           LayerTypeInfo::CrossTileIndex::NotRequired,
                                           LayerTypeInfo::TileKind::NotRequired};
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
    LineDrawableTweaker(const shaders::LineEvaluatedPropsUBO& properties)
        : linePropertiesUBO(properties) {}
    ~LineDrawableTweaker() override = default;

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, const PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto zoom = parameters.state.getZoom();
        mat4 tileMatrix;
        parameters.state.matrixFor(/*out*/ tileMatrix, tileID);

        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, drawable, false);

        const shaders::LineDrawableUBO drawableUBO = {/*matrix = */ util::cast<float>(matrix),
                                                      /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, zoom),
                                                      0,
                                                      0,
                                                      0};
        const shaders::LineInterpolationUBO lineInterpolationUBO{/*color_t =*/0.f,
                                                                 /*blur_t =*/0.f,
                                                                 /*opacity_t =*/0.f,
                                                                 /*gapwidth_t =*/0.f,
                                                                 /*offset_t =*/0.f,
                                                                 /*width_t =*/0.f,
                                                                 0,
                                                                 0};
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idLineDrawableUBO, &drawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idLineInterpolationUBO, &lineInterpolationUBO, parameters.context);
        drawableUniforms.createOrUpdate(idLineEvaluatedPropsUBO, &linePropertiesUBO, parameters.context);

        // We would need to set up `idLineExpressionUBO` if the expression mask isn't empty
        assert(linePropertiesUBO.expressionMask == LineExpressionMask::None);
    };

private:
    shaders::LineEvaluatedPropsUBO linePropertiesUBO;
};

class WideVectorDrawableTweaker : public gfx::DrawableTweaker {
public:
    WideVectorDrawableTweaker(const CustomDrawableLayerHost::Interface::LineOptions& options_)
        : options(options_) {}

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, const PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
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
        shaders::WideVectorUniformsUBO uniform{
            /*mvpMatrix     */ mvpMatrix,
            /*mvpMatrixDiff */ mvpMatrixDiff,
            /*mvMatrix      */ mvMatrix,
            /*mvMatrixDiff  */ mvMatrixDiff,
            /*pMatrix       */ pMatrix,
            /*pMatrixDiff   */ pMatrixDiff,
            /*frameSize     */ {(float)renderableSize.width, (float)renderableSize.height}};

        shaders::WideVectorUniformWideVecUBO wideVec{
            /*color         */ options.color,
            /*w2            */ options.width,
            /*offset        */ options.offset,
            /*edge          */ 0.0f, // TODO: MLN does not provide a value. Analyze impact.
            /*texRepeat     */ 0.0f, // N/A
            /*texOffset     */ {},   // N/A
            /*miterLimit    */ options.geometry.miterLimit,
            /*join          */ static_cast<int32_t>(options.geometry.joinType),
            /*cap           */ static_cast<int32_t>(options.geometry.beginCap), // TODO: MLN option for endCap to be
                                                                                // implemented in the shader!
            /*hasExp        */ false,                                           // N/A
            /*interClipLimit*/ 0.0f                                             // N/A
        };

        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idWideVectorUniformsUBO, &uniform, parameters.context);
        drawableUniforms.createOrUpdate(idWideVectorUniformWideVecUBO, &wideVec, parameters.context);
    };

private:
    CustomDrawableLayerHost::Interface::LineOptions options;
};

class FillDrawableTweaker : public gfx::DrawableTweaker {
public:
    FillDrawableTweaker(const Color& color_, float opacity_)
        : color(color_),
          opacity(opacity_) {}
    ~FillDrawableTweaker() override = default;

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, const PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        mat4 tileMatrix;
        parameters.state.matrixFor(/*out*/ tileMatrix, tileID);

        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, drawable, false);

        const shaders::FillDrawableUBO fillDrawableUBO{/*matrix = */ util::cast<float>(matrix)};

        const shaders::FillInterpolateUBO fillInterpolateUBO{
            /* .color_t = */ 0.f,
            /* .opacity_t = */ 0.f,
            0,
            0,
        };
        const shaders::FillEvaluatedPropsUBO fillPropertiesUBO{
            /* .color = */ color,
            /* .outline_color = */ Color::white(),
            /* .opacity = */ opacity,
            /* .fade = */ 0.f,
            /* .from_scale = */ 0.f,
            /* .to_scale = */ 0.f,
        };
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idFillDrawableUBO, &fillDrawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idFillInterpolateUBO, &fillInterpolateUBO, parameters.context);
        drawableUniforms.createOrUpdate(idFillEvaluatedPropsUBO, &fillPropertiesUBO, parameters.context);
    };

private:
    Color color;
    float opacity;
};

class SymbolDrawableTweaker : public gfx::DrawableTweaker {
public:
    SymbolDrawableTweaker(const CustomDrawableLayerHost::Interface::SymbolOptions& options_)
        : options(options_) {}
    ~SymbolDrawableTweaker() override = default;

    void init(gfx::Drawable&) override {}

    void execute(gfx::Drawable& drawable, const PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        mat4 tileMatrix;
        parameters.state.matrixFor(/*out*/ tileMatrix, tileID);

        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, drawable, false);

        const shaders::CustomSymbolIconDrawableUBO drawableUBO{/*matrix = */ util::cast<float>(matrix)};

        const auto pixelsToTileUnits = tileID.pixelsToTileUnits(
            1.0f, options.scaleWithMap ? tileID.canonical.z : parameters.state.getZoom());
        const float factor = options.scaleWithMap
                                 ? static_cast<float>(std::pow(2.f, parameters.state.getZoom() - tileID.canonical.z))
                                 : 1.0f;
        const auto extrudeScale = options.pitchWithMap ? std::array<float, 2>{pixelsToTileUnits, pixelsToTileUnits}
                                                       : std::array<float, 2>{parameters.pixelsToGLUnits[0] * factor,
                                                                              parameters.pixelsToGLUnits[1] * factor};

        const shaders::CustomSymbolIconParametersUBO parametersUBO{
            /*extrude_scale*/ {extrudeScale[0] * options.size.width, extrudeScale[1] * options.size.height},
            /*anchor*/ options.anchor,
            /*angle_degrees*/ options.angleDegrees,
            /*scale_with_map*/ options.scaleWithMap,
            /*pitch_with_map*/ options.pitchWithMap,
            /*camera_to_center_distance*/ parameters.state.getCameraToCenterDistance(),
            /*aspect_ratio*/ parameters.pixelsToGLUnits[0] / parameters.pixelsToGLUnits[1],
            0,
            0,
            0};

        // set UBOs
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idCustomSymbolDrawableUBO, &drawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idCustomSymbolParametersUBO, &parametersUBO, parameters.context);
    };

private:
    CustomDrawableLayerHost::Interface::SymbolOptions options;
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

bool CustomDrawableLayerHost::Interface::addPolyline(const LineString<double>& coordinates) {
    switch (lineOptions.shaderType) {
        case LineShaderType::Classic: {
            // TODO: build classic polyline with Geo coordinates
            return false;
        } break;

        case LineShaderType::MetalWideVector: {
            // build wide vector polyline with Geo coordinates
            if (!updateBuilder(BuilderType::LineWideVector, "custom-lines-widevector", lineShaderWideVector()))
                return false;

            // geographic coordinates require tile {0, 0, 0}
            setTileID({0, 0, 0});

            builder->addWideVectorPolylineGlobal(coordinates, lineOptions.geometry);
        } break;
    }

    return true;
}

bool CustomDrawableLayerHost::Interface::addPolyline(const GeometryCoordinates& coordinates) {
    switch (lineOptions.shaderType) {
        case LineShaderType::Classic: {
            // build classic polyline with Tile coordinates
            if (!updateBuilder(BuilderType::LineClassic, "custom-lines", lineShaderDefault())) return false;
            builder->addPolyline(coordinates, lineOptions.geometry);
        } break;

        case LineShaderType::MetalWideVector: {
            // build wide vector polyline
            if (!updateBuilder(BuilderType::LineWideVector, "custom-lines-widevector", lineShaderWideVector()))
                return false;

            builder->addWideVectorPolylineLocal(coordinates, lineOptions.geometry);
        } break;
    }

    return true;
}

bool CustomDrawableLayerHost::Interface::addFill(const GeometryCollection& geometry) {
    // build fill
    if (!updateBuilder(BuilderType::Fill, "custom-fill", fillShaderDefault())) return false;

    // provision buffers for fill vertices, indexes and segments
    using VertexVector = gfx::VertexVector<FillLayoutVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    SegmentVector<FillAttributes> triangleSegments;

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

    // flush current builder drawable
    builder->flush(context);

    return true;
}

bool CustomDrawableLayerHost::Interface::addSymbol(const GeometryCoordinate& point) {
    // build symbol
    if (!updateBuilder(BuilderType::Symbol, "custom-symbol", symbolShaderDefault())) return false;

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
                CustomSymbolIcon{{static_cast<float>(point.x * 2 + x), static_cast<float>(point.y * 2 + y)},
                                 {symbolOptions.textureCoordinates[x][0], symbolOptions.textureCoordinates[y][1]}});
        }
    }

    // indexes
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& triangles = *sharedTriangles;

    triangles.emplace_back(0, 1, 2, 1, 2, 3);

    SegmentVector<CustomSymbolIcon> triangleSegments;
    triangleSegments.emplace_back(Segment<CustomSymbolIcon>{0, 0, 4, 6});

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

    // create fill tweaker
    auto tweaker = std::make_shared<SymbolDrawableTweaker>(symbolOptions);
    builder->addTweaker(tweaker);

    // flush current builder drawable
    builder->flush(context);

    return true;
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
                const shaders::LineEvaluatedPropsUBO linePropertiesUBO = {lineOptions.color,
                                                                          lineOptions.blur,
                                                                          lineOptions.opacity,
                                                                          lineOptions.gapWidth,
                                                                          lineOptions.offset,
                                                                          lineOptions.width,
                                                                          /*floorwidth=*/0,
                                                                          LineExpressionMask::None,
                                                                          0};
                auto tweaker = std::make_shared<LineDrawableTweaker>(linePropertiesUBO);

                // finish drawables
                finish_(tweaker);
            } break;
            case BuilderType::LineWideVector: {
                // finish building wide vector lines

                // create line tweaker
                auto tweaker = std::make_shared<WideVectorDrawableTweaker>(lineOptions);

                // finish drawables
                finish_(tweaker);
            } break;
            case BuilderType::Fill: {
                // finish building fills

                // create fill tweaker
                auto tweaker = std::make_shared<FillDrawableTweaker>(fillOptions.color, fillOptions.opacity);

                // finish drawables
                finish_(tweaker);
            } break;
            case BuilderType::Symbol:
                // finish building symbols
                finish_(nullptr);
                break;
            default:
                break;
        }
    }
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
