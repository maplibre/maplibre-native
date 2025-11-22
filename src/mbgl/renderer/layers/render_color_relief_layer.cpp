#include <mbgl/renderer/layers/render_color_relief_layer.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/mat3.hpp>
#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

inline const ColorReliefLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == ColorReliefLayer::Impl::staticTypeInfo());
    return static_cast<const ColorReliefLayer::Impl&>(*impl);
}

RenderColorReliefLayer::RenderColorReliefLayer(Immutable<style::ColorReliefLayer::Impl> _impl)
    : RenderLayer(makeMutable<ColorReliefLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderColorReliefLayer::~RenderColorReliefLayer() = default;

void RenderColorReliefLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderColorReliefLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<ColorReliefLayerProperties>(
        staticImmutableCast<ColorReliefLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters));

    auto& evaluated_ = properties->evaluated;
    passes = RenderPass::Translucent;

    // Check if color ramp changed
    const auto previousColorRamp = std::move(evaluated.get<ColorReliefColor>());
    if (previousColorRamp != evaluated_.get<ColorReliefColor>()) {
        colorRampChanged = true;
    }

    evaluated = std::move(evaluated_);
    properties->renderPasses = makeMutable<RenderLayerPasses>(passes);
    evaluatedProperties = std::move(properties);
}

bool RenderColorReliefLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderColorReliefLayer::hasCrossfade() const {
    return false;
}

void RenderColorReliefLayer::markContextDestroyed() {
    colorRampTexture.reset();
    elevationStopsTexture.reset();
}

void RenderColorReliefLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    
    if (colorRampChanged) {
        updateColorRamp();
        colorRampChanged = false;
    }
}

