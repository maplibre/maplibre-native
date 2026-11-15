#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/gfx/rendering_stats.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using namespace mln;

namespace {

// Flat ground under one black line, a single pixel wide, that runs the middle of
// a white drape. Seen from a steep pitch the line recedes straight up the middle of the view.
// The style is built in parts: the lines are too many to fit in a string literal that MSVC accepts.
std::string lineStyle() {
    // 700 lines, 0.0003 degrees apart (33 m), as one MultiLineString
    std::string lines;
    for (int i = 0; i < 700; ++i) {
        const int latitude = 459000 + 3 * i; // in ten-thousandths of a degree
        const int degrees = latitude / 10000;
        const int fraction = latitude % 10000;
        char line[64];
        std::snprintf(line,
                      sizeof(line),
                      "%s[[9.3, %d.%04d], [10.7, %d.%04d]]",
                      i ? "," : "",
                      degrees,
                      fraction,
                      degrees,
                      fraction);
        lines += line;
    }

    return R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    },
    "line": {
      "type": "geojson",
      "data": {
        "type": "Feature",
        "properties": {},
        "geometry": {"type": "MultiLineString", "coordinates": [)STYLE" +
           lines + R"STYLE(]}
      }
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "ground", "type": "background", "paint": {"background-color": "rgb(255, 255, 255)"}},
    {"id": "line", "type": "line", "source": "line",
     "paint": {"line-color": "rgb(0, 0, 0)", "line-width": 1}}
  ]
})STYLE";
}

std::string flatDEMTile() {
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

constexpr uint32_t viewSize = 512;

struct Rendered {
    PremultipliedImage image;
    gfx::RenderingStats stats;      // after the frames that bake the drapes
    gfx::RenderingStats statsAfter; // after extra frames of the same unchanged view
};

Rendered renderLine(double pitch) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string dem = flatDEMTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(dem);
        return res;
    };
    HeadlessFrontend frontend{{viewSize, viewSize}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(lineStyle());
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(13.0).withBearing(0.0).withPitch(pitch));

    Rendered rendered;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        auto result = frontend.render(map);
        rendered.image = std::move(result.image);
        rendered.stats = result.stats;
    }
    // The view does not change: nothing is re-rendered, so no chain may be rebuilt
    for (int i = 0; i < 3; ++i) {
        loop.runOnce();
        rendered.statsAfter = frontend.render(map).stats;
    }
    return rendered;
}

// The mean darkness of each row of the view, 0 to 255 below the white drape, over the middle of the
// row; the rows run from the top of the view down.
std::vector<double> rowDarkness(const PremultipliedImage& image) {
    std::vector<double> rows;
    const uint32_t from = image.size.width / 8;
    const uint32_t to = image.size.width - from;
    for (uint32_t y = 0; y < image.size.height; ++y) {
        double sum = 0;
        for (uint32_t x = from; x < to; ++x) {
            sum += 255 - image.data.get()[(y * image.size.width + x) * 4 + 1];
        }
        rows.push_back(sum / (to - from));
    }
    return rows;
}

} // namespace

// Only the OpenGL and Metal drape targets build a mip chain.
// Lines one pixel wide every 33 m, draped on flat ground seen at 84 degrees: far off, a row of the
// view spans many lines and a line is much thinner than a row. Sampled from the target's base level
// alone, the terrain pass steps over most of them: rows alternate between catching a line and seeing
// bare ground, which shows as dashes and moire. Sampled from a mip chain every row averages the lines
// it spans, so the darkness changes smoothly from row to row.
TEST(TerrainDrapeMipmaps, ThinLinesDoNotAliasFarAcrossSteepGround) {
#if !(MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_METAL)
    GTEST_SKIP() << "drape targets have no mip chain on this backend";
#endif
    const std::vector<double> rows = rowDarkness(renderLine(84.0).image);

    // The far ground: the 64 rows below the horizon, past its first few, where a row of the view
    // spans many lines
    size_t top = 0;
    while (top < rows.size() && rows[top] == 0) ++top;
    const size_t first = top + 8;
    const size_t last = first + 64;
    ASSERT_LT(last, rows.size()) << "no far ground in the view";

    // Row to row change, relative to the darkness there: smooth if small
    double jitter = 0;
    std::string profile;
    for (size_t i = first + 1; i < last; ++i) {
        jitter += std::abs(rows[i] - rows[i - 1]) / std::max(rows[i], 1.0);
        profile += std::to_string(static_cast<int>(rows[i])) + " ";
    }
    jitter /= static_cast<double>(last - first - 1);
    EXPECT_LT(jitter, 0.25) << "row darkness over the far ground, row by row: " << profile;
}

// The chain is rebuilt from a drape target's base level each time the target is rendered, and not
// otherwise: a view that stays put re-renders no target, so it rebuilds no chain.
TEST(TerrainDrapeMipmaps, ChainsAreRebuiltOnlyWhenATargetIsRendered) {
#if !(MLN_RENDER_BACKEND_OPENGL || MLN_RENDER_BACKEND_METAL)
    GTEST_SKIP() << "drape targets have no mip chain on this backend";
#endif
    const Rendered rendered = renderLine(84.0);
    EXPECT_GT(rendered.stats.numDrapeTargetsRendered, 0);
    EXPECT_GT(rendered.stats.numMipmapChainsBuilt, 0);
    EXPECT_EQ(rendered.statsAfter.numMipmapChainsBuilt, rendered.stats.numMipmapChainsBuilt)
        << "chains were rebuilt on frames that rendered no target";
}
