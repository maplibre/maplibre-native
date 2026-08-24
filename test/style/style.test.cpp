#include <chrono>
#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/fixture_log_observer.hpp>

#include <mln/style/style_impl.hpp>
#include <mln/style/source_impl.hpp>
#include <mln/style/sources/vector_source.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/util/io.hpp>
#include <mln/util/rapidjson.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/client_options.hpp>

#include <memory>

#ifdef WIN32
#include <Windows.h>
#endif

using namespace mln;
using namespace mln::style;

TEST(Style, Properties) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.loadJSON(R"STYLE({"name": "Test"})STYLE");
    ASSERT_EQ("Test", style.getName());

    style.loadJSON(R"STYLE({"center": [10, 20]})STYLE");
    ASSERT_EQ("", style.getName());
    ASSERT_EQ((LatLng{20, 10}), *style.getDefaultCamera().center);

    style.loadJSON(R"STYLE({"centerAltitude": 999})STYLE");
    ASSERT_EQ("", style.getName());
    ASSERT_EQ(999, *style.getDefaultCamera().centerAltitude);

    style.loadJSON(R"STYLE({"bearing": 24})STYLE");
    ASSERT_EQ("", style.getName());
    ASSERT_EQ(LatLng{}, *style.getDefaultCamera().center);
    ASSERT_EQ(24, *style.getDefaultCamera().bearing);

    style.loadJSON(R"STYLE({"zoom": 13.3})STYLE");
    ASSERT_EQ("", style.getName());
    ASSERT_EQ(13.3, *style.getDefaultCamera().zoom);

    style.loadJSON(R"STYLE({"pitch": 60})STYLE");
    ASSERT_EQ("", style.getName());
    ASSERT_EQ(60, *style.getDefaultCamera().pitch);

    style.loadJSON(R"STYLE({"roll": 99})STYLE");
    ASSERT_EQ("", style.getName());
    ASSERT_EQ(99, *style.getDefaultCamera().roll);

    style.loadJSON(R"STYLE({})STYLE");
    ASSERT_EQ(Milliseconds(300), *style.getTransitionOptions().duration);
    ASSERT_EQ(std::optional<Duration>{}, style.getTransitionOptions().delay);

    style.loadJSON(R"STYLE({"transition": { "duration": 500, "delay": 50 }})STYLE");
    ASSERT_EQ(Milliseconds(500), *style.getTransitionOptions().duration);
    ASSERT_EQ(Milliseconds(50), *style.getTransitionOptions().delay);

    style.loadJSON(R"STYLE({"name": 23, "center": {}, "bearing": "north", "zoom": null, "pitch": "wide"})STYLE");
    ASSERT_EQ("", style.getName());
    ASSERT_EQ(LatLng{}, *style.getDefaultCamera().center);
    ASSERT_EQ(0, *style.getDefaultCamera().zoom);
    ASSERT_EQ(0, *style.getDefaultCamera().bearing);
    ASSERT_EQ(0, *style.getDefaultCamera().pitch);
}

TEST(Style, GlobalState) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    // Defaults from the root "state" property are used as the initial global state.
    style.loadJSON(R"STYLE({
        "version": 8,
        "state": {
            "showLabels": {"default": true},
            "categories": {"default": ["restaurant", "hotel"]},
            "noDefault": {}
        },
        "sources": {},
        "layers": []
    })STYLE");

    auto state = style.getGlobalState();
    ASSERT_EQ(3u, state.size());
    EXPECT_EQ(Value(true), state.at("showLabels"));
    EXPECT_EQ(Value(mapbox::base::ValueArray({Value("restaurant"), Value("hotel")})), state.at("categories"));
    EXPECT_EQ(Value(NullValue()), state.at("noDefault"));

    // Setting a property updates the state.
    style.setGlobalStateProperty("showLabels", false);
    EXPECT_EQ(Value(false), style.getGlobalState().at("showLabels"));

    // Properties that are not defined in the style can also be set.
    style.setGlobalStateProperty("custom", 42.0);
    EXPECT_EQ(Value(42.0), style.getGlobalState().at("custom"));

    // A null value resets the property to its default.
    style.setGlobalStateProperty("showLabels", NullValue());
    EXPECT_EQ(Value(true), style.getGlobalState().at("showLabels"));

    // A null value on a property without default resets to null.
    style.setGlobalStateProperty("custom", NullValue());
    EXPECT_EQ(Value(NullValue()), style.getGlobalState().at("custom"));

    // Loading a new style resets the global state.
    style.loadJSON(R"STYLE({"version": 8, "sources": {}, "layers": []})STYLE");
    EXPECT_TRUE(style.getGlobalState().empty());

    // Setting an absent property to null is a no-op: a missing property
    // already evaluates to null.
    style.setGlobalStateProperty("neverSet", NullValue());
    EXPECT_TRUE(style.getGlobalState().empty());
}

