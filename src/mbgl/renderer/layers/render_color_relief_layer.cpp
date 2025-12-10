#include <mbgl/renderer/layers/render_color_relief_layer.hpp>
#include <mbgl/renderer/layers/color_relief_layer_tweaker.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/style/layers/color_relief_layer_impl.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
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
    // Ensure data structures are initialized
    if (!elevationStopsData || !colorStops) {
        return;
    }

    // Get the color property value
    auto colorValue = unevaluated.get<ColorReliefColor>().getValue();
    if (colorValue.isUndefined()) {
        return;
    }

    std::vector<float> elevationStopsVector;
    std::vector<Color> colorStopsVector;

    // Get the expression from ColorRampPropertyValue
    // Note: getExpression() dereferences the internal pointer, so we must ensure isUndefined() is false first
    const mbgl::style::expression::Expression* exprPtr = nullptr;
    try {
        exprPtr = &colorValue.getExpression();
    } catch (...) {
        return;
    }
    if (!exprPtr) {
        return;
    }
    const mbgl::style::expression::Expression& expr = *exprPtr;

    if (expr.getKind() == mbgl::style::expression::Kind::Interpolate) {
        const auto* interpolate = static_cast<const mbgl::style::expression::Interpolate*>(&expr);

        size_t stopCount = interpolate->getStopCount();

        elevationStopsVector.reserve(stopCount);
        colorStopsVector.reserve(stopCount);

        // Extract elevation values from stops
        interpolate->eachStop([&](double elevation, const mbgl::style::expression::Expression& /*outputExpr*/) {
            elevationStopsVector.push_back(static_cast<float>(elevation));
        });

        // Evaluate expression at each elevation to get colors
        for (float elevation : elevationStopsVector) {
            Color color = {0.0f, 0.0f, 0.0f, 0.0f}; // Default to transparent black

            try {
                // Create evaluation context with elevation as color ramp parameter
                expression::EvaluationContext context(std::nullopt, nullptr, static_cast<double>(elevation));

                expression::EvaluationResult result = expr.evaluate(context);

                if (result && result->is<Color>()) {
                    color = result->get<Color>();
                }
            } catch (...) {
                // If evaluation fails, use the default transparent black
            }

            colorStopsVector.push_back(color);
        }

    } else {
        // Fallback: Sample the color ramp uniformly
        const uint32_t numSamples = 256;
        const float minElevation = -500.0f;
        const float maxElevation = 9000.0f;

        elevationStopsVector.reserve(numSamples);
        colorStopsVector.reserve(numSamples);

        for (uint32_t i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numSamples - 1);
            float elevation = minElevation + t * (maxElevation - minElevation);
            elevationStopsVector.push_back(elevation);

            Color color = colorValue.evaluate(static_cast<double>(elevation));
            colorStopsVector.push_back(color);
        }
    }

    const uint32_t rampSize = elevationStopsVector.size();
    if (rampSize == 0) {
        return;
    }

    // Resize and prepare structures
    elevationStopsData->resize(rampSize * 4); // RGBA float for compatibility
    colorStops->resize({rampSize, 1});
    this->colorRampSize = rampSize;

    for (uint32_t i = 0; i < rampSize; ++i) {
        // Store elevation in the R channel of an RGBA float vector
        (*elevationStopsData)[i * 4 + 0] = elevationStopsVector[i]; // R = elevation
        (*elevationStopsData)[i * 4 + 1] = 0.0f;                    // G = unused
        (*elevationStopsData)[i * 4 + 2] = 0.0f;                    // B = unused
        (*elevationStopsData)[i * 4 + 3] = 1.0f;                    // A = unused

        // Store colors without premultiplication for proper interpolation
        Color color = colorStopsVector[i];
        colorStops->data[i * 4 + 0] = static_cast<uint8_t>(color.r * 255.0f);
        colorStops->data[i * 4 + 1] = static_cast<uint8_t>(color.g * 255.0f);
        colorStops->data[i * 4 + 2] = static_cast<uint8_t>(color.b * 255.0f);
        colorStops->data[i * 4 + 3] = static_cast<uint8_t>(color.a * 255.0f);
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
        mbgl::Log::Error(mbgl::Event::Render, "ColorRelief shader failed to load");
        removeAllDrawables();
        return;
    }
    mbgl::Log::Info(mbgl::Event::Render,
                    "ColorRelief shader loaded, renderTiles count: " + std::to_string(renderTiles->size()));

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
    if (colorRampChanged && elevationStopsData && colorStops) {
        if (!elevationStopsTexture) {
            elevationStopsTexture = context.createTexture2D();
        }

        // Use RGBA32F instead of R32F for llvmpipe compatibility
        elevationStopsTexture->setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::Float);
        elevationStopsTexture->upload(elevationStopsData->data(), Size{colorRampSize, 1});
        elevationStopsTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Nearest,
                                                        .wrapU = gfx::TextureWrapType::Clamp,
                                                        .wrapV = gfx::TextureWrapType::Clamp});

        if (!colorStopsTexture) {
            colorStopsTexture = context.createTexture2D();
        }

        colorStopsTexture->setImage(colorStops);
        colorStopsTexture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                                    .wrapU = gfx::TextureWrapType::Clamp,
                                                    .wrapV = gfx::TextureWrapType::Clamp});

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

                if (const auto& attr = vertexAttrs->set(idColorReliefTexturePosVertexAttribute)) {
                    attr->setSharedRawData(vertices,
                                           offsetof(HillshadeLayoutVertex, a2),
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
            auto demImagePtr = bucket.getDEMData().getImagePtr();
            if (!demImagePtr || !demImagePtr->valid()) {
                return false;
            }
            std::shared_ptr<gfx::Texture2D> demTexture = context.createTexture2D();
            demTexture->setImage(demImagePtr);
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
        auto demImagePtr = bucket.getDEMData().getImagePtr();
        if (!demImagePtr || !demImagePtr->valid()) {
            mbgl::Log::Warning(mbgl::Event::Render, "ColorRelief: DEM image not valid for tile");
            continue; // Skip this tile if DEM data is not ready
        }

        const auto& demData = bucket.getDEMData();
        mbgl::Log::Info(
            mbgl::Event::Render,
            "ColorRelief DEM: dim=" + std::to_string(demData.dim) + ", stride=" + std::to_string(demData.stride) +
                ", imageSize=" + std::to_string(demImagePtr->size.width) + "x" +
                std::to_string(demImagePtr->size.height) + ", colorRampSize=" + std::to_string(colorRampSize));

        // Sample a few pixels from the DEM to verify data
        if (demImagePtr->data) {
            const auto* pixels = reinterpret_cast<const uint8_t*>(demImagePtr->data.get());
            // Sample center pixel
            size_t centerIdx = (demImagePtr->size.height / 2) * demImagePtr->size.width * 4 +
                               (demImagePtr->size.width / 2) * 4;
            uint8_t r = pixels[centerIdx], g = pixels[centerIdx + 1], b = pixels[centerIdx + 2],
                    a = pixels[centerIdx + 3];
            const auto unpackVector = demData.getUnpackVector();
            float elevation = r * unpackVector[0] + g * unpackVector[1] + b * unpackVector[2] - unpackVector[3];
            mbgl::Log::Info(mbgl::Event::Render,
                            "ColorRelief DEM center pixel: R=" + std::to_string(r) + " G=" + std::to_string(g) +
                                " B=" + std::to_string(b) + " A=" + std::to_string(a) +
                                " -> elevation=" + std::to_string(elevation) + "m" + ", unpack=[" +
                                std::to_string(unpackVector[0]) + "," + std::to_string(unpackVector[1]) + "," +
                                std::to_string(unpackVector[2]) + "," + std::to_string(unpackVector[3]) + "]");
        }

        std::shared_ptr<gfx::Texture2D> demTexture = context.createTexture2D();
        demTexture->setImage(demImagePtr);
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

        builder->flush(context);

        for (auto& drawable : builder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);

            // Set up tile properties UBO
            shaders::ColorReliefTilePropsUBO tilePropsUBO;

            const auto unpackVector = demData.getUnpackVector();
            tilePropsUBO.unpack = {{unpackVector[0], unpackVector[1], unpackVector[2], unpackVector[3]}};
            // Use stride (dim + 2) for dimension, as the texture includes a 1-pixel border
            tilePropsUBO.dimension = {{static_cast<float>(demData.stride), static_cast<float>(demData.stride)}};
            tilePropsUBO.color_ramp_size = static_cast<int32_t>(colorRampSize);
            tilePropsUBO.pad_tile0 = 0.0f;

            auto& drawableUniforms = drawable->mutableUniformBuffers();
            drawableUniforms.createOrUpdate(idColorReliefTilePropsUBO, &tilePropsUBO, context);

            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }
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
