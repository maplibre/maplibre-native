#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/style/layers/custom_drawable_layer_impl.hpp>

#include <mbgl/renderer/layers/render_custom_drawable_layer.hpp>
#include <mbgl/style/layer_observer.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

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

    auto shader = lineShaderGroup->getOrCreateShader(context, propertiesAsUniforms);
    return shader;
}

bool CustomDrawableLayerHost::Interface::getTileLayerGroup(std::shared_ptr<TileLayerGroup>& layerGroupRef,
                                                           mbgl::RenderLayer& proxyLayer) const {
    if (!layerGroupRef) {
        if (auto layerGroup_ = context.createTileLayerGroup(
                /*layerIndex*/ proxyLayer.getLayerIndex(), /*initialCapacity=*/64, proxyLayer.getID())) {
            changes.emplace_back(std::make_unique<AddLayerGroupRequest>(layerGroup_));
            layerGroupRef = std::move(layerGroup_);
        }
    }

    return layerGroupRef.get() != nullptr;
}

std::unique_ptr<CustomDrawableLayerHost::LineBuilderHelper>
CustomDrawableLayerHost::Interface::createLineBuilderHelper() const {
    return std::make_unique<CustomDrawableLayerHost::LineBuilderHelper>();
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
