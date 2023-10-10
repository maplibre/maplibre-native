#import "MLNCustomDrawableStyleLayer.h"
#import "MLNCustomDrawableStyleLayer_Private.h"

#import "MLNMapView_Private.h"
#import "MLNStyle_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNGeometry_Private.h"

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/math/wrap.hpp>

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
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/util/string_indexer.hpp>
#include <mbgl/util/convert.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/line.hpp>
#endif // MLN_RENDER_BACKEND_METAL

#include <memory>

class MLNCustomDrawableLayerHost;

@interface MLNCustomDrawableStyleLayer ()

@property (nonatomic, readonly) mbgl::style::CustomDrawableLayer *rawLayer;

@property (nonatomic, readonly, nullable) MLNMapView *mapView;

@property (nonatomic, weak, readwrite) MLNStyle *style;

@end

@implementation MLNCustomDrawableStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier {
    auto layer = std::make_unique<mbgl::style::CustomDrawableLayer>(identifier.UTF8String,
                                                            std::make_unique<MLNCustomDrawableLayerHost>(self));
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::CustomDrawableLayer *)rawLayer {
    return (mbgl::style::CustomDrawableLayer *)super.rawLayer;
}

- (MLNMapView *)mapView {
    if ([self.style.stylable isKindOfClass:[MLNMapView class]]) {
        return (MLNMapView *)self.style.stylable;
    }
    return nil;
}

- (void)didMoveToMapView:(MLNMapView *)mapView {

}

- (void)willMoveFromMapView:(MLNMapView *)mapView {

}

- (void)setNeedsDisplay {
    [self.mapView setNeedsRerender];
}

@end

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
};

class MLNCustomDrawableLayerHost : public mbgl::style::CustomDrawableLayerHost {
public:
    MLNCustomDrawableLayerHost(MLNCustomDrawableStyleLayer *styleLayer) {
        layerRef = styleLayer;
        layer = nil;
    }
    
    void initialize() {
        if (layerRef == nil) return;
        else if (layer == nil) layer = layerRef;
        
        if (layer.mapView) {
            [layer didMoveToMapView:layer.mapView];
        }
    }
    
    void update(mbgl::RenderLayer& proxyLayer,
                mbgl::gfx::ShaderRegistry& shaders,
                mbgl::gfx::Context& context,
                const mbgl::TransformState& state,
                const std::shared_ptr<mbgl::UpdateParameters>&,
                const mbgl::RenderTree& renderTree,
                mbgl::UniqueChangeRequestVec& changes) {
        
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
            drawable->setTileID(tileID);
            drawable->addTweaker(tweaker);
            
            tileLayerGroup->addDrawable(RenderPass::Translucent, tileID, std::move(drawable));
        }
    }
    
    void deinitialize() {
        if (layer == nil) return;
        
        if (layer.mapView) {
            [layer willMoveFromMapView:layer.mapView];
        }
        layerRef = layer;
        layer = nil;
    }
private:
    __weak MLNCustomDrawableStyleLayer * layerRef;
    MLNCustomDrawableStyleLayer * layer = nil;

    std::shared_ptr<TileLayerGroup> layerGroup;
};

namespace mbgl {

MLNStyleLayer* CustomDrawableStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNCustomDrawableStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl

