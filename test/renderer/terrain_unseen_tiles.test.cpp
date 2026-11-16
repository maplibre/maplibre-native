#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/color.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace mln;

namespace {

// Thimble Peak, seen from the south at the zoom and pitch the phone sits at in native 3D.
const LatLng thimblePeak{38.65995, -120.06140};
constexpr double viewZoom = 13.75;
constexpr double thimblePeakMeters = 3930.0;
const Size phone{402, 874};

// The overlay sources of the app, each with the zoom range it is served at and the layers it draws with
// the point rows' floors of native 3D (PointRowHorizonPolicy) on: points and names at zoom 12, streams
// at 11, the names that follow a line at their own. Under the sources, satellite imagery and the DEM.
const char* appStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {"type": "raster-dem", "tiles": ["https://t.test/dem/{z}/{x}/{y}.png"],
            "encoding": "terrarium", "tileSize": 256, "maxzoom": 15},
    "imagery": {"type": "raster", "tiles": ["https://t.test/imagery/{z}/{x}/{y}.png"],
                "tileSize": 256, "maxzoom": 15},
    "backcountry_tile_features": {"type": "vector", "tiles": ["https://t.test/backcountry_tile_features/{z}/{x}/{y}"],
                                  "minzoom": 8, "maxzoom": 14},
    "public_land_label_points": {"type": "vector", "tiles": ["https://t.test/public_land_label_points/{z}/{x}/{y}"],
                                 "minzoom": 11, "maxzoom": 14},
    "mvum_roads": {"type": "vector", "tiles": ["https://t.test/mvum_roads/{z}/{x}/{y}"],
                   "minzoom": 9, "maxzoom": 14},
    "reference_trails": {"type": "vector", "tiles": ["https://t.test/reference_trails/{z}/{x}/{y}"],
                         "minzoom": 9, "maxzoom": 14},
    "reference_streams": {"type": "vector", "tiles": ["https://t.test/reference_streams/{z}/{x}/{y}"],
                          "minzoom": 8, "maxzoom": 14},
    "backcountry_label_points": {"type": "vector", "tiles": ["https://t.test/backcountry_label_points/{z}/{x}/{y}"],
                                 "minzoom": 7, "maxzoom": 14},
    "public_land_units": {"type": "vector", "tiles": ["https://t.test/public_land_units/{z}/{x}/{y}"],
                          "minzoom": 5, "maxzoom": 14}
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "imagery", "type": "raster", "source": "imagery", "layout": {"visibility": "none"}},
    {"id": "units-fill", "type": "fill", "source": "public_land_units", "source-layer": "overlay",
     "layout": {"visibility": "none"}, "paint": {"fill-color": "#33aa33", "fill-opacity": 0.3}},
    {"id": "units-line", "type": "line", "source": "public_land_units", "source-layer": "overlay",
     "layout": {"visibility": "none"}, "paint": {"line-color": "#117711", "line-width": 3}},
    {"id": "streams-casing", "type": "line", "source": "reference_streams", "source-layer": "overlay",
     "minzoom": 11, "layout": {"visibility": "none"}, "paint": {"line-color": "#ffffff", "line-width": 6}},
    {"id": "streams", "type": "line", "source": "reference_streams", "source-layer": "overlay",
     "minzoom": 11, "layout": {"visibility": "none"}, "paint": {"line-color": "#2255ff", "line-width": 2}},
    {"id": "features-fill", "type": "fill", "source": "backcountry_tile_features", "source-layer": "overlay",
     "layout": {"visibility": "none"}, "paint": {"fill-color": "#ff3300", "fill-opacity": 0.4}},
    {"id": "features-line", "type": "line", "source": "backcountry_tile_features", "source-layer": "overlay",
     "layout": {"visibility": "none"}, "paint": {"line-color": "#aa2200", "line-width": 3}},
    {"id": "features-point", "type": "circle", "source": "backcountry_tile_features", "source-layer": "overlay",
     "minzoom": 12, "layout": {"visibility": "none"}, "paint": {"circle-radius": 5}},
    {"id": "features-marker", "type": "symbol", "source": "backcountry_tile_features", "source-layer": "overlay",
     "minzoom": 12, "layout": {"visibility": "none"}},
    {"id": "trails", "type": "line", "source": "reference_trails", "source-layer": "overlay",
     "layout": {"visibility": "none"}, "paint": {"line-color": "#664400", "line-width": 2}},
    {"id": "trails-label", "type": "symbol", "source": "reference_trails", "source-layer": "overlay",
     "layout": {"visibility": "none", "symbol-placement": "line"}},
    {"id": "roads", "type": "line", "source": "mvum_roads", "source-layer": "overlay",
     "layout": {"visibility": "none"}, "paint": {"line-color": "#ffcc00", "line-width": 2}},
    {"id": "roads-label", "type": "symbol", "source": "mvum_roads", "source-layer": "overlay",
     "layout": {"visibility": "none", "symbol-placement": "line"}},
    {"id": "land-labels", "type": "symbol", "source": "public_land_label_points", "source-layer": "overlay",
     "minzoom": 12, "layout": {"visibility": "none"}},
    {"id": "peaks", "type": "symbol", "source": "backcountry_label_points", "source-layer": "overlay",
     "minzoom": 10, "layout": {"visibility": "none"}}
  ]
})STYLE";

