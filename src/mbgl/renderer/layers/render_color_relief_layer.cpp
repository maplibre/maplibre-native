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
#include <mbgl/style/expression/interpolate.hpp>
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

void RenderColorReliefLayer::updateColorRamp() {
    if (!elevationStops || !colorStops) {
        Log::Warning(Event::Render, "elevationStops or colorStops is null!");
        return;
    }

    Log::Info(Event::Render, "updateColorRamp: Getting color property");
    
    // Get the color property value
    auto colorValue = unevaluated.get<ColorReliefColor>().getValue();
    if (colorValue.isUndefined()) {
        Log::Warning(Event::Render, "colorValue is undefined, using default");
        colorValue = ColorReliefLayer::getDefaultColorReliefColor();
    }

    // Get the expression from the color ramp property
    const auto* expr = &colorValue.getExpression();
    if (!expr) {
        Log::Error(Event::Render, "Expression is null!");
        return;
    }

    // Log the expression type to see what we're actually getting
    Log::Info(Event::Render, "Expression kind: " + std::to_string(static_cast<int>(expr->getKind())));
    
    // The expression might be wrapped - recursively search for the Interpolate expression
    const expression::Interpolate* interpolate = nullptr;
    
    std::function<void(const expression::Expression&)> findInterpolate;
    findInterpolate = [&](const expression::Expression& e) {
        if (interpolate) return; // Already found
        
        if (e.getKind() == expression::Kind::Interpolate) {
            interpolate = static_cast<const expression::Interpolate*>(&e);
            Log::Info(Event::Render, "Found Interpolate expression");
        } else {
            // Recursively search children
            e.eachChild([&](const expression::Expression& child) {
                findInterpolate(child);
            });
        }
    };
    
    findInterpolate(*expr);
    
    if (!interpolate) {
        Log::Error(Event::Render, "Could not find Interpolate expression in tree!");
        // Fall back to sampling approach
        const float minElevation = -11000.0f;
        const float maxElevation = 9000.0f;

        for (uint32_t i = 0; i < colorRampSize; ++i) {
            float t = static_cast<float>(i) / (colorRampSize - 1);
            float elevation = minElevation + t * (maxElevation - minElevation);

            expression::EvaluationContext context(0.0f);
            context.elevation = elevation;

            Color color = Color::black();
            auto result = expr->evaluate(context);
            if (result) {
                color = *expression::fromExpressionValue<Color>(*result);
            }

            auto* elevData = reinterpret_cast<float*>(elevationStops->data.get());
            elevData[i] = elevation;

            colorStops->data[i * 4 + 0] = static_cast<uint8_t>(color.r * 255);
            colorStops->data[i * 4 + 1] = static_cast<uint8_t>(color.g * 255);
            colorStops->data[i * 4 + 2] = static_cast<uint8_t>(color.b * 255);
            colorStops->data[i * 4 + 3] = static_cast<uint8_t>(color.a * 255);
        }
        colorRampChanged = true;
        return;
    }

    Log::Info(Event::Render, "Using Interpolate expression with " + std::to_string(interpolate->getStopCount()) + " stops");
    
    std::vector<double> elevationValues;
    std::vector<Color> colors;
    
    // Extract stops directly from the interpolate expression (like GL JS does)
    interpolate->eachStop([&](double elevation, const expression::Expression& colorExpr) {
        elevationValues.push_back(elevation);
        
        // Evaluate the color expression for this stop
        expression::EvaluationContext context(0.0f);
        context.elevation = elevation;
        
        auto result = colorExpr.evaluate(context);
        if (result) {
            auto colorOpt = expression::fromExpressionValue<Color>(*result);
            if (colorOpt) {
                colors.push_back(*colorOpt);
            } else {
                colors.push_back(Color::black());
            }
        } else {
            colors.push_back(Color::black());
        }
    });

    if (elevationValues.empty()) {
        Log::Error(Event::Render, "No stops found in interpolate expression!");
        return;
    }

    Log::Info(Event::Render, "Extracted " + std::to_string(elevationValues.size()) + " stops from expression");

    // Now sample these stops across our 256-element texture
    const float minElev = elevationValues.front();
    const float maxElev = elevationValues.back();
    
    Log::Info(Event::Render, "Elevation range: " + std::to_string(minElev) + "m to " + std::to_string(maxElev) + "m");

    for (uint32_t i = 0; i < colorRampSize; ++i) {
        float t = static_cast<float>(i) / (colorRampSize - 1);
        float elevation = minElev + t * (maxElev - minElev);

        // Find the two stops that bracket this elevation
        size_t lowerIdx = 0;
        for (size_t j = 0; j < elevationValues.size(); ++j) {
            if (elevationValues[j] <= elevation) {
                lowerIdx = j;
            } else {
                break;
            }
        }
        
        size_t upperIdx = std::min(lowerIdx + 1, elevationValues.size() - 1);
        
        // Interpolate color between the two stops
        Color color;
        if (lowerIdx == upperIdx) {
            color = colors[lowerIdx];
        } else {
            float stopT = (elevation - elevationValues[lowerIdx]) / 
                         (elevationValues[upperIdx] - elevationValues[lowerIdx]);
            stopT = std::clamp(stopT, 0.0f, 1.0f);
            
            const Color& c1 = colors[lowerIdx];
            const Color& c2 = colors[upperIdx];
            color = Color(
                c1.r + stopT * (c2.r - c1.r),
                c1.g + stopT * (c2.g - c1.g),
                c1.b + stopT * (c2.b - c1.b),
                c1.a + stopT * (c2.a - c1.a)
            );
        }

        // Store elevation as raw float
        auto* elevData = reinterpret_cast<float*>(elevationStops->data.get());
        elevData[i] = elevation;

        // Store color in RGBA format
        colorStops->data[i * 4 + 0] = static_cast<uint8_t>(color.r * 255);
        colorStops->data[i * 4 + 1] = static_cast<uint8_t>(color.g * 255);
        colorStops->data[i * 4 + 2] = static_cast<uint8_t>(color.b * 255);
        colorStops->data[i * 4 + 3] = static_cast<uint8_t>(color.a * 255);
    }

    colorRampChanged = true;
    Log::Info(Event::Render, "updateColorRamp complete");
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
            demTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                                 .wrapU = gfx::TextureWrapType::Clamp,
                                                 .wrapV = gfx::TextureWrapType::Clamp});
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
            Log::Info(Event::Render, "Tile updated (existing drawable)");
            continue;
        }

        Log::Info(Event::Render, "Creating new drawable for tile");

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
        demTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                             .wrapU = gfx::TextureWrapType::Clamp,
                                             .wrapV = gfx::TextureWrapType::Clamp});
        builder->setTexture(demTexture, idColorReliefImageTexture);

        // Bind color ramp textures
        if (elevationStopsTexture) {
            builder->setTexture(elevationStopsTexture, idColorReliefElevationStopsTexture);
        }
        if (colorStopsTexture) {
            builder->setTexture(colorStopsTexture, idColorReliefColorStopsTexture);
        }

        Log::Info(Event::Render, "Flushing drawable builder");
        builder->flush(context);

        for (auto& drawable : builder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);

            // Set up tile properties UBO
            shaders::ColorReliefTilePropsUBO tilePropsUBO;

            // Get DEM unpack vector from the actual data (supports both Terrain-RGB and Terrarium)
            const auto& demData = bucket.getDEMData();
            const auto unpackVector = demData.getUnpackVector();
            tilePropsUBO.unpack = {{unpackVector[0], unpackVector[1], unpackVector[2], unpackVector[3]}};

            // Texture dimensions
            tilePropsUBO.dimension = {{static_cast<float>(demData.dim), static_cast<float>(demData.dim)}};

            // Color ramp size
            tilePropsUBO.color_ramp_size = static_cast<int32_t>(colorRampSize);
            tilePropsUBO.pad_tile0 = 0.0f;

            auto& drawableUniforms = drawable->mutableUniformBuffers();
            drawableUniforms.createOrUpdate(idColorReliefTilePropsUBO, &tilePropsUBO, context);

            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
            Log::Info(Event::Render, "Drawable added to layer group");
        }
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
