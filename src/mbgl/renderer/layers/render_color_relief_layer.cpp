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

    passes = (properties->evaluated.get<style::ColorReliefOpacity>() > 0) 
        ? RenderPass::Translucent
        : RenderPass::None;
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

void RenderColorReliefLayer::updateColorRamp() {
    if (!elevationStops || !colorStops) return;
    
    auto colorValue = unevaluated.get<ColorReliefColor>().getValue();
    if (colorValue.isUndefined()) {
        colorValue = ColorReliefLayer::getDefaultColorReliefColor();
    }

    // TODO: Parse the expression to extract elevation/color pairs
    // For now, create a simple gradient from blue (low) to red (high)
    // This is a placeholder - you'll need to properly parse the expression
    
    for (uint32_t i = 0; i < colorRampSize; ++i) {
        float t = static_cast<float>(i) / (colorRampSize - 1);
        
        // Simple RGB encoding for elevation (Mapbox Terrain RGB format)
        // elevation = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1)
        // For a range of -10000m to +8849m (approx -10000 to +18849), we need values 0 to 288490
        float elevation = -10000.0f + t * 18849.0f;
        
        // Encode elevation in RGB format
        int elevInt = static_cast<int>((elevation + 10000.0f) * 10.0f);
        uint8_t r = (elevInt >> 16) & 0xFF;
        uint8_t g = (elevInt >> 8) & 0xFF;
        uint8_t b = elevInt & 0xFF;
        uint8_t a = 0; // Not used in unpacking
        
        elevationStops->data[i * 4 + 0] = r;
        elevationStops->data[i * 4 + 1] = g;
        elevationStops->data[i * 4 + 2] = b;
        elevationStops->data[i * 4 + 3] = a;
        
        // Simple color gradient: blue -> cyan -> green -> yellow -> red
        Color color;
        if (t < 0.25f) {
            float localT = t / 0.25f;
            color = Color{0.0f, localT, 1.0f, 1.0f};
        } else if (t < 0.5f) {
            float localT = (t - 0.25f) / 0.25f;
            color = Color{0.0f, 1.0f, 1.0f - localT, 1.0f};
        } else if (t < 0.75f) {
            float localT = (t - 0.5f) / 0.25f;
            color = Color{localT, 1.0f, 0.0f, 1.0f};
        } else {
            float localT = (t - 0.75f) / 0.25f;
            color = Color{1.0f, 1.0f - localT, 0.0f, 1.0f};
        }
        
        colorStops->data[i * 4 + 0] = static_cast<uint8_t>(color.r * 255);
        colorStops->data[i * 4 + 1] = static_cast<uint8_t>(color.g * 255);
        colorStops->data[i * 4 + 2] = static_cast<uint8_t>(color.b * 255);
        colorStops->data[i * 4 + 3] = static_cast<uint8_t>(color.a * 255);
    }
    
    colorRampChanged = true;
}

static const std::string ColorReliefShaderGroupName = "ColorReliefShader";

