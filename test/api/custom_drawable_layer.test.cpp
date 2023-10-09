#include <mbgl/test/util.hpp>

#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/run_loop.hpp>

#if MLN_DRAWABLE_RENDERER

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_builder.hpp>

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/util/string_indexer.hpp>
#include <mbgl/renderer/layers/line_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/util/string_indexer.hpp>
#include <mbgl/util/convert.hpp>

#include <memory>

using namespace mbgl;
using namespace mbgl::style;

class TestDrawableTweaker : public gfx::DrawableTweaker {
public:
    TestDrawableTweaker() {}
    ~TestDrawableTweaker() override = default;

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
        const shaders::LinePropertiesUBO linePropertiesUBO{/*color =*/Color(1.f, 0.f, 1.f, 1.f),
                                                           /*blur =*/0.f,
                                                           /*opacity =*/1.f,
                                                           /*gapwidth =*/0.f,
                                                           /*offset =*/0.f,
                                                           /*width =*/8.f,
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
    };
};

class TestDrawableLayer : public mbgl::style::CustomDrawableLayerHost {
public:
    void initialize() override {}

    /**
     * @brief Create Drawables for the Custom Drawable Layer
     *
     * @param proxyLayer
     * @param shaders
     * @param context
     * @param state
     * @param updateParameters
     * @param renderTree
     * @param changes
     */
    void update(RenderLayer& proxyLayer,
                gfx::ShaderRegistry& shaders,
                gfx::Context& context,
                [[maybe_unused]] const TransformState& state,
                [[maybe_unused]] const std::shared_ptr<UpdateParameters>& updateParameters,
                [[maybe_unused]] const RenderTree& renderTree,
                UniqueChangeRequestVec& changes) override {
        // Set up a layer group
        if (!layerGroup) {
            if (auto layerGroup_ = context.createTileLayerGroup(
                    /*layerIndex*/ proxyLayer.getLayerIndex(), /*initialCapacity=*/2, proxyLayer.getID())) {
                changes.emplace_back(std::make_unique<AddLayerGroupRequest>(layerGroup_));
                layerGroup = std::move(layerGroup_);
            }
        }

        if (!layerGroup) return;

        // if we have build our drawable(s) already, either update or skip
        if (layerGroup->getDrawableCount()) return;

        // create drawable(s)
        const OverscaledTileID tileID{11, 327, 791};

        auto createLineBuilder = [&](const std::string& name,
                                     gfx::ShaderPtr shader) -> std::unique_ptr<gfx::DrawableBuilder> {
            std::unique_ptr<gfx::DrawableBuilder> builder = context.createDrawableBuilder(name);
            builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
            builder->setSubLayerIndex(0);
            builder->setEnableDepth(false);
            builder->setColorMode(gfx::ColorMode::alphaBlended());
            builder->setCullFaceMode(gfx::CullFaceMode::disabled());
            builder->setEnableStencil(false);
            builder->setRenderPass(RenderPass::Translucent);

            return builder;
        };

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
        auto builder = createLineBuilder("thick-lines", shader);

        auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

        // add polylines
        const auto size{util::EXTENT};
        GeometryCoordinates geom{{0, 0}, {size, 0}, {0, size}, {size, size}, {size / 3, size / 3}};

        gfx::PolylineGeneratorOptions options;
        options.beginCap = style::LineCapType::Round;
        options.endCap = style::LineCapType::Round;
        options.joinType = style::LineJoinType::Round;
        builder->addPolyline(geom, options);

        // create tweaker
        auto tweaker = std::make_shared<TestDrawableTweaker>();

        // finish
        builder->flush();
        for (auto& drawable : builder->clearDrawables()) {
            drawable->setType(mbgl::underlying_type(LineLayerTweaker::LineType::Simple));
            drawable->setTileID(tileID);
            drawable->addTweaker(tweaker);

            tileLayerGroup->addDrawable(RenderPass::Translucent, tileID, std::move(drawable));
        }
    }

    void deinitialize() override {
        // layerGroup->reset();
    }

private:
    std::shared_ptr<TileLayerGroup> layerGroup;
};

TEST(CustomDrawableLayer, Basic) {
    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));

    // load style
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.4426032}).withZoom(10.0));

    // add fill layer
    auto layer = std::make_unique<FillLayer>("landcover", "mapbox");
    layer->setSourceLayer("landcover");
    layer->setFillColor(Color{1.0, 1.0, 0.0, 1.0});
    map.getStyle().addLayer(std::move(layer));

    // add custom drawable layer
    map.getStyle().addLayer(
        std::make_unique<CustomDrawableLayer>("custom-drawable", std::make_unique<TestDrawableLayer>()));

    // render and test
    test::checkImage("test/fixtures/custom_drawable_layer/basic", frontend.render(map).image, 0.0006, 0.1);
}

#endif // MLN_DRAWABLE_RENDERER
