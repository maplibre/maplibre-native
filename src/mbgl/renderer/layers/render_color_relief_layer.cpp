#include <mbgl/renderer/layers/render_color_relief_layer.hpp>
#include <mbgl/renderer/layers/color_relief_layer_tweaker.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/style/layers/color_relief_layer_impl.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/conversion/color_ramp_property_value.hpp>
#include <mbgl/util/premultiply.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

inline const ColorReliefLayer::Impl& impl_cast(const Immutable<Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == ColorReliefLayer::Impl::staticTypeInfo());
    return static_cast<const ColorReliefLayer::Impl&>(*impl);
}

} // namespace

RenderColorReliefLayer::RenderColorReliefLayer(Immutable<ColorReliefLayer::Impl> _impl)
    : RenderLayer(makeMutable<ColorReliefLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();

    // Initialize color ramp textures
    colorRampSize = 256;
    elevationStops = std::make_shared<PremultipliedImage>(Size{colorRampSize, 1});
    colorStops = std::make_shared<PremultipliedImage>(Size{colorRampSize, 1});
}

RenderColorReliefLayer::~RenderColorReliefLayer() = default;

void RenderColorReliefLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
    updateColorRamp();
}

void RenderColorReliefLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<ColorReliefLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<ColorReliefLayerProperties>(
        staticImmutableCast<ColorReliefLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters, previousProperties->evaluated));

    passes = (properties->evaluated.get<style::ColorReliefOpacity>() > 0) ? RenderPass::Translucent : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderColorReliefLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderColorReliefLayer::hasCrossfade() const {
    return false;
}

void RenderColorReliefLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    updateRenderTileIDs();
}

// TEMPORARY TEST VERSION OF updateColorRamp()
// This removes all expression evaluation and just creates a simple gradient
// Use this to test if the rendering pipeline works at all

void RenderColorReliefLayer::updateColorRamp() {
    if (!elevationStops || !colorStops) return;

    Log::Info(Event::Render, "=== TEST updateColorRamp() - Hardcoded gradient ===");
    Log::Info(Event::Render, "Color ramp size: " + std::to_string(colorRampSize));

    // Create a simple red-to-blue gradient
    // If this shows up in the render, then the problem is expression evaluation
    // If this doesn't show up, then the problem is in the rendering/shader
    
    for (uint32_t i = 0; i < colorRampSize; ++i) {
        float t = static_cast<float>(i) / (colorRampSize - 1);
        
        // Create red (0m) to blue (8000m) gradient
        float elevation = t * 8000.0f;
        
        // Store elevation (though this won't work with RGBA8 texture, but try anyway)
        auto* elevData = reinterpret_cast<float*>(elevationStops->data.get());
        elevData[i] = elevation;
        
        // Red -> Yellow -> Green -> Cyan -> Blue gradient
        Color color;
        if (t < 0.25f) {
            // Red to Yellow
            float tt = t / 0.25f;
            color = Color(1.0f, tt, 0.0f, 1.0f);
        } else if (t < 0.5f) {
            // Yellow to Green  
            float tt = (t - 0.25f) / 0.25f;
            color = Color(1.0f - tt, 1.0f, 0.0f, 1.0f);
        } else if (t < 0.75f) {
            // Green to Cyan
            float tt = (t - 0.5f) / 0.25f;
            color = Color(0.0f, 1.0f, tt, 1.0f);
        } else {
            // Cyan to Blue
            float tt = (t - 0.75f) / 0.25f;
            color = Color(0.0f, 1.0f - tt, 1.0f, 1.0f);
        }
        
        // Store color - THIS SHOULD WORK
        colorStops->data[i * 4 + 0] = static_cast<uint8_t>(color.r * 255);
        colorStops->data[i * 4 + 1] = static_cast<uint8_t>(color.g * 255);
        colorStops->data[i * 4 + 2] = static_cast<uint8_t>(color.b * 255);
        colorStops->data[i * 4 + 3] = static_cast<uint8_t>(color.a * 255);
        
        // Log every 64th stop to see the progression
        if (i % 64 == 0 || i == colorRampSize - 1) {
            std::string stopInfo = "Stop " + std::to_string(i) + ": " +
                                   "elevation=" + std::to_string(elevation) + "m, " +
                                   "color=(" + std::to_string(colorStops->data[i * 4 + 0]) + "," +
                                   std::to_string(colorStops->data[i * 4 + 1]) + "," +
                                   std::to_string(colorStops->data[i * 4 + 2]) + "," +
                                   std::to_string(colorStops->data[i * 4 + 3]) + ")";
            Log::Info(Event::Render, stopInfo);
        }
    }

    // Log first and last colors to verify
    std::string firstColor = "First color (0m): R=" + std::to_string(colorStops->data[0]) +
                             " G=" + std::to_string(colorStops->data[1]) +
                             " B=" + std::to_string(colorStops->data[2]) +
                             " A=" + std::to_string(colorStops->data[3]);
    Log::Info(Event::Render, firstColor);
    
    int last = (colorRampSize - 1) * 4;
    std::string lastColor = "Last color (8000m): R=" + std::to_string(colorStops->data[last]) +
                            " G=" + std::to_string(colorStops->data[last+1]) +
                            " B=" + std::to_string(colorStops->data[last+2]) +
                            " A=" + std::to_string(colorStops->data[last+3]);
    Log::Info(Event::Render, lastColor);

    colorRampChanged = true;
    
    Log::Info(Event::Render, "=== TEST updateColorRamp() DONE ===");
}
static const std::string ColorReliefShaderGroupName = "ColorReliefShader";