TEST(Style, GlobalStateNumericEquality) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.loadJSON(R"STYLE({
        "version": 8,
        "state": {"minSpeed": {"default": 50}},
        "sources": {},
        "layers": []
    })STYLE");

    // Setting the same numeric value with a different arithmetic type
    // (integer default vs. runtime double) must be treated as unchanged.
    const auto before = style.getGlobalStateShared();
    style.setGlobalStateProperty("minSpeed", 50.0);
    EXPECT_EQ(before, style.getGlobalStateShared());

    style.setGlobalStateProperty("minSpeed", 51.0);
    EXPECT_NE(before, style.getGlobalStateShared());
}

TEST(Style, GlobalStateVisibility) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.loadJSON(R"STYLE({
        "version": 8,
        "state": {"showBackground": {"default": true}},
        "sources": {},
        "layers": [{
            "id": "background",
            "type": "background",
            "layout": {
                "visibility": ["case", ["to-boolean", ["global-state", "showBackground"]], "visible", "none"]
            }
        }]
    })STYLE");

    Layer* layer = style.getLayer("background");
    ASSERT_TRUE(layer);
    // The default from the root "state" property applies.
    EXPECT_EQ(VisibilityType::Visible, layer->getVisibility());

    style.setGlobalStateProperty("showBackground", false);
    EXPECT_EQ(VisibilityType::None, layer->getVisibility());

    style.setGlobalStateProperty("showBackground", NullValue());
    EXPECT_EQ(VisibilityType::Visible, layer->getVisibility());

    // Setting a constant visibility replaces the expression: further state
    // changes no longer apply.
    layer->setVisibility(VisibilityType::None);
    style.setGlobalStateProperty("showBackground", true);
    EXPECT_EQ(VisibilityType::None, layer->getVisibility());
}

TEST(Style, VisibilityExpressionSetAtRuntime) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.loadJSON(R"STYLE({
        "version": 8,
        "state": {"showBackground": {"default": false}},
        "sources": {},
        "layers": [{"id": "background", "type": "background"}]
    })STYLE");

    Layer* layer = style.getLayer("background");
    ASSERT_TRUE(layer);
    EXPECT_EQ(VisibilityType::Visible, layer->getVisibility());

    // A visibility expression set after the style has loaded must be
    // evaluated against the current global state right away.
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
    document.Parse<0>(R"(["case", ["to-boolean", ["global-state", "showBackground"]], "visible", "none"])");
    ASSERT_FALSE(document.HasParseError());
    const JSValue* json = &document;
    auto error = layer->setProperty("visibility", conversion::Convertible(json));
    EXPECT_FALSE(error);
    EXPECT_EQ(VisibilityType::None, layer->getVisibility());

    // Runtime global state (not only style defaults) applies as well.
    style.setGlobalStateProperty("showBackground", true);
    ASSERT_TRUE(layer);
    EXPECT_EQ(VisibilityType::Visible, layer->getVisibility());

    document.Parse<0>(R"(["case", ["to-boolean", ["global-state", "showBackground"]], "none", "visible"])");
    ASSERT_FALSE(document.HasParseError());
    error = layer->setProperty("visibility", conversion::Convertible(json));
    EXPECT_FALSE(error);
    EXPECT_EQ(VisibilityType::None, layer->getVisibility());
}

TEST(Style, VisibilityExpressionRejectsOtherDependencies) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.loadJSON(R"STYLE({
        "version": 8,
        "sources": {},
        "layers": [{"id": "background", "type": "background"}]
    })STYLE");

    Layer* layer = style.getLayer("background");
    ASSERT_TRUE(layer);

    // Visibility expressions may only depend on the global state.
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
    document.Parse<0>(R"(["case", [">", ["zoom"], 10], "visible", "none"])");
    ASSERT_FALSE(document.HasParseError());
    const JSValue* json = &document;
    auto error = layer->setProperty("visibility", conversion::Convertible(json));
    EXPECT_TRUE(error);
    EXPECT_EQ(VisibilityType::Visible, layer->getVisibility());
}

