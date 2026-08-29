#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/fixture_log_observer.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/map/map_options.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/util/exception.hpp>
#include <mln/util/run_loop.hpp>

#include <future>

using namespace mln;

TEST(API, RenderWithoutCallback) {
    FixtureLog log;

    util::RunLoop loop;

    HeadlessFrontend frontend{1};

    auto map = std::make_unique<MapAdapter>(frontend,
                                            MapObserver::nullObserver(),
                                            std::make_shared<StubFileSource>(),
                                            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map->renderStill(nullptr);

    // Force Map thread to join.
    map.reset();

    const FixtureLogObserver::LogMessage logMessage{
        EventSeverity::Error,
        Event::General,
        int64_t(-1),
        "StillImageCallback not set",
    };

    EXPECT_EQ(log.count(logMessage), 1u);
}