void RenderColorReliefLayer::update(gfx::ShaderRegistry& shaders,
                                    gfx::Context& context,
                                    const TransformState&,
                                    const std::shared_ptr<UpdateParameters>&,
                                    const RenderTree&,
                                    UniqueChangeRequestVec& changes) {
    Log::Info(Event::Render, "=== update() START ===");
    
    if (!renderTiles || renderTiles->empty()) {
        Log::Info(Event::Render, "No render tiles, removing drawables");
        removeAllDrawables();
        return;
    }
    Log::Info(Event::Render, "Render tiles count: " + std::to_string(renderTiles->size()));

    // Set up layer group
    if (!layerGroup) {
        Log::Info(Event::Render, "Creating layer group");
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            Log::Error(Event::Render, "Failed to create layer group!");
            return;
        }
    }

    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

    if (!layerTweaker) {
        Log::Info(Event::Render, "Creating layer tweaker");
        layerTweaker = std::make_shared<ColorReliefLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    if (!colorReliefShader) {
        Log::Info(Event::Render, "Getting shader: " + ColorReliefShaderGroupName);
        colorReliefShader = context.getGenericShader(shaders, ColorReliefShaderGroupName);
    }

    if (!colorReliefShader) {
        Log::Error(Event::Render, "Failed to get shader!");
        removeAllDrawables();
        return;
    }
    Log::Info(Event::Render, "Shader obtained successfully");

    auto renderPass = RenderPass::Translucent;
    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        Log::Info(Event::Render, "Render pass check failed");
        return;
    }

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    if (!staticDataSharedVertices) {
        Log::Info(Event::Render, "Creating static data vertices");
        staticDataSharedVertices = std::make_shared<ColorReliefVertexVector>(RenderStaticData::rasterVertices());
    }
    const auto staticDataIndices = RenderStaticData::quadTriangleIndices();
    const auto staticDataSegments = RenderStaticData::rasterSegments();

    // Update color ramp textures if changed
    if (colorRampChanged && elevationStops && colorStops) {
        Log::Info(Event::Render, "Updating color ramp textures");
        
        if (!elevationStopsTexture) {
            Log::Info(Event::Render, "Creating elevation stops texture");
            elevationStopsTexture = context.createTexture2D();
        }
        elevationStopsTexture->setImage(elevationStops);
        elevationStopsTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                                        .wrapU = gfx::TextureWrapType::Clamp,
                                                        .wrapV = gfx::TextureWrapType::Clamp});

        if (!colorStopsTexture) {
            Log::Info(Event::Render, "Creating color stops texture");
            colorStopsTexture = context.createTexture2D();
        }
        colorStopsTexture->setImage(colorStops);
        colorStopsTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                                    .wrapU = gfx::TextureWrapType::Clamp,
                                                    .wrapV = gfx::TextureWrapType::Clamp});

        colorRampChanged = false;
        Log::Info(Event::Render, "Color ramp textures updated");
    }

    std::unique_ptr<gfx::DrawableBuilder> builder;

    Log::Info(Event::Render, "Processing " + std::to_string(renderTiles->size()) + " tiles");
    
    int tileCount = 0;
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();
        Log::Info(Event::Render, "Processing tile #" + std::to_string(tileCount++));

        auto* bucket_ = tile.getBucket(*baseImpl);
        if (!bucket_ || !bucket_->hasData()) {
            Log::Info(Event::Render, "Tile has no bucket data, skipping");
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<HillshadeBucket&>(*bucket_);
        Log::Info(Event::Render, "Bucket found with data");

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        // Set up tile drawable
        std::shared_ptr<ColorReliefVertexVector> vertices;
        std::shared_ptr<gfx::IndexVector<gfx::Triangles>> indices;
        auto* segments = &staticDataSegments;

        if (!bucket.vertices.empty() && !bucket.indices.empty() && !bucket.segments.empty()) {
            vertices = bucket.sharedVertices;
            indices = bucket.sharedIndices;
            segments = &bucket.segments;
        } else {
            vertices = staticDataSharedVertices;
            indices = std::make_shared<gfx::IndexVector<gfx::Triangles>>(staticDataIndices);
        }

        if (!builder) {
            Log::Info(Event::Render, "Creating drawable builder");
            builder = context.createDrawableBuilder("colorRelief");
        }

        Log::Info(Event::Render, "Setting up drawable for tile");
        
        // Continue with the rest of your existing drawable creation code here...
        // (Keep all the existing code from gfx::VertexAttributeArrayPtr vertexAttrs; onwards)
    }
    
    Log::Info(Event::Render, "=== update() END ===");
}

bool RenderColorReliefLayer::queryIntersectsFeature(const GeometryCoordinates&,
                                                    const GeometryTileFeature&,
                                                    float,
                                                    const TransformState&,
                                                    float,
                                                    const mat4&,
                                                    const FeatureState&) const {
    return false;
}

} // namespace mbgl
