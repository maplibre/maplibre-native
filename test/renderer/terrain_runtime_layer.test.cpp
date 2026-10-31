#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <mapbox/geojson.hpp>

#include <memory>
#include <string>

using namespace mln;

namespace {

// A red line running north-south a little west of the map center, over flat terrain at sea level,
// seen straight down at zoom 16.
constexpr const char* lineGeoJSON =
    R"({"type": "Feature", "properties": {}, "geometry": {"type": "LineString", "coordinates": [[9.9995, 45.99], [9.9995, 46.01]]}})";

std::string terrainStyle(bool withLine) {
    std::string style = R"({"version": 8, "sources": {
      "dem": {"type": "raster-dem", "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"], "encoding": "mapbox",
              "maxzoom": 11, "tileSize": 256})";
    if (withLine) style += std::string(R"(, "line": {"type": "geojson", "data": )") + lineGeoJSON + "}";
    style += R"(}, "terrain": {"source": "dem", "exaggeration": 1.0},
      "layers": [{"id": "bg", "type": "background", "paint": {"background-color": "white"}})";
    if (withLine) {
        style +=
            R"(, {"id": "line", "type": "line", "source": "line", "paint": {"line-color": "red", "line-width": 16}})";
    }
    return style + "]}";
}

std::string seaLevelDEMTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t i = 0; i < 256 * 256; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        // Sea level in Terrain-RGB: (0 + 10000) / 0.1 = 100000 = 1 * 65536 + 134 * 256 + 160
        px[0] = 1;
        px[1] = 134;
        px[2] = 160;
        px[3] = 255;
    }
    return encodePNG(image);
}

// Red pixels in the band where the line is (about 47 px west of the center, 16 px wide).
int redInLineBand(const PremultipliedImage& image) {
    int red = 0;
    for (uint32_t y = 16; y < 240; ++y) {
        for (uint32_t x = 70; x < 92; ++x) {
            const uint8_t* px = image.data.get() + (y * image.size.width + x) * 4;
            red += px[0] > 150 && px[1] < 90 && px[2] < 90 ? 1 : 0;
        }
    }
    return red;
}

class TerrainStyleChangeTest {
public:
    explicit TerrainStyleChangeTest(TerrainLoadMode loadMode)
        : fileSource(std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions())),
          dem(seaLevelDEMTile()),
          frontend({256, 256}, 1),
          map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(dem);
            return res;
        };
        map.setTerrainLoadMode(loadMode);
        map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(16.0).withPitch(0.0));
    }

    int renderRedInLineBand() {
        PremultipliedImage image;
        for (int i = 0; i < 4; ++i) {
            loop.runOnce();
            image = frontend.render(map).image;
        }
        return redInLineBand(image);
    }

    void addLine() {
        auto source = std::make_unique<style::GeoJSONSource>("line");
        source->setGeoJSON(mapbox::geojson::parse(lineGeoJSON));
        map.getStyle().addSource(std::move(source));
        auto layer = std::make_unique<style::LineLayer>("line", "line");
        layer->setLineColor(Color::red());
        layer->setLineWidth(16.0f);
        map.getStyle().addLayer(std::move(layer));
    }

    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource;
    std::string dem;
    HeadlessFrontend frontend;
    MapAdapter map;
};

} // namespace

// An app adds its route once the map is up. The drapes rendered before it must take it in: the
// fork used to drop every drape target when a layer or source was added, and the route then never
// reached the terrain.
TEST(TerrainDrape, ALineAddedAfterTheFirstFrameIsDraped) {
    for (const auto loadMode : {TerrainLoadMode::Quality, TerrainLoadMode::Balanced}) {
        int expected = 0;
        {
            TerrainStyleChangeTest fromTheStart(loadMode);
            fromTheStart.map.getStyle().loadJSON(terrainStyle(true));
            expected = fromTheStart.renderRedInLineBand();
        }
        ASSERT_GT(expected, 1000) << "the line drapes when the style has it from the start";

        TerrainStyleChangeTest addedLater(loadMode);
        addedLater.map.getStyle().loadJSON(terrainStyle(false));
        EXPECT_EQ(0, addedLater.renderRedInLineBand());
        addedLater.addLine();
        const int red = addedLater.renderRedInLineBand();
        EXPECT_GT(red, expected * 9 / 10) << (loadMode == TerrainLoadMode::Quality ? "Quality" : "Balanced") << ": "
                                          << red << " of " << expected << " red pixels";
    }
}

// A new style over terrain replaces what the drapes showed.
TEST(TerrainDrape, AStyleSwitchReplacesTheDrape) {
    TerrainStyleChangeTest test(TerrainLoadMode::Balanced);
    test.map.getStyle().loadJSON(terrainStyle(false));
    EXPECT_EQ(0, test.renderRedInLineBand());
    test.map.getStyle().loadJSON(terrainStyle(true));
    EXPECT_GT(test.renderRedInLineBand(), 1000);
}
