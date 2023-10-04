#include <mbgl/test/util.hpp>

#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/run_loop.hpp>

#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/util/string_indexer.hpp>
#include <mbgl/renderer/layers/line_layer_tweaker.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>

using namespace mbgl;
using namespace mbgl::style;

class TestLayer : public mbgl::style::CustomDrawableLayerHost {
public:
    void initialize() override {
    }

    void upload() override {        
        if(layerGroup) {
            layerGroup->render(orchestrator, parameters);
        }
    }

    void draw(const PaintParameters&) override {
        if(layerGroup) {
            layerGroup->render(orchestrator, parameters);
        }
    }

    void update() override {
        if(layerGroup && layerGroup->getDrawableCount()) {
            return;
        }

        // create drawables
        const OverscaledTileID tile{6, 19, 23};
        const float zoom = tile.overscaledZ;
        const UnwrappedTileID tileID = tile.toUnwrapped();
        const auto matrix = LayerTweaker::getTileMatrix(
            tileID, renderTree, parameters.state, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false);

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

        static const StringIdentity idLineUBOName = StringIndexer::get("LineUBO");
        const shaders::LineUBO lineUBO{
            /*matrix = */ util::cast<float>(matrix),
            /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
            /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, zoom),
            /*device_pixel_ratio = */ parameters.pixelRatio};

        static const StringIdentity idLinePropertiesUBOName = StringIndexer::get("LinePropertiesUBO");
        const shaders::LinePropertiesUBO linePropertiesUBO{/*color =*/Color::red(),
                                                           /*blur =*/0.f,
                                                           /*opacity =*/1.f,
                                                           /*gapwidth =*/0.f,
                                                           /*offset =*/0.f,
                                                           /*width =*/8.f,
                                                           0,
                                                           0,
                                                           0};

        static const StringIdentity idLineInterpolationUBOName = StringIndexer::get("LineInterpolationUBO");
        const shaders::LineInterpolationUBO lineInterpolationUBO{/*color_t =*/0.f,
                                                                 /*blur_t =*/0.f,
                                                                 /*opacity_t =*/0.f,
                                                                 /*gapwidth_t =*/0.f,
                                                                 /*offset_t =*/0.f,
                                                                 /*width_t =*/0.f,
                                                                 0,
                                                                 0};

        gfx::ShaderGroupPtr lineShaderGroup = staticData->shaders->getShaderGroup("LineShader");
        ;
        const std::unordered_set<StringIdentity> propertiesAsUniforms{
            StringIndexer::get("a_color"),
            StringIndexer::get("a_blur"),
            StringIndexer::get("a_opacity"),
            StringIndexer::get("a_gapwidth"),
            StringIndexer::get("a_offset"),
            StringIndexer::get("a_width"),
        };

        auto shader = lineShaderGroup->getOrCreateShader(context, propertiesAsUniforms);
        auto builder = createLineBuilder("thick-lines", shader);

        auto layerGroup_ = context.createTileLayerGroup(0, /*initialCapacity=*/1, "tlg");
        auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup_.get());

        // add polylines
        const auto size{util::EXTENT};
        GeometryCoordinates geom{{0, 0}, {size, 0}, {size / 2, size / 2}};

        gfx::DrawableBuilder::PolylineOptions options;
        options.beginCap = style::LineCapType::Round;
        options.endCap = style::LineCapType::Round;
        options.joinType = style::LineJoinType::Round;
        builder->addPolyline(geom, options);

        // finish
        builder->flush();
        for (auto& drawable : builder->clearDrawables()) {
            drawable->setType(mbgl::underlying_type(LineLayerTweaker::LineType::Simple));
            drawable->setTileID(tile);
            auto& uniforms = drawable->mutableUniformBuffers();
            uniforms.createOrUpdate(idLineUBOName, &lineUBO, context);
            uniforms.createOrUpdate(idLinePropertiesUBOName, &linePropertiesUBO, context);
            uniforms.createOrUpdate(idLineInterpolationUBOName, &lineInterpolationUBO, context);

            tileLayerGroup->addDrawable(RenderPass::Translucent, tile, std::move(drawable));
        }        
    }

    void deinitialize() override {
        layerGroup->reset();
    }

private:
    gfx::TileLayerGroupPtr layerGroup;
};

TEST(CustomDrawableLayer, Basic) {
    if (gfx::Backend::GetType() != gfx::Backend::Type::OpenGL) {
        return;
    }

    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.5}).withZoom(10.0));
    map.getStyle().addLayer(std::make_unique<CustomDrawableLayer>("custom-drawable", std::make_unique<TestLayer>()));

    auto layer = std::make_unique<FillLayer>("landcover", "mapbox");
    layer->setSourceLayer("landcover");
    layer->setFillColor(Color{1.0, 1.0, 0.0, 1.0});
    map.getStyle().addLayer(std::move(layer));

    test::checkImage("test/fixtures/custom_drawable_layer/basic", frontend.render(map).image, 0.0006, 0.1);
}
