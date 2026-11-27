// Custom layers are registered with the default layer manager on OpenGL and Vulkan only
#if MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_VULKAN

#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/layers/custom_layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>

#if MLN_RENDER_BACKEND_OPENGL
#include <mln/platform/gl_functions.hpp>
#include <mln/gl/defines.hpp>
#include <mln/style/layers/gl/custom_layer_render_parameters.hpp>
#else
#include <mln/style/layers/vulkan/custom_layer_render_parameters.hpp>
#endif

#include <cmath>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

using namespace mln;

namespace {

// What a custom layer is handed over 3D terrain: the exaggeration, and the terrain's packed depth texture
// (OpenGL: the texture name, Vulkan: the image view, sampler and layout). The scene is the occlusion test's: a plain at
// sea level with a 3000 m ridge across it, the camera looking north at pitch 60.
constexpr double centreLat = 46.0;
constexpr double centreLon = 10.0;
constexpr double metresPerDegreeLat = 111195.0;
constexpr double ridgeSouthLat = centreLat + 2000.0 / metresPerDegreeLat;
constexpr double ridgeNorthLat = centreLat + 3000.0 / metresPerDegreeLat;
constexpr uint32_t demTileSize = 256;
constexpr uint32_t frameSize = 512;
constexpr double exaggeration = 1.5;

void encodeHeight(uint8_t* px, double metres) {
    const auto value = static_cast<uint32_t>(std::lround((metres + 10000.0) * 10.0));
    px[0] = static_cast<uint8_t>(value >> 16);
    px[1] = static_cast<uint8_t>((value >> 8) & 0xff);
    px[2] = static_cast<uint8_t>(value & 0xff);
    px[3] = 255;
}

std::string makeRidgeTile(const CanonicalTileID& id) {
    PremultipliedImage image({demTileSize, demTileSize});
    const double tiles = std::pow(2.0, id.z);
    for (uint32_t y = 0; y < demTileSize; ++y) {
        const double worldY = (id.y + (y + 0.5) / demTileSize) / tiles;
        const double lat = util::rad2deg(std::atan(std::sinh(std::numbers::pi * (1.0 - 2.0 * worldY))));
        const double metres = lat >= ridgeSouthLat && lat <= ridgeNorthLat ? 3000.0 : 0.0;
        for (uint32_t x = 0; x < demTileSize; ++x) {
            encodeHeight(image.data.get() + (y * demTileSize + x) * 4, metres);
        }
    }
    return encodePNG(image);
}

const std::string terrainStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {"type": "raster-dem", "tiles": ["http://example.com/{z}-{x}-{y}.png"],
            "encoding": "mapbox", "maxzoom": 12, "tileSize": 256}
  },
  "TERRAIN": 0,
  "layers": [{"id": "background", "type": "background", "paint": {"background-color": "white"}}]
})STYLE";

// What the custom layer saw on its last draw
struct Seen {
    int draws = 0;
    double terrainExaggeration = -1;
    bool hasDepthTexture = false;
    // The same, seen by preRender() in the last frame
    int preRenders = 0;
    bool preRenderHasDepthTexture = false;
    // OpenGL only: how many depth texture pixels are not the far plane's white
    int coveredPixels = -1;
    // Vulkan only
    bool depthSamplerSet = false;
    bool depthLayoutShaderRead = false;
};

class RecordingHost : public style::CustomLayerHost {
public:
    explicit RecordingHost(Seen& seen_)
        : seen(seen_) {}

    void initialize(const style::CustomLayerInitParameters&) override {}
    void contextLost() override {}
    void deinitialize() override {}

    void preRender(const gfx::Context&, const style::CustomLayerRenderParameters& parameters) override {
        seen.preRenders++;
#if MLN_RENDER_BACKEND_OPENGL
        seen.preRenderHasDepthTexture =
            static_cast<const style::gl::CustomLayerRenderParameters&>(parameters).terrainDepthTexture != 0;
#else
        seen.preRenderHasDepthTexture = static_cast<bool>(
            static_cast<const style::vulkan::CustomLayerRenderParameters&>(parameters).terrainDepthImageView);
#endif
    }

