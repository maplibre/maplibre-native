#include <mbgl/gfx/rendering_stats.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/sources/custom_geometry_source.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/util/monotonic_timer.hpp>

#include <initializer_list>
#include <sstream>
#include <iomanip>

namespace mbgl {
namespace gfx {

bool RenderingStats::isZero() const {
    const auto expectedZeros = {numActiveTextures,
                                numTextureBindings,
                                numBuffers,
                                numVertexBuffers,
                                numIndexBuffers,
                                numUniformBuffers,
                                numFrameBuffers,
                                memTextures,
                                memBuffers,
                                memIndexBuffers,
                                memVertexBuffers,
                                memUniformBuffers};
    return std::ranges::all_of(expectedZeros, [](auto x) { return x == 0; });
}

RenderingStats& RenderingStats::operator+=(const RenderingStats& r) {
    encodingTime += r.encodingTime;
    renderingTime += r.renderingTime;
    numFrames += r.numFrames;
    numDrawCalls += r.numDrawCalls;
    totalDrawCalls += r.totalDrawCalls;
    numCreatedTextures += r.numCreatedTextures;
    numActiveTextures += r.numActiveTextures;
    numTextureBindings += r.numTextureBindings;
    numTextureUpdates += r.numTextureUpdates;
    textureUpdateBytes += r.textureUpdateBytes;
    totalBuffers += r.totalBuffers;
    totalBufferObjs += r.totalBufferObjs;
    bufferUpdates += r.bufferUpdates;
    bufferObjUpdates += r.bufferObjUpdates;
    bufferUpdateBytes += r.bufferUpdateBytes;
    numBuffers += r.numBuffers;
    numFrameBuffers += r.numFrameBuffers;
    numIndexBuffers += r.numIndexBuffers;
    indexUpdateBytes += r.indexUpdateBytes;
    numVertexBuffers += r.numVertexBuffers;
    vertexUpdateBytes += r.vertexUpdateBytes;
    numUniformBuffers += r.numUniformBuffers;
    numUniformUpdates += r.numUniformUpdates;
    uniformUpdateBytes += r.uniformUpdateBytes;
    memTextures += r.memTextures;
    memBuffers += r.memBuffers;
    memIndexBuffers += r.memIndexBuffers;
    memVertexBuffers += r.memVertexBuffers;
    memUniformBuffers += r.memUniformBuffers;
    stencilClears += r.stencilClears;
    stencilUpdates += r.stencilUpdates;
    return *this;
}

#if !defined(NDEBUG)
template <typename T>
std::ostream& optionalStatLine(std::ostream& stream, T value, std::string_view label, std::string_view sep) {
    if (value) {
        stream << label << " = " << value << sep;
    }
    return stream;
}
std::string RenderingStats::toString(std::string_view sep) const {
    std::stringstream ss;
    ss.precision(2);

    optionalStatLine(ss, encodingTime, "encodingTime", sep);
    optionalStatLine(ss, renderingTime, "renderingTime", sep);
    optionalStatLine(ss, numFrames, "numFrames", sep);
    optionalStatLine(ss, numDrawCalls, "numDrawCalls", sep);
    optionalStatLine(ss, totalDrawCalls, "totalDrawCalls", sep);
    optionalStatLine(ss, numCreatedTextures, "numCreatedTextures", sep);
    optionalStatLine(ss, numActiveTextures, "numActiveTextures", sep);
    optionalStatLine(ss, numTextureBindings, "numTextureBindings", sep);
    optionalStatLine(ss, numTextureUpdates, "numTextureUpdates", sep);
    optionalStatLine(ss, textureUpdateBytes, "textureUpdateBytes", sep);
    optionalStatLine(ss, totalBuffers, "totalBuffers", sep);
    optionalStatLine(ss, totalBufferObjs, "totalBufferObjs", sep);
    optionalStatLine(ss, bufferUpdates, "bufferUpdates", sep);
    optionalStatLine(ss, bufferObjUpdates, "bufferObjUpdates", sep);
    optionalStatLine(ss, bufferUpdateBytes, "bufferUpdateBytes", sep);
    optionalStatLine(ss, numBuffers, "numBuffers", sep);
    optionalStatLine(ss, numFrameBuffers, "numFrameBuffers", sep);
    optionalStatLine(ss, numIndexBuffers, "numIndexBuffers", sep);
    optionalStatLine(ss, indexUpdateBytes, "indexUpdateBytes", sep);
    optionalStatLine(ss, numVertexBuffers, "numVertexBuffers", sep);
    optionalStatLine(ss, vertexUpdateBytes, "vertexUpdateBytes", sep);
    optionalStatLine(ss, numUniformBuffers, "numUniformBuffers", sep);
    optionalStatLine(ss, numUniformUpdates, "numUniformUpdates", sep);
    optionalStatLine(ss, uniformUpdateBytes, "uniformUpdateBytes", sep);
    optionalStatLine(ss, memTextures, "memTextures", sep);
    optionalStatLine(ss, memBuffers, "memBuffers", sep);
    optionalStatLine(ss, memIndexBuffers, "memIndexBuffers", sep);
    optionalStatLine(ss, memVertexBuffers, "memVertexBuffers", sep);
    optionalStatLine(ss, memUniformBuffers, "memUniformBuffers", sep);
    optionalStatLine(ss, stencilClears, "stencilClears", sep);
    optionalStatLine(ss, stencilUpdates, "stencilUpdates", sep);
    return ss.str();
}
#endif

void RenderingStatsView::create(style::Style& style) {
    if (!style.getSource(sourceID)) {
        style::CustomGeometrySource::Options sourceOptions;

        sourceOptions.zoomRange = {0, 0};

        sourceOptions.fetchTileFunction = [&](const CanonicalTileID& tileID) {
            auto source = static_cast<style::CustomGeometrySource*>(style.getSource(sourceID));

            if (!source) {
                return;
            }

            mbgl::FeatureCollection features;

            mapbox::geojson::feature feature;
            feature.geometry = mapbox::geometry::geometry<double>(Point<double>{0, 0});

            features.emplace_back(feature);
            source->setTileData(tileID, features);
        };

        style.addSource(std::make_unique<style::CustomGeometrySource>(sourceID, sourceOptions));
    }

    if (!style.getLayer(layerID)) {
        auto infoLayer = std::make_unique<style::SymbolLayer>(layerID, sourceID);

        // required
        infoLayer->setSymbolScreenSpace(true);
        infoLayer->setTextAllowOverlap(true);
        infoLayer->setIconAllowOverlap(true);
        infoLayer->setTextIgnorePlacement(true);

        // customizable
        infoLayer->setTextColor(options.textColor);
        infoLayer->setTextSize(options.textSize);
        infoLayer->setTextMaxWidth(300.0f);
        infoLayer->setTextJustify(mbgl::style::TextJustifyType::Left);
        infoLayer->setTextAnchor(mbgl::style::SymbolAnchorType::TopRight);

        const float translation = mbgl::util::EXTENT / 2.0f - 100.0f;
        infoLayer->setTextTranslateAnchor(mbgl::style::TranslateAnchorType::Viewport);
        infoLayer->setTextTranslate(std::array<float, 2>{translation, translation});

        style.addLayer(std::move(infoLayer));
    }
}

void RenderingStatsView::destroy(style::Style& style) {
    style.removeLayer(layerID);
    style.removeSource(sourceID);
}

mbgl::style::SymbolLayer* RenderingStatsView::getLayer(style::Style& style) {
    return static_cast<mbgl::style::SymbolLayer*>(style.getLayer(layerID));
}

namespace {
template <typename T>
void printNumber(std::stringstream& ss, const std::string_view label, const T& value, bool print) {
    if (!print) {
        return;
    }

    ss << label << ": ";

    if (value >= 1e9) {
        ss << value / 1e9 << "B";
    } else if (value >= 1e6) {
        ss << value / 1e6 << "M";
    } else if (value >= 1e3) {
        ss << value / 1e3 << "K";
    } else {
        ss << value;
    }

    ss << "\n";
};

template <typename T>
void printMemory(std::stringstream& ss, const std::string_view label, const T& value, bool print) {
    if (!print) {
        return;
    }

    constexpr auto base = 1024;
    constexpr auto kb = base;
    constexpr auto mb = kb * base;
    constexpr auto gb = mb * base;

    ss << label << ": ";

    if (value >= gb) {
        ss << value / gb << "GB";
    } else if (value >= mb) {
        ss << value / mb << "MB";
    } else if (value >= kb) {
        ss << value / kb << "KB";
    } else {
        ss << value << "B";
    }

    ss << "\n";
};

} // namespace

void RenderingStatsView::update(style::Style& style, const gfx::RenderingStats& stats) {
    auto layer = getLayer(style);

    // style reloaded? layer got removed?
    if (!layer) {
        create(style);
        layer = getLayer(style);
    }

    if (!layer) {
        return;
    }

    ++frameCount;
    encodingTime += stats.encodingTime;
    renderingTime += stats.renderingTime;

    const auto currentTime = util::MonotonicTimer::now().count();
    if (currentTime - lastUpdate < options.updateInterval) {
        return;
    }

    std::stringstream ss;
    ss << std::setprecision(3) << std::fixed;

    ss << "Encoding time (ms): " << std::setw(7) << encodingTime / frameCount * 1000 << "\n";
    ss << "Rendering time (ms): " << std::setw(7) << renderingTime / frameCount * 1000 << "\n";

    printNumber(ss, "Frame count", stats.numFrames, true);
    printNumber(ss, "Draw calls", stats.numDrawCalls, true);
    printNumber(ss, "Total draw calls", stats.totalDrawCalls, options.verbose);

    printNumber(ss, "Textures", stats.numActiveTextures, true);
    printNumber(ss, "Total textures", stats.numCreatedTextures, options.verbose);
    printNumber(ss, "Texture updates", stats.numTextureUpdates, options.verbose);
    printMemory(ss, "Texture updates", stats.textureUpdateBytes, options.verbose);

    printNumber(ss, "Buffers", stats.numBuffers, true);
    printNumber(ss, "Total buffers", stats.totalBuffers, options.verbose);
    printNumber(ss, "Total buffer updates", stats.bufferUpdates, options.verbose);
    printMemory(ss, "Total buffer updates", stats.bufferUpdateBytes, options.verbose);

    printNumber(ss, "Index buffers", stats.numIndexBuffers, true);
    printMemory(ss, "Index buffers updates", stats.indexUpdateBytes, options.verbose);

    printNumber(ss, "Vertex buffers", stats.numVertexBuffers, true);
    printMemory(ss, "Vertex buffers updates", stats.vertexUpdateBytes, options.verbose);

    printNumber(ss, "Uniform buffers", stats.numUniformBuffers, true);
    printNumber(ss, "Uniform buffer updates", stats.numUniformUpdates, options.verbose);
    printMemory(ss, "Uniform buffer updates", stats.uniformUpdateBytes, options.verbose);

    printMemory(ss, "Texture memory", stats.memTextures, true);
    printMemory(ss, "Buffer memory", stats.memBuffers, true);
    printMemory(ss, "Index buffer memory", stats.memIndexBuffers, true);
    printMemory(ss, "Vertex buffer memory", stats.memVertexBuffers, true);
    printMemory(ss, "Uniform buffer memory", stats.memUniformBuffers, true);

    printNumber(ss, "Stencil buffer clears", stats.stencilClears, true);
    printNumber(ss, "Stencil buffer updates", stats.stencilUpdates, options.verbose);

    layer->setTextField(mbgl::style::expression::Formatted(ss.str().c_str()));

    frameCount = 0;
    encodingTime = 0.0;
    renderingTime = 0.0;
    lastUpdate = currentTime;
}

} // namespace gfx
} // namespace mbgl