// The layers of appStyle but the DEM, the ones the test shows once the DEM is in.
const char* const overlayLayers[] = {"imagery",
                                     "units-fill",
                                     "units-line",
                                     "streams-casing",
                                     "streams",
                                     "features-fill",
                                     "features-line",
                                     "features-point",
                                     "features-marker",
                                     "trails",
                                     "trails-label",
                                     "roads",
                                     "roads-label",
                                     "land-labels",
                                     "peaks"};

void varint(std::string& out, uint64_t value) {
    while (value >= 0x80) {
        out.push_back(static_cast<char>((value & 0x7f) | 0x80));
        value >>= 7;
    }
    out.push_back(static_cast<char>(value));
}

void bytesField(std::string& out, uint32_t number, const std::string& payload) {
    varint(out, (number << 3) | 2);
    varint(out, payload.size());
    out += payload;
}

void varintField(std::string& out, uint32_t number, uint64_t value) {
    varint(out, number << 3);
    varint(out, value);
}

uint32_t zigzag(int32_t n) {
    return (static_cast<uint32_t>(n) << 1) ^ static_cast<uint32_t>(n >> 31);
}

// A vector tile of one layer, "overlay", with one square polygon in the middle of the tile.
std::string overlayTile() {
    constexpr int32_t extent = 4096;
    constexpr int32_t from = extent / 8;
    constexpr int32_t side = extent * 3 / 4;
    std::string geometry;
    varint(geometry, (1 << 3) | 1); // MoveTo
    varint(geometry, zigzag(from));
    varint(geometry, zigzag(from));
    varint(geometry, (3 << 3) | 2); // LineTo, three points
    varint(geometry, zigzag(side));
    varint(geometry, zigzag(0));
    varint(geometry, zigzag(0));
    varint(geometry, zigzag(side));
    varint(geometry, zigzag(-side));
    varint(geometry, zigzag(0));
    varint(geometry, (1 << 3) | 7); // ClosePath

    std::string feature;
    varintField(feature, 1, 1); // id
    varintField(feature, 3, 3); // polygon
    bytesField(feature, 4, geometry);

    std::string layer;
    varintField(layer, 15, 2); // version
    bytesField(layer, 1, "overlay");
    bytesField(layer, 2, feature);
    varintField(layer, 5, extent);

    std::string tile;
    bytesField(tile, 3, layer);
    return tile;
}

// Ground at Thimble Peak's height everywhere: Terrarium, (r * 256 + g + b / 256) - 32768.
std::string demTile() {
    constexpr double stored = thimblePeakMeters + 32768.0;
    PremultipliedImage image({256, 256});
    for (size_t i = 0; i < image.bytes(); i += 4) {
        image.data[i] = static_cast<uint8_t>(stored / 256.0);
        image.data[i + 1] = static_cast<uint8_t>(std::fmod(stored, 256.0));
        image.data[i + 2] = 0;
        image.data[i + 3] = 255;
    }
    return encodePNG(image);
}