    void render(const style::CustomLayerRenderParameters& parameters) override {
        seen.draws++;
        seen.terrainExaggeration = parameters.terrainExaggeration;
#if MLN_RENDER_BACKEND_OPENGL
        const auto& gl = static_cast<const style::gl::CustomLayerRenderParameters&>(parameters);
        seen.hasDepthTexture = gl.terrainDepthTexture != 0;
        seen.coveredPixels = seen.hasDepthTexture ? countCoveredPixels(gl.terrainDepthTexture) : -1;
#else
        const auto& vulkan = static_cast<const style::vulkan::CustomLayerRenderParameters&>(parameters);
        seen.hasDepthTexture = static_cast<bool>(vulkan.terrainDepthImageView);
        seen.depthSamplerSet = static_cast<bool>(vulkan.terrainDepthSampler);
        seen.depthLayoutShaderRead = vulkan.terrainDepthImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal;
#endif
    }

private:
#if MLN_RENDER_BACKEND_OPENGL
    // Reads the depth texture back through a throwaway framebuffer. The custom layer tweaker rebinds the
    // drawable's framebuffer after render(), so nothing needs restoring here.
    static int countCoveredPixels(platform::GLuint texture) {
        using namespace platform;
        GLuint framebuffer = 0;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        int covered = -1;
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
            std::vector<uint8_t> pixels(static_cast<size_t>(frameSize) * frameSize * 4);
            glReadPixels(0, 0, frameSize, frameSize, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            covered = 0;
            for (size_t i = 0; i < pixels.size(); i += 4) {
                if (pixels[i] != 255 || pixels[i + 1] != 255 || pixels[i + 2] != 255 || pixels[i + 3] != 255) {
                    covered++;
                }
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &framebuffer);
        return covered;
    }
#endif

    Seen& seen;
};

Seen drawCustomLayer(bool terrainOn) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRidgeTile({static_cast<uint8_t>(resource.tileData->z),
                                                                static_cast<uint32_t>(resource.tileData->x),
                                                                static_cast<uint32_t>(resource.tileData->y)}));
        return res;
    };
    HeadlessFrontend frontend{{frameSize, frameSize}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));

    std::string style = terrainStyle;
    style.replace(style.find("\"TERRAIN\": 0,"),
                  std::string("\"TERRAIN\": 0,").size(),
                  terrainOn ? R"("terrain": {"source": "dem", "exaggeration": 1.5},)" : "");
    map.getStyle().loadJSON(style);

    Seen seen;
    map.getStyle().addLayer(std::make_unique<style::CustomLayer>("custom", std::make_unique<RecordingHost>(seen)));
    map.jumpTo(
        CameraOptions().withCenter(LatLng{centreLat, centreLon}).withZoom(12.0).withPitch(60.0).withBearing(0.0));

    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        frontend.render(map);
    }
    return seen;
}

} // namespace

TEST(TerrainCustomLayer, ReceivesTheExaggerationOverTerrain) {
    const Seen seen = drawCustomLayer(true);
    ASSERT_GT(seen.draws, 0);
    EXPECT_DOUBLE_EQ(seen.terrainExaggeration, exaggeration);
}

TEST(TerrainCustomLayer, ReceivesNoExaggerationOrDepthTextureWithoutTerrain) {
    const Seen seen = drawCustomLayer(false);
    ASSERT_GT(seen.draws, 0);
    EXPECT_DOUBLE_EQ(seen.terrainExaggeration, 0.0);
    EXPECT_FALSE(seen.hasDepthTexture);
}

TEST(TerrainCustomLayer, ReceivesTheTerrainDepthTexture) {
    const Seen seen = drawCustomLayer(true);
    ASSERT_GT(seen.draws, 0);
    EXPECT_TRUE(seen.hasDepthTexture) << "the depth pass has drawn before the main pass the layer is in";
#if MLN_RENDER_BACKEND_OPENGL
    // The terrain covers most of the frame; only the sky above the horizon stays the far plane's white
    EXPECT_GT(seen.coveredPixels, static_cast<int>(frameSize * frameSize / 4));
#else
    EXPECT_TRUE(seen.depthSamplerSet);
    EXPECT_TRUE(seen.depthLayoutShaderRead);
#endif
}

// preRender() is given the same subclass, with the depth texture of the frame before
TEST(TerrainCustomLayer, PreRenderReceivesTheTerrainDepthTexture) {
    const Seen seen = drawCustomLayer(true);
    ASSERT_GT(seen.preRenders, 0);
    EXPECT_TRUE(seen.preRenderHasDepthTexture);
}

#endif // MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_VULKAN
