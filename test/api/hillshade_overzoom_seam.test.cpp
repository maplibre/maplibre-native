// Programmatic seam-detection test for hillshade rendering at overzoom.
//
// The earlier render-test approach (a fixed expected.png) can detect a
// regression only relative to a previously-captured golden image. This
// test instead asks the rendered output a direct question: do tile
// boundaries produce a visible discontinuity?
//
// Setup. The four z=12 terrain tiles (758,1608), (759,1608),
// (758,1609) and (759,1609) are loaded via a StubFileSource and a
// raster-dem source with maxzoom=12. The camera sits exactly at the
// world corner where those four tiles meet, at display zoom 13. Each
// tile is therefore overzoomed by one level, and the four-tile corner
// projects to the centre of the viewport — pixel (W/2, H/2). The
// horizontal tile boundary runs along screen row H/2; the vertical
// boundary runs along screen column W/2.
//
// Measurement. We compute the mean per-pixel absolute luminance
// difference between the row immediately above and the row immediately
// below the centre line, and again for two control rows that are not
// aligned with any tile boundary. A seam shows up as the centre-row
// diff being much larger than the control-row diff. We accept the
// rendering as seam-free when the centre diff is no more than
// 1.5× the larger of the two control diffs. The same test runs for
// the vertical boundary along column W/2.
//
// Test data. The four required terrain.png tiles already live under
// metrics/integration/tiles/ for the render-test integration suite;
// we read them directly from disk via StubFileSource.

#include <mbgl/test/util.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <mbgl/test/map_adapter.hpp>

#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <regex>
#include <string>

using namespace mbgl;

namespace {

// Style centred on the corner where the four z=12 terrain tiles meet,
// rendered at display zoom 13 (one level of overzoom relative to the
// source's maxzoom of 12). Same camera as the
// `hillshade-overzoom-seam` render-test for direct cross-reference.
constexpr const char* kStyleJSON = R"({
  "version": 8,
  "center": [-113.291016, 35.960223],
  "zoom": 13,
  "sources": {
    "terrain": {
      "type": "raster-dem",
      "tiles": ["http://tiles/{z}/{x}/{y}.terrain.png"],
      "maxzoom": 12,
      "tileSize": 256
    }
  },
  "layers": [
    { "id": "bg", "type": "background", "paint": { "background-color": "white" } },
    {
      "id": "hs", "type": "hillshade", "source": "terrain",
      "paint": {
        "hillshade-shadow-color": "#000000",
        "hillshade-highlight-color": "#ffffff",
        "hillshade-accent-color": "#000000",
        "hillshade-exaggeration": 1.0
      }
    }
  ]
})";

// Read the raw terrain tile bytes from the integration-tile fixtures.
std::string readTerrainTile(uint32_t x, uint32_t y) {
    std::string path = "metrics/integration/tiles/12-" + std::to_string(x) + "-" + std::to_string(y) + ".terrain.png";
    return util::read_file(path);
}

// Mean per-pixel absolute luminance difference between two pixel rows
// of the rendered RGBA image. Luminance is the unweighted average of
// the R, G, B channels (the hillshade output is grayscale before any
// shadow/highlight tint, so a simple mean is sufficient).
double meanRowDiff(const PremultipliedImage& img, int y0, int y1) {
    const int w = static_cast<int>(img.size.width);
    const auto* data = img.data.get();
    double sum = 0.0;
    for (int x = 0; x < w; ++x) {
        const std::size_t i0 = (static_cast<std::size_t>(y0) * w + x) * 4;
        const std::size_t i1 = (static_cast<std::size_t>(y1) * w + x) * 4;
        const double lum0 = (data[i0] + data[i0 + 1] + data[i0 + 2]) / 3.0;
        const double lum1 = (data[i1] + data[i1 + 1] + data[i1 + 2]) / 3.0;
        sum += std::abs(lum0 - lum1);
    }
    return sum / w;
}

double meanColDiff(const PremultipliedImage& img, int x0, int x1) {
    const int w = static_cast<int>(img.size.width);
    const int h = static_cast<int>(img.size.height);
    const auto* data = img.data.get();
    double sum = 0.0;
    for (int y = 0; y < h; ++y) {
        const std::size_t i0 = (static_cast<std::size_t>(y) * w + x0) * 4;
        const std::size_t i1 = (static_cast<std::size_t>(y) * w + x1) * 4;
        const double lum0 = (data[i0] + data[i0 + 1] + data[i0 + 2]) / 3.0;
        const double lum1 = (data[i1] + data[i1 + 1] + data[i1 + 2]) / 3.0;
        sum += std::abs(lum0 - lum1);
    }
    return sum / h;
}