// Opaque imagery, in a checker so that the drape differs from place to place.
std::string imageryTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            uint8_t* px = image.data.get() + (y * 256 + x) * 4;
            const bool dark = ((x / 32) + (y / 32)) % 2 == 0;
            px[0] = dark ? 60 : 120;
            px[1] = dark ? 90 : 140;
            px[2] = dark ? 50 : 80;
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

struct Request {
    std::string source;
    CanonicalTileID tile{0, 0, 0};
};

// "https://t.test/<source>/<z>/<x>/<y>[.png]"
Request requestOf(const std::string& url) {
    const std::string path = url.substr(std::string("https://t.test/").size());
    const size_t slash = path.find('/');
    unsigned z = 0, x = 0, y = 0;
    std::sscanf(path.c_str() + slash + 1, "%u/%u/%u", &z, &x, &y);
    return {path.substr(0, slash), CanonicalTileID(static_cast<uint8_t>(z), x, y)};
}

using TilesBySource = std::map<std::string, std::set<CanonicalTileID>>;

// The app's view over the ground: terrain, every source asked for what it
// covers. The DEM is in before the overlays are shown, so the covers are the ones of a settled view.
class FarFieldView {
public:
    explicit FarFieldView(bool vectorContent)
        : fileSource(std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions())),
          dem(demTile()),
          imagery(imageryTile()),
          overlay(overlayTile()),
          frontend(phone, 1),
          map(frontend,
              observer,
              fileSource,
              MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this, vectorContent](const Resource& resource) {
            Response res;
            const Request request = requestOf(resource.url);
            if (shown || request.source == "dem") requested[request.source].insert(request.tile);
            if (request.source == "dem") {
                res.data = std::make_shared<std::string>(dem);
            } else if (request.source == "imagery") {
                res.data = std::make_shared<std::string>(imagery);
            } else if (vectorContent) {
                res.data = std::make_shared<std::string>(overlay);
            } else {
                res.noContent = true;
            }
            return res;
        };
        observer.didBecomeIdleCallback = [this] {
            idle = true;
        };
        map.setTerrainLoadMode(TerrainLoadMode::Balanced);
        map.setBounds(BoundOptions().withMaxPitch(85.0));
        map.getStyle().loadJSON(appStyle);
        // The ground the app puts the center on, which lifts the camera onto it
        map.setCenterClampedToGround(true);
    }

    void pump(std::chrono::milliseconds duration) {
        const auto end = std::chrono::steady_clock::now() + duration;
        while (std::chrono::steady_clock::now() < end) {
            loop.runOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    }

    PremultipliedImage read() {
        gfx::BackendScope guard{*frontend.getBackend()};
        return frontend.readStillImage();
    }

    // True once the map has said it is idle and the frame has stayed the same for a second; false if
    // a minute passes first.
    bool settle() {
        idle = false;
        const auto start = std::chrono::steady_clock::now();
        pump(std::chrono::milliseconds(500)); // a frame is drawn before the first read
        PremultipliedImage last = read();
        int unchanged = 0;
        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(60)) {
            if (unchanged >= 4 && idle) return true;
            pump(std::chrono::milliseconds(250));
            PremultipliedImage now = read();
            unchanged = std::memcmp(now.data.get(), last.data.get(), now.bytes()) == 0 ? unchanged + 1 : 0;
            last = std::move(now);
        }
        return false;
    }

    // Looks at the peak at `pitch`, with the DEM in first and the overlays shown after. What the
    // overlays ask for from then on is `requested`.
    bool open(double pitch) {
        look(pitch);
        if (!settle()) return false;
        for (const char* id : overlayLayers) {
            map.getStyle().getLayer(id)->setVisibility(style::VisibilityType::Visible);
        }
        shown = true;
        return settle();
    }

    // The camera starts on the ground, as the app's clamp leaves it. Left to the clamp, a view whose first
    // DEM tile is the eye's, not the center's, sat under the ground and covered other tiles.
    void look(double pitch) {
        map.jumpTo(CameraOptions()
                       .withCenter(thimblePeak)
                       .withCenterAltitude(thimblePeakMeters)
                       .withZoom(viewZoom)
                       .withPitch(pitch));
    }

    TilesBySource requested;

    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource;
    std::string dem;
    std::string imagery;
    std::string overlay;
    StubMapObserver observer;
    HeadlessFrontend frontend;
    MapAdapter map;
    bool idle = false;
    bool shown = false;
};

