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
#include <vector> 
#include <cstring> 

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

    // Initialize color ramp data
    colorRampSize = 256;
    // Initialization aligns with the updated header
    elevationStopsData = std::make_shared<std::vector<float>>(colorRampSize);
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
    auto colorValue = unevaluated.get<ColorReliefColor>().getValue();
    
    std::vector<float> elevationStopsVector;
    std::vector<Color> colorStopsVector;
    
    // Get the expression from ColorRampPropertyValue
    const mbgl::style::expression::Expression& expr = colorValue.getExpression();
    
    // FIX 1: Use the modern helper to check expression type
    if (mbgl::style::expression::is<mbgl::style::expression::Interpolate>(expr)) {
        // The cast is safe now as the type check passes
        const auto* interpolate = static_cast<const mbgl::style::expression::Interpolate*>(&expr);
        
        size_t stopCount = interpolate->getStopCount();
        Log::Info(Event::Render, "Found Interpolate expression with " + 
                 std::to_string(stopCount) + " stops");
        
        elevationStopsVector.reserve(stopCount);
        colorStopsVector.reserve(stopCount);
        
        // Extract elevation values from stops
        interpolate->eachStop([&](double elevation, const mbgl::style::expression::Expression& /*outputExpr*/) {
            elevationStopsVector.push_back(static_cast<float>(elevation));
        });
        
        // Now evaluate colors using PropertyValue::evaluate
        for (float elevation : elevationStopsVector) {
            Color color = colorValue.evaluate(elevation);
            colorStopsVector.push_back(color);
            
            Log::Info(Event::Render, "  Stop at " + std::to_string(elevation) + "m: " +
                     "rgba(" + std::to_string(color.r) + ", " +
                     std::to_string(color.g) + ", " +
                     std::to_string(color.b) + ", " +
                     std::to_string(color.a) + ")");
        }
        
        Log::Info(Event::Render, "Extracted " + std::to_string(elevationStopsVector.size()) + 
                 " stops from expression");
    } else {
        Log::Warning(Event::Render, "Expression is not an Interpolate, using fallback");
    }
    
    // Fallback: sample a range if no stops extracted
    if (elevationStopsVector.empty()) {
        Log::Info(Event::Render, "Using fallback sampling (-500m to 9000m, 256 samples)");
        
        const float minElevation = -500.0f;
        const float maxElevation = 9000.0f;
        const uint32_t numSamples = 256;
        
        elevationStopsVector.reserve(numSamples);
        colorStopsVector.reserve(numSamples);
        
        for (uint32_t i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numSamples - 1);
            float elevation = minElevation + t * (maxElevation - minElevation);
            elevationStopsVector.push_back(elevation);
            
            Color color = colorValue.evaluate(elevation);
            colorStopsVector.push_back(color);
        }
    }
    
    // Now use elevationStopsVector and colorStopsVector
    const uint32_t rampSize = elevationStopsVector.size();
    float minElevation = elevationStopsVector.front();
    float maxElevation = elevationStopsVector.back();
    
    Log::Info(Event::Render, "Final color ramp:");
    Log::Info(Event::Render, "  Size: " + std::to_string(rampSize) + " stops");
    Log::Info(Event::Render, "  Range: " + std::to_string(minElevation) + "m to " + 
             std::to_string(maxElevation) + "m");
    
    // FIX 2: Prepare data for GPU upload (use existing class members)
    this->elevationStopsData = std::make_shared<std::vector<float>>();
    this->elevationStopsData->reserve(rampSize * 4);  // RGBA
    
    // Re-initialize and size the existing colorStops PremultipliedImage (RGBA8 data)
    // The PremultipliedImage is expected to be a `PremultipliedImage` of size `rampSize x 1` with 4 channels (RGBA)
    this->colorStops = std::make_shared<PremultipliedImage>(Size{rampSize, 1});
    this->colorStops->resize({rampSize, 1}); 

    for (uint32_t i = 0; i < rampSize; ++i) {
        // Elevation stops (RGBA format for llvmpipe compatibility)
        this->elevationStopsData->push_back(elevationStopsVector[i]);  // R = elevation
        this->elevationStopsData->push_back(0.0f);                      // G = unused
        this->elevationStopsData->push_back(0.0f);                      // B = unused
        this->elevationStopsData->push_back(1.0f);                      // A = unused
        
        // Color stops (RGBA8 for PremultipliedImage)
        // Convert to unassociated color, premultiply, and store as 8-bit bytes.
        const auto premultiplied = util::premultiply(colorStopsVector[i].toUnassociated());
        this->colorStops->data[i * 4 + 0] = static_cast<uint8_t>(premultiplied.r * 255.0f);
        this->colorStops->data[i * 4 + 1] = static_cast<uint8_t>(premultiplied.g * 255.0f);
        this->colorStops->data[i * 4 + 2] = static_cast<uint8_t>(premultiplied.b * 255.0f);
        this->colorStops->data[i * 4 + 3] = static_cast<uint8_t>(premultiplied.a * 255.0f);
    }
    
    // Update class member
    this->colorRampSize = rampSize;
    
    // Upload elevation stops texture
    elevationStopsTexture->setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::Float);
    elevationStopsTexture->upload(this->elevationStopsData->data(), Size{rampSize, 1});
    
    // Upload color stops texture  
    // The colorStopsTexture->setImage(colorStops) is performed in the update() loop, so no
    // direct upload here is necessary, unlike the broken code that followed.
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
    if (colorRampChanged && elevationStopsData && colorStops) {
        Log::Info(Event::Render, "Updating color ramp textures");
        
        if (!elevationStopsTexture) {
            Log::Info(Event::Render, "Creating elevation stops texture");
            elevationStopsTexture = context.createTexture2D();
        }
        
        // Use RGBA32F instead of R32F for llvmpipe compatibility
        elevationStopsTexture->setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::Float);
        
        // Convert single-channel data to RGBA format  
        auto elevationRGBA = std::make_shared<std::vector<float>>(colorRampSize * 4);
        for (uint32_t i = 0; i < colorRampSize; ++i) {
            (*elevationRGBA)[i*4 + 0] = (*elevationStopsData)[i];  // R = elevation
            (*elevationRGBA)[i*4 + 1] = 0.0f;  // G = unused
            (*elevationRGBA)[i*4 + 2] = 0.0f;  // B = unused  
            (*elevationRGBA)[i*4 + 3] = 1.0f;  // A = unused
        }
        elevationStopsTexture->upload(elevationRGBA->data(), Size{colorRampSize, 1});

        Log::Info(Event::Render, "=== TEXTURE DEBUG ===");
        Log::Info(Event::Render, "getPixelStride: " + std::to_string(elevationStopsTexture->getPixelStride()));
        Log::Info(Event::Render, "getDataSize: " + std::to_string(elevationStopsTexture->getDataSize()));
        Log::Info(Event::Render, "Expected data size: " + std::to_string(colorRampSize * 4));
        Log::Info(Event::Render, "First 4 floats: " + 
                std::to_string((*elevationStopsData)[0]) + ", " +
                std::to_string((*elevationStopsData)[1]) + ", " +
                std::to_string((*elevationStopsData)[2]) + ", " +
                std::to_string((*elevationStopsData)[3]));
        
        // Set sampler state (already correct)
        elevationStopsTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Nearest,
                                                        .wrapU = gfx::TextureWrapType::Clamp,
                                                        .wrapV = gfx::TextureWrapType::Clamp});

        if (!colorStopsTexture) {
            Log::Info(Event::Render, "Creating color stops texture");
            colorStopsTexture = context.createTexture2D();
        }
        
        // The color stops texture is correctly handled by PremultipliedImage
        colorStopsTexture->setImage(colorStops);
        colorStopsTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                                    .wrapU = gfx::TextureWrapType::Clamp,
                                                    .wrapV = gfx::TextureWrapType::Clamp});

        colorRampChanged = false;
        Log::Info(Event::Render, "Color ramp textures updated");
        
        // Log texture binding info
        Log::Info(Event::Render, "Texture binding plan:");
        Log::Info(Event::Render, "  u_image (DEM) -> texture unit 0");
        Log::Info(Event::Render, "  u_elevation_stops (R32F) -> texture unit 1");
        Log::Info(Event::Render, "  u_color_stops (RGBA8) -> texture unit 2");
        Log::Info(Event::Render, "UBO values:");
        Log::Info(Event::Render, "  u_color_ramp_size: " + std::to_string(colorRampSize));
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

        // Bind DEM texture to unit 0
        std::shared_ptr<gfx::Texture2D> demTexture = context.createTexture2D();
        demTexture->setImage(bucket.getDEMData().getImagePtr());
        demTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                             .wrapU = gfx::TextureWrapType::Clamp,
                                             .wrapV = gfx::TextureWrapType::Clamp});
        builder->setTexture(demTexture, idColorReliefImageTexture);

        // Bind color ramp textures to units 1 and 2
        if (elevationStopsTexture) {
            builder->setTexture(elevationStopsTexture, idColorReliefElevationStopsTexture);
            Log::Info(Event::Render, "Bound elevation stops texture");
        } else {
            Log::Warning(Event::Render, "elevationStopsTexture is null!");
        }
        
        if (colorStopsTexture) {
            builder->setTexture(colorStopsTexture, idColorReliefColorStopsTexture);
            Log::Info(Event::Render, "Bound color stops texture");
        } else {
            Log::Warning(Event::Render, "colorStopsTexture is null!");
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

            Log::Info(Event::Render, "TilePropsUBO values:");
            Log::Info(Event::Render, "  u_unpack: [" + std::to_string(unpackVector[0]) + ", " +
                     std::to_string(unpackVector[1]) + ", " + std::to_string(unpackVector[2]) + ", " +
                     std::to_string(unpackVector[3]) + "]");
            Log::Info(Event::Render, "  u_dimension: " + std::to_string(demData.dim) + " x " + std::to_string(demData.dim));
            Log::Info(Event::Render, "  u_color_ramp_size: " + std::to_string(colorRampSize));

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
