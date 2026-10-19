#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/style/layers/symbol_layer.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/image.hpp>
#include <mln/style/style.hpp>
#include <mln/util/exception.hpp>
#include <mln/util/geometry.hpp>
#include <mln/util/geojson.hpp>
#include <mln/util/io.hpp>
#include <mln/util/run_loop.hpp>

using namespace mln;
using namespace mln::style;

TEST(API, RecycleMapUpdateImages) {
    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    auto map = std::make_unique<MapAdapter>(
        frontend,
        MapObserver::nullObserver(),
        std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions()),
        MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));

    EXPECT_TRUE(map);

    auto loadStyle = [&](auto markerName, auto markerPath) {
        auto source = std::make_unique<GeoJSONSource>("geometry");
        source->setGeoJSON(Geometry<double>{Point<double>{0, 0}});

        auto layer = std::make_unique<SymbolLayer>("geometry", "geometry");
        layer->setIconImage({markerName});

        map->getStyle().loadJSON(util::read_file("test/fixtures/api/empty.json"));
        map->getStyle().addSource(std::move(source));
        map->getStyle().addLayer(std::move(layer));
        map->getStyle().addImage(
            std::make_unique<style::Image>(markerName, decodeImage(util::read_file(markerPath)), 1.0f));
    };

    // default marker

    loadStyle("default_marker", "test/fixtures/sprites/default_marker.png");
    test::checkImage("test/fixtures/recycle_map/default_marker", frontend.render(*map).image, 0.0006, 0.1);

    // flipped marker

    loadStyle("flipped_marker", "test/fixtures/sprites/flipped_marker.png");
    test::checkImage("test/fixtures/recycle_map/flipped_marker", frontend.render(*map).image, 0.0006, 0.1);
}