// Turns the leaving out of tiles that build none of a source's layers on or off, and back on.
class LeavingOut {
public:
    explicit LeavingOut(bool on) { RenderTerrain::setLeaveOutEmptyTilesForTesting(on); }
    ~LeavingOut() { RenderTerrain::setLeaveOutEmptyTilesForTesting(true); }
};

// Turns the ending of a source where the ground is far below its minzoom on or off, and back on.
class EndingBelowMinZoom {
public:
    explicit EndingBelowMinZoom(bool on) { RenderTerrain::setSourcesEndBelowMinZoomForTesting(on); }
    ~EndingBelowMinZoom() { RenderTerrain::setSourcesEndBelowMinZoomForTesting(true); }
};

TilesBySource tilesRequested(bool leaveOut, double pitch, bool endBelowMinZoom = false) {
    const LeavingOut use(leaveOut);
    const EndingBelowMinZoom end(endBelowMinZoom);
    FarFieldView view(false);
    EXPECT_TRUE(view.open(pitch));
    return view.requested;
}

size_t total(const TilesBySource& tiles) {
    size_t sum = 0;
    for (const auto& [source, ids] : tiles) {
        if (source != "dem") sum += ids.size();
    }
    return sum;
}

void print(const char* what, double pitch, const TilesBySource& tiles) {
    std::printf("%-12s pitch %2.0f:", what, pitch);
    for (const auto& [source, ids] : tiles) std::printf(" %s=%zu", source.c_str(), ids.size());
    std::printf("  (overlays %zu)\n", total(tiles));
}

// Sources none of whose layers draws over the first zoom levels of the source: the style gives
// each of them a layer minzoom above the source's own (peaks 10 over 7, land labels 12 over 11,
// streams 11 over 8). Every other source has a layer that draws at every zoom it is served at.
const std::set<std::string> thinned = {"backcountry_label_points", "public_land_label_points", "reference_streams"};

// What leaving the tiles out does to what each source asks for, against the same view without it
// in this process: every source asks for no tile it did not ask for before, the thinned ones for
// fewer, and the others, the DEM and the imagery for the same ones.
void expectLeftOut(const TilesBySource& today, const TilesBySource& now, const char* when) {
    for (const auto& [source, before] : today) {
        const auto it = now.find(source);
        const std::set<CanonicalTileID> after = it == now.end() ? std::set<CanonicalTileID>{} : it->second;
        EXPECT_TRUE(std::includes(before.begin(), before.end(), after.begin(), after.end()))
            << source << ", " << when << ": asks for a tile it did not ask for without leaving out";
        if (thinned.count(source)) {
            EXPECT_LT(after.size(), before.size()) << source << ", " << when;
        } else {
            EXPECT_EQ(before, after) << source << ", " << when;
        }
    }
    EXPECT_LT(total(now), total(today)) << when;
}

struct Difference {
    size_t pixels = 0;
    int largest = 0;
};

Difference compare(const PremultipliedImage& a, const PremultipliedImage& b) {
    Difference difference;
    for (size_t i = 0; i < a.bytes(); i += 4) {
        int largest = 0;
        for (size_t channel = 0; channel < 4; ++channel) {
            largest = std::max(largest, std::abs(static_cast<int>(a.data[i + channel]) - b.data[i + channel]));
        }
        if (largest > 0) ++difference.pixels;
        difference.largest = std::max(difference.largest, largest);
    }
    return difference;
}

