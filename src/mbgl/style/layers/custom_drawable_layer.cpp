#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/style/layers/custom_drawable_layer_impl.hpp>

#include <mbgl/renderer/layers/render_custom_drawable_layer.hpp>
#include <mbgl/style/layer_observer.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/util/string_indexer.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/geometry.hpp>

namespace mbgl {
namespace style {

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
    LineDrawableTweaker(Color color_, float width_)
        : color(color_), width(width_) {}
    ~LineDrawableTweaker() override = default;

    void init(gfx::Drawable&) override{};

    void execute(gfx::Drawable& drawable, const PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto zoom = parameters.state.getZoom();
        mat4 tileMatrix;
        parameters.state.matrixFor(/*out*/ tileMatrix, tileID);

        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, false);

        static const StringIdentity idLineUBOName = stringIndexer().get("LineUBO");
        const shaders::LineUBO lineUBO{
            /*matrix = */ util::cast<float>(matrix),
            /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
            /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, zoom),
            /*device_pixel_ratio = */ parameters.pixelRatio};

        static const StringIdentity idLinePropertiesUBOName = stringIndexer().get("LinePropertiesUBO");
        const shaders::LinePropertiesUBO linePropertiesUBO{/*color =*/color,
                                                           /*blur =*/0.f,
                                                           /*opacity =*/1.f,
                                                           /*gapwidth =*/0.f,
                                                           /*offset =*/0.f,
                                                           /*width =*/width,
                                                           0,
                                                           0,
                                                           0};

        static const StringIdentity idLineInterpolationUBOName = stringIndexer().get("LineInterpolationUBO");
        const shaders::LineInterpolationUBO lineInterpolationUBO{/*color_t =*/0.f,
                                                                 /*blur_t =*/0.f,
                                                                 /*opacity_t =*/0.f,
                                                                 /*gapwidth_t =*/0.f,
                                                                 /*offset_t =*/0.f,
                                                                 /*width_t =*/0.f,
                                                                 0,
                                                                 0};
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.createOrUpdate(idLineUBOName, &lineUBO, parameters.context);
        uniforms.createOrUpdate(idLinePropertiesUBOName, &linePropertiesUBO, parameters.context);
        uniforms.createOrUpdate(idLineInterpolationUBOName, &lineInterpolationUBO, parameters.context);

#if MLN_RENDER_BACKEND_METAL
        static const StringIdentity idExpressionInputsUBOName = stringIndexer().get("ExpressionInputsUBO");
        const auto expressionUBO = LayerTweaker::buildExpressionUBO(zoom, parameters.frameCount);
        uniforms.createOrUpdate(idExpressionInputsUBOName, &expressionUBO, parameters.context);

        static const StringIdentity idLinePermutationUBOName = stringIndexer().get("LinePermutationUBO");
        const shaders::LinePermutationUBO permutationUBO = {
            /* .color = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .blur = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .opacity = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .gapwidth = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .offset = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .width = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .floorwidth = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .pattern_from = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .pattern_to = */ {/*.source=*/shaders::AttributeSource::Constant, /*.expression=*/{}},
            /* .overdrawInspector = */ false,
            /* .pad = */ 0,
            0,
            0,
            0};
        uniforms.createOrUpdate(idLinePermutationUBOName, &permutationUBO, parameters.context);
#endif // MLN_RENDER_BACKEND_METAL
    };

private:
    Color color;
    float width;
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
    tileID = tileID_;
}

void CustomDrawableLayerHost::Interface::setColor(Color color) {
    if (currentColor.has_value() && currentColor.value() != color) {
        finish();
    }
    currentColor = color;
}

void CustomDrawableLayerHost::Interface::setWidth(float width) {
    if (currentWidth.has_value() && currentWidth.value() != width) {
        finish();
    }
    currentWidth = width;
}

void CustomDrawableLayerHost::Interface::addPolyline(const GeometryCoordinates& coordinates,
                                                     const gfx::PolylineGeneratorOptions& options) {
    if (!builder) {
        builder = createBuilder("thick-lines", lineShaderDefault());
    } else {
        // TODO: check builder
    }
    builder->addPolyline(coordinates, options);
}

void CustomDrawableLayerHost::Interface::finish() {
    if(builder->curVertexCount()) {
        // create tweaker
        assert(currentColor.has_value());
        assert(currentWidth.has_value());
        auto tweaker = std::make_shared<LineDrawableTweaker>(currentColor.value(), currentWidth.value());
        
        // finish
        builder->flush();
        for (auto& drawable : builder->clearDrawables()) {
            assert(tileID.has_value());
            drawable->setTileID(tileID.value());
            drawable->addTweaker(tweaker);
            
            TileLayerGroup* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
            tileLayerGroup->addDrawable(RenderPass::Translucent, tileID.value(), std::move(drawable));
        }
    }
}

gfx::ShaderPtr CustomDrawableLayerHost::Interface::lineShaderDefault() const {
    gfx::ShaderGroupPtr lineShaderGroup = shaders.getShaderGroup("LineShader");

    const std::unordered_set<StringIdentity> propertiesAsUniforms{
        stringIndexer().get("a_color"),
        stringIndexer().get("a_blur"),
        stringIndexer().get("a_opacity"),
        stringIndexer().get("a_gapwidth"),
        stringIndexer().get("a_offset"),
        stringIndexer().get("a_width"),
    };

    return lineShaderGroup->getOrCreateShader(context, propertiesAsUniforms);
}

std::unique_ptr<gfx::DrawableBuilder> CustomDrawableLayerHost::Interface::createBuilder(const std::string& name,
                                                                                        gfx::ShaderPtr shader) const {
    std::unique_ptr<gfx::DrawableBuilder> builder = context.createDrawableBuilder(name);
    builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
    builder->setSubLayerIndex(0);
    builder->setEnableDepth(false);
    builder->setColorMode(gfx::ColorMode::alphaBlended());
    builder->setCullFaceMode(gfx::CullFaceMode::disabled());
    builder->setEnableStencil(false);
    builder->setRenderPass(RenderPass::Translucent);

    return builder;
}

} // namespace style
} // namespace mbgl
