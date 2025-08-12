#pragma once

#include <cstddef>
#include <string>
#include <memory>
#include <mbgl/util/color.hpp>

namespace mbgl {

namespace style {
class Style;
class SymbolLayer;
} // namespace style

namespace gfx {

struct RenderingStats {
    bool isZero() const;

    /// Frame CPU encoding time (seconds)
    double encodingTime = 0.0;
    /// Frame CPU rendering time (seconds)
    double renderingTime = 0.0;

    /// Number of frames rendered
    int numFrames = 0;
    /// Number of draw calls (`glDrawElements`, `drawIndexedPrimitives`, etc.) executed during the most recent frame
    int numDrawCalls = 0;
    /// Total number of draw calls executed during all the frames
    int totalDrawCalls = 0;

    /// Total number of textures created
    int numCreatedTextures = 0;
    /// Net textures
    int numActiveTextures = 0;
    /// Net texture bindings
    int numTextureBindings = 0;
    /// Number of times a texture was updated
    int numTextureUpdates = 0;
    /// Number of bytes used in texture updates
    std::size_t textureUpdateBytes = 0;

    /// Number of buffers created
    std::size_t totalBuffers = 0;
    /// Number of SDK-specific buffers created
    std::size_t totalBufferObjs = 0;
    /// Number of times a buffer is updated
    std::size_t bufferUpdates = 0;
    /// Number of times an SDK-specific buffer is updated
    std::size_t bufferObjUpdates = 0;
    /// Sum of update sizes
    std::size_t bufferUpdateBytes = 0;

    /// Number of active buffers
    int numBuffers = 0;
    /// Number of active offscreen frame buffers
    int numFrameBuffers = 0;

    /// Number of active index buffers
    int numIndexBuffers = 0;
    /// Sum of index buffers update sizes
    std::size_t indexUpdateBytes = 0;

    /// Number of active vertex buffers
    int numVertexBuffers = 0;
    /// Sum of vertex buffers update sizes
    std::size_t vertexUpdateBytes = 0;

    /// Number of active uniform buffers
    int numUniformBuffers = 0;
    /// Number of times a uniform buffer is updated
    int numUniformUpdates = 0;
    /// Sum of uniform buffers update sizes
    std::size_t uniformUpdateBytes = 0;

    /// Total texture memory
    int memTextures = 0;
    /// Total buffer memory
    int memBuffers = 0;
    /// Total index buffer memory
    int memIndexBuffers = 0;
    /// Total vertex buffer memory
    int memVertexBuffers = 0;
    /// Total uniform buffer memory
    int memUniformBuffers = 0;

    /// Number of stencil buffer clears
    int stencilClears = 0;
    /// Number of stencil buffer updates
    int stencilUpdates = 0;

    RenderingStats& operator+=(const RenderingStats&);

#if !defined(NDEBUG)
    std::string toString(std::string_view separator) const;
#endif
};

class RenderingStatsView final {
public:
    struct Options {
        float updateInterval = 0.25f;
        bool verbose = false;
        Color textColor = Color::red();
        float textSize = 4.0f;
    };

    RenderingStatsView() = default;
    RenderingStatsView(const Options& options_)
        : options(options_) {}
    ~RenderingStatsView() = default;

    void create(style::Style& style);
    void destroy(style::Style& style);

    mbgl::style::SymbolLayer* getLayer(style::Style& style);

    void update(style::Style& style, const gfx::RenderingStats& stats);

protected:
    const std::string layerID = "rendering-stats";
    const std::string sourceID = layerID + "-source";

    Options options;

    double lastUpdate = 0.0;
    uint32_t frameCount = 0;
    double encodingTime = 0.0;
    double renderingTime = 0.0;
};

} // namespace gfx
} // namespace mbgl