void RenderColorReliefLayer::updateColorRamp() {
    const auto& colorRampValue = evaluated.get<ColorReliefColor>();
    
    if (!colorRampValue.expression) {
        // No color ramp specified, use transparent defaults
        const size_t count = 2;
        std::vector<uint8_t> elevationData(count * 4, 0);
        std::vector<uint8_t> colorData(count * 4, 0);
        
        elevationStopsTexture = gfx::Texture({count, 1},
                                             gfx::TexturePixelType::RGBA,
                                             gfx::TextureChannelDataType::UnsignedByte);
        elevationStopsTexture->upload(elevationData.data());
        
        colorRampTexture = gfx::Texture({count, 1},
                                        gfx::TexturePixelType::RGBA,
                                        gfx::TextureChannelDataType::UnsignedByte);
        colorRampTexture->upload(colorData.data());
        return;
    }

    // Parse the color ramp expression
    // The expression should be an interpolation expression with 'elevation' as the input
    std::vector<std::pair<float, Color>> stops;
    
    // Extract stops from expression
    // This assumes the expression is an interpolation with elevation input
    const auto* expr = colorRampValue.expression.get();
    
    if (const auto* interpolate = dynamic_cast<const expression::Interpolate*>(expr)) {
        // Get the stops from the interpolation expression
        const auto& stopMap = interpolate->getStops();
        
        for (const auto& stop : stopMap) {
            float elevation = stop.first;
            Color color = stop.second.get<Color>();
            stops.emplace_back(elevation, color);
        }
    } else {
        // Fallback: evaluate at sample points
        expression::EvaluationContext context(0.0f); // zoom level
        
        // Sample at reasonable elevation intervals
        const int sampleCount = 256;
        const float minElevation = 0.0f;
        const float maxElevation = 10000.0f; // 10km max
        
        for (int i = 0; i < sampleCount; i++) {
            float elevation = minElevation + (maxElevation - minElevation) * i / (sampleCount - 1);
            context.elevation = elevation;
            
            auto result = colorRampValue.expression->evaluate(context);
            if (result) {
                Color color = result->get<Color>();
                stops.emplace_back(elevation, color);
            }
        }
    }
    
    // Ensure we have at least 2 stops
    if (stops.empty()) {
        stops.emplace_back(0.0f, Color::transparent());
        stops.emplace_back(1.0f, Color::transparent());
    } else if (stops.size() == 1) {
        stops.emplace_back(stops[0].first + 1.0f, stops[0].second);
    }
    
    // Sort by elevation
    std::sort(stops.begin(), stops.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    // Create elevation stops texture (1D RGBA texture encoding floats)
    const size_t count = stops.size();
    std::vector<uint8_t> elevationData(count * 4);
    
    for (size_t i = 0; i < count; i++) {
        float elevation = stops[i].first;
        // Encode elevation as RGBA using DEM encoding (similar to terrain-rgb)
        // We'll use a simple byte encoding for now
        // For proper terrain encoding: elevation = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1)
        int encoded = static_cast<int>((elevation + 10000.0f) * 10.0f);
        elevationData[i * 4 + 0] = (encoded >> 16) & 0xFF; // R
        elevationData[i * 4 + 1] = (encoded >> 8) & 0xFF;  // G
        elevationData[i * 4 + 2] = (encoded >> 0) & 0xFF;  // B
        elevationData[i * 4 + 3] = 255;                    // A (unused, set to -1 in shader)
    }

    elevationStopsTexture = gfx::Texture({count, 1},
                                         gfx::TexturePixelType::RGBA,
                                         gfx::TextureChannelDataType::UnsignedByte);
    elevationStopsTexture->upload(elevationData.data());

    // Create color stops texture (1D RGBA texture)
    std::vector<uint8_t> colorData(count * 4);
    for (size_t i = 0; i < count; i++) {
        const Color& color = stops[i].second;
        colorData[i * 4 + 0] = static_cast<uint8_t>(color.r * 255);
        colorData[i * 4 + 1] = static_cast<uint8_t>(color.g * 255);
        colorData[i * 4 + 2] = static_cast<uint8_t>(color.b * 255);
        colorData[i * 4 + 3] = static_cast<uint8_t>(color.a * 255);
    }

    colorRampTexture = gfx::Texture({count, 1},
                                    gfx::TexturePixelType::RGBA,
                                    gfx::TextureChannelDataType::UnsignedByte);
    colorRampTexture->upload(colorData.data());
    
    colorRampSize = static_cast<int32_t>(count);
}

void RenderColorReliefLayer::render(PaintParameters& parameters) {
    if (renderTiles.empty()) return;
    if (!colorRampTexture || !elevationStopsTexture) return;

    const auto& evaluated_ = static_cast<const ColorReliefLayerProperties&>(*evaluatedProperties).evaluated;
    const float opacity = evaluated_.get<ColorReliefOpacity>();

    if (opacity <= 0.0f) return;

    auto& context = parameters.context;
    const auto& state = parameters.state;
    
    // Get the color relief program
    auto& programInstance = parameters.programs.getColorReliefLayerPrograms().colorRelief;

    // Set up common render state
    const gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    const gfx::StencilMode stencilMode = gfx::StencilMode::disabled();
    const gfx::ColorMode colorMode = parameters.colorModeForRenderPass();
    const gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();

    for (const RenderTile& tile : renderTiles) {
        const auto& bucket = tile.getBucket(*baseImpl);
        if (!bucket) continue;

        auto* demBucket = dynamic_cast<DEMBucket*>(bucket.get());
        if (!demBucket) continue;

        const auto& dem = demBucket->getDEMData();
        if (!dem || !dem.data) continue;

        const auto& demTexture = demBucket->getTexture();
        if (!demTexture) continue;

        // Calculate transformation matrix for this tile
        mat4 matrix;
        state.matrixFor(matrix, tile.id);
        
        // Apply tile-specific transformation
        matrix::multiply(matrix, parameters.alignedProjMatrix, matrix);

        // Set up drawable UBO
        ColorReliefDrawableUBO drawableUBO;
        std::copy(matrix, matrix + 16, drawableUBO.matrix.begin());

        // Set up tile properties UBO
        ColorReliefTilePropsUBO tilePropsUBO;
        const auto& unpackVector = dem.getUnpackVector();
        tilePropsUBO.unpack = {unpackVector[0], unpackVector[1], unpackVector[2], unpackVector[3]};
        
        const auto tileSize = dem.dim;
        tilePropsUBO.dimension = {static_cast<float>(tileSize), static_cast<float>(tileSize)};
        tilePropsUBO.color_ramp_size = colorRampSize;
        tilePropsUBO.pad0 = 0.0f;

        // Set up evaluated properties UBO
        ColorReliefEvaluatedPropsUBO evaluatedPropsUBO;
        evaluatedPropsUBO.opacity = opacity;
        evaluatedPropsUBO.pad0 = 0.0f;
        evaluatedPropsUBO.pad1 = 0.0f;
        evaluatedPropsUBO.pad2 = 0.0f;

        // Update UBO buffers
        auto& drawableUBOBuffer = context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        auto& tilePropsUBOBuffer = context.createUniformBuffer(&tilePropsUBO, sizeof(tilePropsUBO));
        auto& evaluatedPropsUBOBuffer = context.createUniformBuffer(&evaluatedPropsUBO, sizeof(evaluatedPropsUBO));

        // Create drawable for rendering
        auto drawable = context.createDrawable();
        
        // Bind vertex and index buffers (use raster tile quad)
        drawable->setVertexBuffer(parameters.staticData.rasterVertexBuffer);
        drawable->setIndexBuffer(parameters.staticData.quadTriangleIndexBuffer);
        
        // Bind UBOs
        drawable->setUniformBuffer(0, drawableUBOBuffer);
        drawable->setUniformBuffer(1, tilePropsUBOBuffer);
        drawable->setUniformBuffer(2, evaluatedPropsUBOBuffer);

        // Bind textures
        drawable->setTexture(demTexture, 0);  // u_image (DEM data)
        drawable->setTexture(*elevationStopsTexture, 1);  // u_elevation_stops
        drawable->setTexture(*colorRampTexture, 2);  // u_color_stops

        // Set render state
        drawable->setDepthMode(depthMode);
        drawable->setStencilMode(stencilMode);
        drawable->setColorMode(colorMode);
        drawable->setCullFaceMode(cullFaceMode);

        // Set the program
        drawable->setProgram(programInstance.getProgram());

        // Submit for rendering
        parameters.renderPass.pushDrawable(std::move(drawable));
    }
}

bool RenderColorReliefLayer::queryIntersectsFeature(const GeometryCoordinates&,
                                                     const GeometryTileFeature&,
                                                     float,
                                                     const TransformState&,
                                                     float,
                                                     const mat4&,
                                                     const FeatureState&) const {
    // Color relief layers don't support feature querying
    return false;
}

} // namespace mbgl