// Pixels the map draws on: the ground, which the sky leaves clear
size_t drawn(const PremultipliedImage& image) {
    size_t count = 0;
    for (size_t i = 0; i < image.bytes(); i += 4) count += image.data[i + 3] > 0 ? 1 : 0;
    return count;
}

} // namespace

// At a steep pitch a source asks for the 192 tiles nearest the camera however far they are, and one with a
// deep minzoom covers the far field at it. A tile none of whose source's layers draws at its zoom builds
// nothing, so asking for it is for nothing.
TEST(TerrainUnseenTiles, ASourceDoesNotAskForTilesThatBuildNothingAtPitch77) {
    const TilesBySource today = tilesRequested(false, 77.0);
    const TilesBySource now = tilesRequested(true, 77.0);
    print("today", 77.0, today);
    print("left out", 77.0, now);

    expectLeftOut(today, now, "pitch 77");
}

TEST(TerrainUnseenTiles, ASourceDoesNotAskForTilesThatBuildNothingAtPitch85) {
    const TilesBySource today = tilesRequested(false, 85.0);
    const TilesBySource now = tilesRequested(true, 85.0);
    print("today", 85.0, today);
    print("left out", 85.0, now);

    expectLeftOut(today, now, "pitch 85");
}

// A source with a deep minzoom was covered at it over the far field, as far as the tile limit
// let it. It ends where the ground is more than two zooms below its minzoom.
TEST(TerrainUnseenTiles, ASourceEndsWhereTheGroundIsFarBelowItsMinZoom) {
    for (const double pitch : {77.0, 85.0}) {
        const TilesBySource whole = tilesRequested(true, pitch);
        const TilesBySource now = tilesRequested(true, pitch, true);
        print("whole cover", pitch, whole);
        print("ends", pitch, now);

        for (const auto& [source, ids] : now) {
            const auto it = whole.find(source);
            ASSERT_NE(it, whole.end()) << source << ", pitch " << pitch;
            EXPECT_LE(ids.size(), it->second.size()) << source << ", pitch " << pitch;
        }
        EXPECT_LT(total(now) * 2, total(whole)) << "pitch " << pitch;
        EXPECT_EQ(whole.at("imagery"), now.at("imagery")) << "pitch " << pitch;
        EXPECT_EQ(whole.at("dem"), now.at("dem")) << "pitch " << pitch;
    }
}

// What is left out must change no pixel: the same views with and without, after the view has been
// somewhere else first (a drape keeps what it baked).
TEST(TerrainUnseenTiles, LeavingThemOutLeavesThePictureAsItWas) {
    struct Look {
        double pitch;
        PremultipliedImage image;
    };
    std::vector<Look> without;
    std::vector<Look> with;
    for (const bool leaveOut : {false, true}) {
        const LeavingOut use(leaveOut);
        FarFieldView view(true);
        ASSERT_TRUE(view.open(45.0));
        for (const double pitch : {60.0, 77.0, 85.0, 77.0}) {
            view.look(pitch);
            ASSERT_TRUE(view.settle()) << "pitch " << pitch;
            (leaveOut ? with : without).push_back({pitch, view.read()});
        }
    }
    ASSERT_EQ(without.size(), with.size());
    // Two maps draw the same frame to within a few pixels at most
    constexpr size_t tolerance = 20;
    for (size_t i = 0; i < without.size(); ++i) {
        const Difference difference = compare(without[i].image, with[i].image);
        std::printf("pitch %2.0f: %zu of %zu pixels differ, by at most %d (of 255); %zu drawn\n",
                    without[i].pitch,
                    difference.pixels,
                    without[i].image.bytes() / 4,
                    difference.largest,
                    drawn(with[i].image));
        EXPECT_LE(difference.pixels, tolerance) << "at pitch " << without[i].pitch << " step " << i;
        EXPECT_GT(drawn(with[i].image), 10000u) << "the picture is not blank";
    }
}