void RenderColorReliefLayer::update(gfx::ShaderRegistry& shaders,
                                    gfx::Context& context,
                                    const TransformState&,
                                    const std::shared_ptr<UpdateParameters>&,
                                    const RenderTree&,
                                    UniqueChangeRequestVec& changes) {
    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }

    // Set up layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            return;
        }
    }

    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

    if (!layerTweaker) {
        layerTweaker = std::make_shared<ColorReliefLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    if (!colorReliefShader) {
        colorReliefShader = context.getGenericShader(shaders, ColorReliefShaderGroupName);
    }

    if (!colorReliefShader) {
        removeAllDrawables();
        return;
    }

    auto renderPass = RenderPass::Translucent;
    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    if (!staticDataSharedVertices) {
        staticDataSharedVertices = std::make_shared<ColorReliefVertexVector>(RenderStaticData::rasterVertices());
    }
    const auto staticDataIndices = RenderStaticData::quadTriangleIndices();
    const auto staticDataSegments = RenderStaticData::rasterSegments();

    // Update color ramp textures if changed
    if (colorRampChanged && elevationStops && colorStops) {
        if (!elevationStopsTexture) {
            elevationStopsTexture = context.createTexture2D();
        }
        elevationStopsTexture->setImage(elevationStops);
        elevationStopsTexture->setSamplerConfiguration({
            .filter = gfx::TextureFilterType::Linear,
            .wrapU = gfx::TextureWrapType::Clamp,
            .wrapV = gfx::TextureWrapType::Clamp
        });

        if (!colorStopsTexture) {
            colorStopsTexture = context.createTexture2D();
        }
        colorStopsTexture->setImage(colorStops);
        colorStopsTexture->setSamplerConfiguration({
            .filter = gfx::TextureFilterType::Linear,
            .wrapU = gfx::TextureWrapType::Clamp,
            .wrapV = gfx::TextureWrapType::Clamp
        });

        colorRampChanged = false;
    }

    std::unique_ptr<gfx::DrawableBuilder> builder;

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        auto* bucket_ = tile.getBucket(*baseImpl);
        if (!bucket_ || !bucket_->hasData()) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<HillshadeBucket&>(*bucket_);

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
            builder = context.createDrawableBuilder("colorRelief");
        }

        gfx::VertexAttributeArrayPtr vertexAttrs;
        auto buildVertexAttributes = [&] {
            if (!vertexAttrs) {
                vertexAttrs = context.createVertexAttributeArray();

                if (const auto& attr = vertexAttrs->set(idColorReliefPosVertexAttribute)) {
                    attr->setSharedRawData(vertices,
                                         offsetof(HillshadeLayoutVertex, a1),
                                         0,
                                         sizeof(HillshadeLayoutVertex),
                                         gfx::AttributeDataType::Short2);
                }
            }
            return vertexAttrs;
        };

        const auto updateExisting = [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                return false;
            }

            drawable.updateVertexAttributes(buildVertexAttributes(),
                                          vertices->elements(),
                                          gfx::Triangles(),
                                          std::move(indices),
                                          segments->data(),
                                          segments->size());

            // Update textures
            std::shared_ptr<gfx::Texture2D> demTexture = context.createTexture2D();
            demTexture->setImage(bucket.getDEMData().getImagePtr());
            demTexture->setSamplerConfiguration({
                .filter = gfx::TextureFilterType::Linear,
                .wrapU = gfx::TextureWrapType::Clamp,
                .wrapV = gfx::TextureWrapType::Clamp
            });
            drawable.setTexture(demTexture, idColorReliefImageTexture);
            
            if (elevationStopsTexture) {
                drawable.setTexture(elevationStopsTexture, idColorReliefElevationStopsTexture);
            }
            if (colorStopsTexture) {
                drawable.setTexture(colorStopsTexture, idColorReliefColorStopsTexture);
            }

            return true;
        };
        
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

        builder->setShader(colorReliefShader);
        builder->setDepthType(gfx::DepthMaskType::ReadOnly);
        builder->setColorMode(gfx::ColorMode::alphaBlended());
        builder->setCullFaceMode(gfx::CullFaceMode::disabled());
        builder->setRenderPass(renderPass);
        builder->setVertexAttributes(buildVertexAttributes());
        builder->setRawVertices({}, vertices->elements(), gfx::AttributeDataType::Short2);
        builder->setSegments(gfx::Triangles(), indices->vector(), segments->data(), segments->size());

        // Bind DEM texture
        std::shared_ptr<gfx::Texture2D> demTexture = context.createTexture2D();
        demTexture->setImage(bucket.getDEMData().getImagePtr());
        demTexture->setSamplerConfiguration({
            .filter = gfx::TextureFilterType::Linear,
            .wrapU = gfx::TextureWrapType::Clamp,
            .wrapV = gfx::TextureWrapType::Clamp
        });
        builder->setTexture(demTexture, idColorReliefImageTexture);

        // Bind color ramp textures
        if (elevationStopsTexture) {
            builder->setTexture(elevationStopsTexture, idColorReliefElevationStopsTexture);
        }
        if (colorStopsTexture) {
            builder->setTexture(colorStopsTexture, idColorReliefColorStopsTexture);
        }

        builder->flush(context);

        for (auto& drawable : builder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);

            // Set up tile properties UBO
            shaders::ColorReliefTilePropsUBO tilePropsUBO;
            
            // DEM unpack vector (Mapbox Terrain RGB format)
            tilePropsUBO.unpack = {{6553.6f, 25.6f, 0.1f, 10000.0f}};
            
            // Texture dimensions
            const auto& demData = bucket.getDEMData();
            tilePropsUBO.dimension = {{static_cast<float>(demData.dim), 
                                       static_cast<float>(demData.dim)}};
            
            // Color ramp size
            tilePropsUBO.color_ramp_size = static_cast<int32_t>(colorRampSize);
            tilePropsUBO.pad0 = 0.0f;
            
            auto& drawableUniforms = drawable->mutableUniformBuffers();
            drawableUniforms.createOrUpdate(idColorReliefTilePropsUBO, &tilePropsUBO, context);

            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }
}

bool RenderColorReliefLayer::queryIntersectsFeature(
        const GeometryCoordinates&,
        const GeometryTileFeature&,
        float,
        const TransformState&,
        float,
        const mat4&,
        const FeatureState&) const {
    return false;
}

} // namespace mbgl
