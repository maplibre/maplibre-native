#include <mbgl/renderer/layers/hillshade_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/util/geo.hpp>
#include <iostream>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

// Structure to hold processed illumination properties
struct IlluminationProperties {
    std::vector<float> directionRadians;
    std::vector<float> altitudeRadians;
    std::vector<Color> shadowColors;
    std::vector<Color> highlightColors;

    size_t numSources() const { return directionRadians.size(); }
};

// Convert style properties to illumination properties
IlluminationProperties getIlluminationProperties(const HillshadePaintProperties::PossiblyEvaluated& evaluated) {
    IlluminationProperties props;

    // Get the values from evaluated properties (these are now vectors)
    std::vector<float> directions = evaluated.get<HillshadeIlluminationDirection>();
    std::vector<float> altitudes = evaluated.get<HillshadeIlluminationAltitude>();
    std::vector<Color> highlights = evaluated.get<HillshadeHighlightColor>();
    std::vector<Color> shadows = evaluated.get<HillshadeShadowColor>();

    // ADD DEBUG LOGGING:
    std::cout << "DEBUG - Altitudes: ";
    for (auto v : altitudes) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "DEBUG - Directions: ";
    for (auto v : directions) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "DEBUG - Highlights: ";
    for (const auto& c : highlights) 
        std::cout << "(" << c.r << "," << c.g << "," << c.b << "," << c.a << ") ";
    std::cout << "\n";

    // Find the maximum length to ensure all arrays are the same size
    size_t maxLength = std::max({directions.size(), altitudes.size(), highlights.size(), shadows.size()});

    // Ensure we don't exceed the maximum supported
    maxLength = std::min(maxLength, static_cast<size_t>(MAX_ILLUMINATION_SOURCES));

    // Pad shorter arrays by repeating the last element
    auto padArray = [maxLength](auto& arr) {
        if (arr.empty()) arr.push_back(typename std::decay<decltype(arr)>::type::value_type{});
        while (arr.size() < maxLength) {
            arr.push_back(arr.back());
        }
    };

    padArray(directions);
    padArray(altitudes);
    padArray(highlights);
    padArray(shadows);

    // Convert degrees to radians
    props.directionRadians.reserve(directions.size());
    for (float deg : directions) {
        props.directionRadians.push_back(util::deg2radf(deg));
    }

    props.altitudeRadians.reserve(altitudes.size());
    for (float deg : altitudes) {
        props.altitudeRadians.push_back(util::deg2radf(deg));
    }

    props.shadowColors = std::move(shadows);
    props.highlightColors = std::move(highlights);

    return props;
}

// Pack illumination properties into the UBO format
HillshadeEvaluatedPropsUBO packEvaluatedProps(const IlluminationProperties& illumination, const Color& accentColor) {
    HillshadeEvaluatedPropsUBO ubo{};

    // Pack accent color
    ubo.accent = {accentColor.r, accentColor.g, accentColor.b, accentColor.a};

    // Pack altitudes (up to 4)
    for (size_t i = 0; i < illumination.altitudeRadians.size() && i < 4; ++i) {
        ubo.altitudes[i] = illumination.altitudeRadians[i];
    }

    // Pack azimuths (up to 4)
    for (size_t i = 0; i < illumination.directionRadians.size() && i < 4; ++i) {
        ubo.azimuths[i] = illumination.directionRadians[i];
    }

    // Pack shadow colors (4 colors * 4 components = 16 floats)
    for (size_t i = 0; i < illumination.shadowColors.size() && i < 4; ++i) {
        const auto& color = illumination.shadowColors[i];
        ubo.shadows[i * 4 + 0] = color.r;
        ubo.shadows[i * 4 + 1] = color.g;
        ubo.shadows[i * 4 + 2] = color.b;
        ubo.shadows[i * 4 + 3] = color.a;
    }

    // Pack highlight colors (4 colors * 4 components = 16 floats)
    for (size_t i = 0; i < illumination.highlightColors.size() && i < 4; ++i) {
        const auto& color = illumination.highlightColors[i];
        ubo.highlights[i * 4 + 0] = color.r;
        ubo.highlights[i * 4 + 1] = color.g;
        ubo.highlights[i * 4 + 2] = color.b;
        ubo.highlights[i * 4 + 3] = color.a;
    }

    return ubo;
}

// Convert HillshadeMethodType enum to int for shader
int32_t methodToInt(HillshadeMethodType method) {
    return static_cast<int32_t>(method);
}

// Calculate latitude range for tile
std::array<float, 2> getLatRange(const UnwrappedTileID& id) {
    const LatLng latlng0 = LatLng(id);
    const LatLng latlng1 = LatLng(UnwrappedTileID(id.canonical.z, id.canonical.x, id.canonical.y + 1));
    return {{static_cast<float>(latlng0.latitude()), static_cast<float>(latlng1.latitude())}};
}

} // namespace

void HillshadeLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    // Get illumination properties
    auto illumination = getIlluminationProperties(evaluated);

    // Adjust azimuths if anchor is viewport
    if (evaluated.get<HillshadeIlluminationAnchor>() == HillshadeIlluminationAnchorType::Viewport) {
        float bearing = static_cast<float>(parameters.state.getBearing());
        for (auto& azimuth : illumination.directionRadians) {
            azimuth -= bearing;
        }
    }

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const HillshadeEvaluatedPropsUBO evaluatedPropsUBO = packEvaluatedProps(illumination,
                                                                                evaluated.get<HillshadeAccentColor>());
        parameters.context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &evaluatedPropsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idHillshadeEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<HillshadeDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<HillshadeTilePropsUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto matrix = getTileMatrix(
            tileID, parameters, {0.f, 0.f}, TranslateAnchorType::Viewport, false, false, drawable, true);

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i] = {
#else
        const HillshadeDrawableUBO drawableUBO = {
#endif
            /* .matrix = */ util::cast<float>(matrix)
        };

#if MLN_UBO_CONSOLIDATION
        tilePropsUBOVector[i] = {
#else
        const HillshadeTilePropsUBO tilePropsUBO = {
#endif
            .latrange = getLatRange(tileID),
            .exaggeration = evaluated.get<HillshadeExaggeration>(),
            .method = methodToInt(evaluated.get<style::HillshadeMethod>()),
            .num_lights = static_cast<int32_t>(illumination.numSources()),
            .pad0 = 0.0f,
            .pad1 = 0.0f,
            .pad2 = 0.0f
        };

#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idHillshadeDrawableUBO, &drawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idHillshadeTilePropsUBO, &tilePropsUBO, parameters.context);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    auto& context = parameters.context;
    const size_t drawableUBOVectorSize = sizeof(HillshadeDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(HillshadeTilePropsUBO) * tilePropsUBOVector.size();
    if (!tilePropsUniformBuffer || tilePropsUniformBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsUniformBuffer = context.createUniformBuffer(
            tilePropsUBOVector.data(), tilePropsUBOVectorSize, false, true);
    } else {
        tilePropsUniformBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    layerUniforms.set(idHillshadeDrawableUBO, drawableUniformBuffer);
    layerUniforms.set(idHillshadeTilePropsUBO, tilePropsUniformBuffer);
#endif
}

} // namespace mbgl