TEST(Style, DuplicateSource) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.loadJSON(util::read_file("test/fixtures/resources/style-unused-sources.json"));

    style.addSource(std::make_unique<VectorSource>("sourceId", "mptiler://tiles/contours"));

    try {
        style.addSource(std::make_unique<VectorSource>("sourceId", "mptiler://tiles/contours"));
        FAIL() << "Should not have been allowed to add a duplicate source id";
    } catch (const std::runtime_error&) {
        // Expected
    }
}

TEST(Style, RemoveSourceInUse) {
    util::RunLoop loop;

    FixtureLog log;

    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.loadJSON(util::read_file("test/fixtures/resources/style-unused-sources.json"));
    style.addSource(std::make_unique<VectorSource>("sourceId", "mptiler://tiles/contours"));
    style.addLayer(std::make_unique<LineLayer>("layerId", "sourceId"));

    // Should not remove the source
    auto removed = style.removeSource("sourceId");
    ASSERT_EQ(nullptr, removed);
    ASSERT_NE(nullptr, style.getSource("sourceId"));

    const FixtureLogObserver::LogMessage logMessage{
        EventSeverity::Warning,
        Event::General,
        int64_t(-1),
        "Source 'sourceId' is in use, cannot remove",
    };

#if defined(WIN32)
    Sleep(1000);
#endif

    EXPECT_EQ(log.count(logMessage), 1u);
}

TEST(Style, LoadJSONCancelsPendingLoadURL) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<::StubFileSource>(
        ResourceOptions::Default(), ClientOptions(), StubFileSource::ResponseType::Manual);
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    // Start loading a URL (this will be pending)
    auto url = "http://some-url";
    fileSource->styleResponse = [](const Resource&) {
        Response result;
        result.data = std::make_shared<std::string>(util::read_file("test/fixtures/resources/style_vector.json"));
        return result;
    };
    style.loadURL(url);

    // Before the URL request completes, load JSON directly
    const std::string json = R"STYLE({
        "version": 8,
        "name": "Test Style",
        "sources": {},
        "layers": []
    })STYLE";
    style.loadJSON(json);

    // The style should now be loaded with our JSON content
    ASSERT_EQ("Test Style", style.getName());
    ASSERT_EQ("", style.getURL());
    ASSERT_TRUE(style.getJSON().find("Test Style") != std::string::npos);

    fileSource->respondToAll();

    // The style should still show our JSON content, not the URL content
    ASSERT_EQ("Test Style", style.getName());
    ASSERT_NE("Streets", style.getName());
}

TEST(Style, SourceImplsOrder) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};

    style.addSource(std::make_unique<VectorSource>("c", "mptiler://tiles/contours"));
    style.addSource(std::make_unique<VectorSource>("b", "mptiler://tiles/contours"));
    style.addSource(std::make_unique<VectorSource>("a", "mptiler://tiles/contours"));

    auto sources = style.getSources();
    ASSERT_EQ(3u, sources.size());
    EXPECT_EQ("c", sources[0]->getID());
    EXPECT_EQ("b", sources[1]->getID());
    EXPECT_EQ("a", sources[2]->getID());

    const auto& sourceImpls = *style.getSourceImpls();
    ASSERT_EQ(3u, sourceImpls.size());
    EXPECT_EQ("a", sourceImpls[0]->id);
    EXPECT_EQ("b", sourceImpls[1]->id);
    EXPECT_EQ("c", sourceImpls[2]->id);
}

TEST(Style, AddRemoveImage) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};
    style.addImage(std::make_unique<style::Image>("one", PremultipliedImage({16, 16}), 2.0f));
    style.addImage(std::make_unique<style::Image>("two", PremultipliedImage({16, 16}), 2.0f));
    style.addImage(std::make_unique<style::Image>("three", PremultipliedImage({16, 16}), 2.0f));

    style.removeImage("one");
    style.removeImage("two");

    EXPECT_TRUE(!!style.getImage("three"));
    EXPECT_FALSE(!!style.getImage("two"));
    EXPECT_FALSE(!!style.getImage("four"));
}

TEST(Style, AddRemoveRemoveImage) {
    // regression test for https://github.com/mapbox/mapbox-gl-native/pull/16391
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};
    style.addImage(std::make_unique<style::Image>("one", PremultipliedImage({16, 16}), 2.0f));
    style.addImage(std::make_unique<style::Image>("two", PremultipliedImage({16, 16}), 2.0f));
    style.addImage(std::make_unique<style::Image>("three", PremultipliedImage({16, 16}), 2.0f));

    style.removeImage("one");
    style.removeImage("two");
    style.removeImage("two");

    EXPECT_TRUE(!!style.getImage("three"));
    EXPECT_FALSE(!!style.getImage("two"));
    EXPECT_FALSE(!!style.getImage("four"));
}