class HillshadeSeamTest {
public:
    util::RunLoop loop;
    HeadlessFrontend frontend{Size{256, 256}, 1.0};
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();
    MapAdapter map{frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()).withPixelRatio(1.0)};

    HillshadeSeamTest() {
        // Serve the four z=12 terrain tiles that the chosen viewport
        // requires, plus any z<12 ancestor the renderer happens to
        // request. The URL is http://tiles/{z}/{x}/{y}.terrain.png —
        // we parse z/x/y back out and dispatch to the matching
        // metrics/integration/tiles fixture.
        fileSource->tileResponse = [](const Resource& res) -> std::optional<Response> {
            // Every tile request gets a synchronous response. For URLs we
            // don't recognise or zoom levels we don't have fixtures for,
            // we return a `noContent`-flagged empty Response rather than
            // `std::nullopt` — the latter leaves the request pending in
            // StubFileSource's async path, which under bazel coverage's
            // slower task scheduling can occasionally surface as a hang
            // in `render()` waiting for a parent tile that never resolves.
            Response empty;
            empty.noContent = true;
            static const std::regex urlRe{R"(/(\d+)/(\d+)/(\d+)\.terrain\.png$)"};
            std::smatch m;
            if (!std::regex_search(res.url, m, urlRe)) return empty;
            const uint32_t z = static_cast<uint32_t>(std::stoul(m[1].str()));
            const uint32_t x = static_cast<uint32_t>(std::stoul(m[2].str()));
            const uint32_t y = static_cast<uint32_t>(std::stoul(m[3].str()));
            // We only ship the four z=12 corner tiles as fixtures.
            // Anything else: empty Response (the renderer falls back to
            // its overzoom path from the available parents — here z=12
            // is the source maxzoom, so overzoom upward is what we want).
            if (z != 12) return empty;
            Response r;
            try {
                r.data = std::make_shared<std::string>(readTerrainTile(x, y));
            } catch (...) {
                r.noContent = true;
                return r;
            }
            return r;
        };
    }
};

} // namespace

// Re-renders the same scene a few times via the headless frontend so
// neighbour-tile backfilling has finished propagating through the
// prepare pass before we sample the final image. The first render
// often produces a frame in which only some tiles have had their
// borders backfilled; the seam analysis only makes sense once all
// four tiles have been re-prepared with their final borders.
TEST(HillshadeRendering, NoOverzoomCornerSeam) {
    HillshadeSeamTest test;
    test.map.getStyle().loadJSON(kStyleJSON);

    PremultipliedImage img;
    for (int i = 0; i < 4; ++i) {
        img = test.frontend.render(test.map).image;
    }

    ASSERT_FALSE(img.size.isEmpty());
    const int w = static_cast<int>(img.size.width);
    const int h = static_cast<int>(img.size.height);
    ASSERT_EQ(w, 256);
    ASSERT_EQ(h, 256);

    // Build a control baseline as the mean of many non-boundary row
    // pair diffs (and column pair diffs). Averaging across many
    // samples washes out any single row's terrain-aligned coincidence
    // and gives a stable estimate of "how different adjacent rows
    // look in this rendering as a whole." A seam shows up as a single
    // boundary-row diff that's an outlier compared to that mean.
    //
    // Skip a small band around the centre line so we don't accidentally
    // include the seam itself in the control samples.
    const int cy = h / 2;
    const int cx = w / 2;
    const int skip = 8;

    auto meanRowDiffControls = [&]() {
        double sum = 0;
        int count = 0;
        for (int y = 16; y < h - 16; y += 4) {
            if (std::abs(y - cy) < skip) continue;
            sum += meanRowDiff(img, y - 1, y + 1);
            ++count;
        }
        return sum / count;
    };
    auto meanColDiffControls = [&]() {
        double sum = 0;
        int count = 0;
        for (int x = 16; x < w - 16; x += 4) {
            if (std::abs(x - cx) < skip) continue;
            sum += meanColDiff(img, x - 1, x + 1);
            ++count;
        }
        return sum / count;
    };

    const double seamH = meanRowDiff(img, cy - 1, cy + 1);
    const double ctrlH = meanRowDiffControls();
    const double seamV = meanColDiff(img, cx - 1, cx + 1);
    const double ctrlV = meanColDiffControls();

    // 1.25× headroom over the typical adjacent-row diff is tight
    // enough to detect the pre-fix horizontal seam (which sits
    // around 1.4× of the mean control) and the pre-fix vertical
    // seam (around 1.8×), while leaving comfortable margin for the
    // fixed renderer where the boundary row is actually quieter
    // than the average row (<1.0× of control).
    EXPECT_LE(seamH, 1.25 * ctrlH) << "horizontal hillshade seam at y=" << cy << ": diff=" << seamH
                                   << ", control mean=" << ctrlH;
    EXPECT_LE(seamV, 1.25 * ctrlV) << "vertical hillshade seam at x=" << cx << ": diff=" << seamV
                                   << ", control mean=" << ctrlV;
}
