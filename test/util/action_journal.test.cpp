#include <mbgl/util/action_journal.hpp>
#include <mbgl/util/action_journal_impl.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/rapidjson.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/map/map_impl.hpp>

#include <mbgl/test/util.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <mbgl/test/stub_map_observer.hpp>

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>

#include <regex>

using namespace mbgl;
using namespace mbgl::util;
using namespace ::testing;

class ActionJournalTest {
public:
    class MapAdapter : public Map {
    public:
        MapAdapter(RendererFrontend& frontend,
                   MapObserver& observer,
                   std::shared_ptr<FileSource> fileSource,
                   const MapOptions& options,
                   const util::ActionJournalOptions& actionJournalOptions)
            : Map(std::make_unique<Map::Impl>(frontend, observer, std::move(fileSource), options),
                  actionJournalOptions) {}

        Map::Impl& getImpl() { return *impl; }
    };

    ActionJournalOptions options;
    StubMapObserver observer;
    std::shared_ptr<StubFileSource> fileSource;
    std::unique_ptr<HeadlessFrontend> frontend;
    std::unique_ptr<MapAdapter> map;

    util::RunLoop runLoop;

    ActionJournalTest(const ActionJournalOptions& options_, MapMode mode = MapMode::Static)
        : options(options_) {
        clear();
        createMap(mode);
    }

    ~ActionJournalTest() {
        map.reset();
        frontend.reset();
        fileSource.reset();
        clear();
    }

    void createMap(MapMode mode = MapMode::Static) {
        fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                      ClientOptions().withName("ActionJournalTest").withVersion("1.0"));
        frontend = std::make_unique<HeadlessFrontend>(1.0f);
        map = std::make_unique<MapAdapter>(
            *frontend, observer, fileSource, MapOptions().withMapMode(mode).withSize(frontend->getSize()), options);
    }

    std::filesystem::path getDirectoryPath() {
        return std::filesystem::canonical(options.path()) / ActionJournal::Impl::getDirectoryName();
    }

    void clear() { std::filesystem::remove_all(getDirectoryPath()); }
};

TEST(ActionJournal, Create) {
    ActionJournalTest test(ActionJournalOptions().enable().withPath("."));

    test.map->getActionJournal()->impl->flush();

    // map creation logs `onMapCreate`
    EXPECT_EQ(test.map->getActionJournal()->getLogDirectory(), test.getDirectoryPath().generic_string());
    EXPECT_TRUE(std::filesystem::exists(test.map->getActionJournal()->getLogDirectory()));
    EXPECT_EQ(test.map->getActionJournal()->getLogFiles().size(), 1u);

    const auto log = test.map->getActionJournal()->getLog();
    EXPECT_EQ(log.size(), 1u);
    EXPECT_THAT(log.front(), HasSubstr("onMapCreate"));
}

TEST(ActionJournal, Clear) {
    ActionJournalTest test(ActionJournalOptions().enable().withPath("."));

    test.map->getActionJournal()->impl->flush();
    test.map->getActionJournal()->clearLog();

    EXPECT_TRUE(std::filesystem::exists(test.map->getActionJournal()->getLogDirectory()));
    EXPECT_EQ(test.map->getActionJournal()->getLogFiles().size(), 1u);
    EXPECT_THAT(test.map->getActionJournal()->getLog(), IsEmpty());
}

TEST(ActionJournal, ReadPreviousSession) {
    // generate `onMapCreate` event
    ActionJournalTest test(ActionJournalOptions().enable().withPath("."));

    // generate `onMapDestroy` event
    test.map.reset();

    // generate `onMapCreate` event
    test.createMap();

    test.map->getActionJournal()->impl->flush();

    const auto log = test.map->getActionJournal()->getLog();
    EXPECT_EQ(log.size(), 3u);
    EXPECT_THAT(log, ElementsAre(HasSubstr("onMapCreate"), HasSubstr("onMapDestroy"), HasSubstr("onMapCreate")));
}

TEST(ActionJournal, RollFiles) {
    // limit the number of files and the size of the file
    ActionJournalTest test(ActionJournalOptions().enable().withPath(".").withLogFileSize(1).withLogFileCount(2));

    // generate multiple log events by loading a style and rendering
    test.map->getStyle().loadJSON(util::read_file("test/fixtures/api/empty.json"));

    test.runLoop.runOnce();
    test.frontend->render(*test.map);

    test.map->getActionJournal()->impl->flush();

    // check if the files roll and respect the limits
    const auto files = test.map->getActionJournal()->getLogFiles();
    EXPECT_TRUE(std::filesystem::exists(test.map->getActionJournal()->getLogDirectory()));
    EXPECT_EQ(files.size(), 2u);
    EXPECT_EQ(test.map->getActionJournal()->getLog().size(), 2u);

    // the newest event should be `onMapDestroy`
    test.map.reset();
    EXPECT_THAT(util::read_file(files.back()), HasSubstr("onMapDestroy"));
}

namespace {

template <typename EventMethod, typename... Args>
void validateEventList(ActionJournalTest& test,
                       std::vector<std::pair<const char*, std::function<void(const mbgl::JSValue&)>>>&& generatedEvents,
                       EventMethod&& eventGenerator,
                       Args&&... args) {
    test.map->getActionJournal()->clearLog();
    (test.map->getImpl().*eventGenerator)(std::forward<Args>(args)...);
    test.map->getActionJournal()->impl->flush();

    const auto& log = test.map->getActionJournal()->getLog();
    EXPECT_EQ(log.size(), generatedEvents.size());

    auto logIt = log.begin();
    for (const auto& generatedEvent : generatedEvents) {
        mbgl::JSDocument document;
        document.Parse<rapidjson::kParseDefaultFlags>(*logIt++);

        EXPECT_FALSE(document.HasParseError());
        EXPECT_TRUE(document.IsObject());

        const auto& json = document.GetObject();

        EXPECT_TRUE(json.HasMember("name"));
        EXPECT_TRUE(json["name"].IsString());
        EXPECT_STREQ(json["name"].GetString(), generatedEvent.first);

        EXPECT_TRUE(json.HasMember("time"));
        EXPECT_TRUE(json["time"].IsString());

        EXPECT_TRUE(json.HasMember("clientName"));
        EXPECT_TRUE(json["clientName"].IsString());
        EXPECT_STREQ(json["clientName"].GetString(), test.fileSource->getClientOptions().name().c_str());

        EXPECT_TRUE(json.HasMember("clientVersion"));
        EXPECT_TRUE(json["clientVersion"].IsString());
        EXPECT_STREQ(json["clientVersion"].GetString(), test.fileSource->getClientOptions().version().c_str());

        // `MatchesRegex(R"(\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\d\.\d\d\dZ)")` fails on linux gcc
        // https://github.com/google/googletest/issues/3084
        // `MatchesRegex(R"([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]\.[0-9][0-9][0-9]Z)"))`
        // fails on windows CI
        // using std::regex for now
        std::cmatch match;
        const std::regex timeRegex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}.\d{3}Z)");
        EXPECT_TRUE(std::regex_match(json["time"].GetString(), match, timeRegex));

        if (generatedEvent.second) {
            EXPECT_TRUE(json.HasMember("event"));
            EXPECT_TRUE(json["event"].IsObject());

            generatedEvent.second(json["event"].GetObject());
        }
    }
}

template <typename EventMethod, typename... Args>
void validateEvent(ActionJournalTest& test,
                   const char* generatedEvent,
                   std::function<void(const mbgl::JSValue&)>&& checkOutput,
                   EventMethod&& eventGenerator,
                   Args&&... args) {
    validateEventList(test, {{generatedEvent, checkOutput}}, eventGenerator, std::forward<Args>(args)...);
}

} // namespace

TEST(ActionJournal, ValidateEvents) {
    ActionJournalTest test(
        ActionJournalOptions().enable().withPath(".").withLogFileCount(2).withLogFileSize(1024 * 1024 * 5),
        MapMode::Continuous);

    test.map->getStyle().loadJSON(util::read_file("test/fixtures/api/empty.json"));
    test.map->getActionJournal()->impl->flush();

    const auto testShaderRegistry = std::make_unique<gfx::ShaderRegistry>();
    const auto testSource = std::make_unique<style::GeoJSONSource>("testSource");
    const auto testSprite = style::Sprite("testID", "testURL");
    const auto testExceptionMessage = "test error";
    const auto testException = std::make_exception_ptr(std::runtime_error(testExceptionMessage));
    const auto testFontStack = FontStack{"Roboto Condensed Italic", "Noto Sans Italic"};
    const auto testGlyphRange = GlyphRange(0, 255);

    validateEvent(
        test,
        "onCameraWillChange",
        [&](const mbgl::JSValue& json) {
            EXPECT_TRUE(json.HasMember("cameraMode"));
            EXPECT_TRUE(json["cameraMode"].IsInt());
            EXPECT_EQ(json["cameraMode"].GetInt(), static_cast<int>(MapObserver::CameraChangeMode::Immediate));
        },
        &TransformObserver::onCameraWillChange,
        MapObserver::CameraChangeMode::Immediate);

    validateEvent(
        test,
        "onCameraDidChange",
        [&](const mbgl::JSValue& json) {
            EXPECT_TRUE(json.HasMember("cameraMode"));
            EXPECT_TRUE(json["cameraMode"].IsInt());
            EXPECT_EQ(json["cameraMode"].GetInt(), static_cast<int>(MapObserver::CameraChangeMode::Immediate));
        },
        &TransformObserver::onCameraDidChange,
        MapObserver::CameraChangeMode::Immediate);

    validateEvent(test, "onWillStartLoadingMap", nullptr, &style::Observer::onStyleLoading);
    validateEvent(test, "onDidFinishLoadingStyle", nullptr, &style::Observer::onStyleLoaded);

    validateEvent(
        test,
        "onSourceChanged",
        [&](const mbgl::JSValue& json) {
            EXPECT_TRUE(json.HasMember("type"));
            EXPECT_TRUE(json["type"].IsString());
            EXPECT_EQ(Enum<style::SourceType>::toEnum(json["type"].GetString()), style::SourceType::GeoJSON);

            EXPECT_TRUE(json.HasMember("id"));
            EXPECT_TRUE(json["id"].IsString());
            EXPECT_STREQ(json["id"].GetString(), testSource->getID().c_str());
        },
        &style::Observer::onSourceChanged,
        *testSource);

    validateEvent(test, "onRegisterShaders", nullptr, &RendererObserver::onRegisterShaders, *testShaderRegistry);

    validateEvent(
        test,
        "onDidFailLoadingMap",
        [&](const mbgl::JSValue& json) {
            EXPECT_TRUE(json.HasMember("error"));
            EXPECT_TRUE(json["error"].IsString());
            EXPECT_STREQ(json["error"].GetString(), testExceptionMessage);

            EXPECT_TRUE(json.HasMember("code"));
            EXPECT_TRUE(json["code"].IsInt());
            EXPECT_EQ(json["code"].GetInt(), static_cast<int>(MapLoadError::UnknownError));
        },
        &style::Observer::onStyleError,
        testException);

    const auto checkSprite = [&](const mbgl::JSValue& json) {
        EXPECT_TRUE(json.HasMember("id"));
        EXPECT_TRUE(json["id"].IsString());
        EXPECT_EQ(json["id"].GetString(), testSprite.id);

        EXPECT_TRUE(json.HasMember("url"));
        EXPECT_TRUE(json["url"].IsString());
        EXPECT_EQ(json["url"].GetString(), testSprite.spriteURL);
    };

    validateEvent(test, "onSpriteLoaded", checkSprite, &style::Observer::onSpriteLoaded, testSprite);
    validateEvent(test, "onSpriteRequested", checkSprite, &style::Observer::onSpriteRequested, testSprite);
    validateEvent(
        test,
        "onSpriteError",
        [&](const mbgl::JSValue& json) {
            checkSprite(json);

            EXPECT_TRUE(json.HasMember("error"));
            EXPECT_TRUE(json["error"].IsString());
            EXPECT_STREQ(json["error"].GetString(), testExceptionMessage);
        },
        &style::Observer::onSpriteError,
        testSprite,
        testException);

    validateEvent(test, "onWillStartRenderingMap", nullptr, &RendererObserver::onWillStartRenderingMap);

    // `onDidFinishRenderingMap` and `onDidFinishLoadingMap` are generated by the same callback
    validateEventList(test,
                      {{"onDidFinishRenderingMap", nullptr}, {"onDidFinishLoadingMap", nullptr}},
                      &RendererObserver::onDidFinishRenderingMap);

    validateEvent(
        test,
        "onStyleImageMissing",
        [&](const mbgl::JSValue& json) {
            EXPECT_TRUE(json.HasMember("id"));
            EXPECT_TRUE(json["id"].IsString());
            EXPECT_STREQ(json["id"].GetString(), "image");
        },
        &RendererObserver::onStyleImageMissing,
        "image",
        []() {});

    const auto checkShader = [&](const mbgl::JSValue& json) {
        EXPECT_TRUE(json.HasMember("shader"));
        EXPECT_TRUE(json["shader"].IsString());
        EXPECT_EQ(Enum<shaders::BuiltIn>::toEnum(json["shader"].GetString()), shaders::BuiltIn::None);

        EXPECT_TRUE(json.HasMember("backend"));
        EXPECT_TRUE(json["backend"].IsInt());
        EXPECT_EQ(json["backend"].GetInt(), static_cast<int>(gfx::Backend::Type::OpenGL));
    };

    validateEvent(test,
                  "onPreCompileShader",
                  checkShader,
                  &RendererObserver::onPreCompileShader,
                  shaders::BuiltIn::None,
                  gfx::Backend::Type::OpenGL,
                  "defines");

    validateEvent(test,
                  "onPostCompileShader",
                  checkShader,
                  &RendererObserver::onPostCompileShader,
                  shaders::BuiltIn::None,
                  gfx::Backend::Type::OpenGL,
                  "defines");

    validateEvent(
        test,
        "onShaderCompileFailed",
        [&](const mbgl::JSValue& json) {
            checkShader(json);

            EXPECT_TRUE(json.HasMember("defines"));
            EXPECT_TRUE(json["defines"].IsString());
            EXPECT_STREQ(json["defines"].GetString(), "defines");
        },
        &RendererObserver::onShaderCompileFailed,
        shaders::BuiltIn::None,
        gfx::Backend::Type::OpenGL,
        "defines");

    const auto checkGlyphs = [&](const mbgl::JSValue& json) {
        EXPECT_TRUE(json.HasMember("fonts"));
        EXPECT_TRUE(json["fonts"].IsArray());
        EXPECT_EQ(json["fonts"].GetArray().Size(), testFontStack.size());

        auto fontIt = testFontStack.begin();
        for (const auto& font : json["fonts"].GetArray()) {
            EXPECT_TRUE(font.IsString());
            EXPECT_EQ(font.GetString(), *fontIt++);
        }

        EXPECT_TRUE(json.HasMember("rangeStart"));
        EXPECT_TRUE(json["rangeStart"].IsInt());
        EXPECT_EQ(json["rangeStart"].GetInt(), testGlyphRange.first);

        EXPECT_TRUE(json.HasMember("rangeEnd"));
        EXPECT_TRUE(json["rangeEnd"].IsInt());
        EXPECT_EQ(json["rangeEnd"].GetInt(), testGlyphRange.second);
    };

    validateEvent(
        test, "onGlyphsLoaded", checkGlyphs, &RendererObserver::onGlyphsLoaded, testFontStack, testGlyphRange);
    validateEvent(
        test, "onGlyphsRequested", checkGlyphs, &RendererObserver::onGlyphsRequested, testFontStack, testGlyphRange);

    validateEvent(
        test,
        "onGlyphsError",
        [&](const mbgl::JSValue& json) {
            checkGlyphs(json);

            EXPECT_TRUE(json.HasMember("error"));
            EXPECT_TRUE(json["error"].IsString());
            EXPECT_STREQ(json["error"].GetString(), testExceptionMessage);
        },
        &RendererObserver::onGlyphsError,
        testFontStack,
        testGlyphRange,
        testException);

    validateEvent(test,
                  "onTileAction",
                  nullptr,
                  &RendererObserver::onTileAction,
                  TileOperation::StartParse,
                  OverscaledTileID(0, 0, 0),
                  "");

    const auto onDidFinishRenderingFrame =
        static_cast<void (RendererObserver::*)(RendererObserver::RenderMode, bool, bool, const gfx::RenderingStats&)>(
            &RendererObserver::onDidFinishRenderingFrame);

    validateEvent(test,
                  "onDidBecomeIdle",
                  nullptr,
                  onDidFinishRenderingFrame,
                  RendererObserver::RenderMode::Full,
                  false,
                  false,
                  gfx::RenderingStats());
}

TEST(ActionJournal, RenderingStats) {
    ActionJournalTest test(ActionJournalOptions()
                               .enable()
                               .withPath(".")
                               .withLogFileCount(2)
                               .withLogFileSize(1024 * 1024 * 5)
                               .withRenderingStatsReportInterval(2),
                           MapMode::Continuous);

    test.map->getStyle().loadJSON(util::read_file("test/fixtures/api/empty.json"));
    test.map->getActionJournal()->impl->flush();
    test.map->getActionJournal()->impl->clearLog();

    const auto onDidFinishRenderingFrame =
        static_cast<void (RendererObserver::*)(RendererObserver::RenderMode, bool, bool, const gfx::RenderingStats&)>(
            &RendererObserver::onDidFinishRenderingFrame);

    const std::vector<gfx::RenderingStats> testStats = {
        {.encodingTime = 0.00273499998729676, .renderingTime = 0.0000957687443587929},
        {.encodingTime = 0.010939366680880388, .renderingTime = 0.0001340633297028641},
        {.encodingTime = 0.023845999967306852, .renderingTime = 0.00011808499693870545},
        {.encodingTime = 0.018161798150416603, .renderingTime = 0.00011148519331106433},
    };

    const auto encodingMin = std::ranges::min_element(
        testStats, [](const auto& a, const auto& b) { return a.encodingTime < b.encodingTime; });
    const auto encodingMax = std::ranges::min_element(
        testStats, [](const auto& a, const auto& b) { return a.encodingTime > b.encodingTime; });
    const auto renderingMin = std::ranges::min_element(
        testStats, [](const auto& a, const auto& b) { return a.renderingTime < b.renderingTime; });
    const auto renderingMax = std::ranges::min_element(
        testStats, [](const auto& a, const auto& b) { return a.renderingTime > b.renderingTime; });

    const auto encodingAvg = std::accumulate(testStats.begin(),
                                             testStats.end(),
                                             0.0,
                                             [](const auto& a, const auto& b) { return a + b.encodingTime; }) /
                             testStats.size();

    const auto renderingAvg = std::accumulate(testStats.begin(),
                                              testStats.end(),
                                              0.0,
                                              [](const auto& a, const auto& b) { return a + b.renderingTime; }) /
                              testStats.size();

    const auto validateDouble = [](const mbgl::JSValue& json, const auto name, const double value) {
        EXPECT_TRUE(json.HasMember(name));
        EXPECT_TRUE(json[name].IsDouble());
        EXPECT_DOUBLE_EQ(json[name].GetDouble(), value);
    };

    const auto validateStats = [&](const mbgl::JSValue& json) {
        ;
        validateDouble(json, "encodingMin", encodingMin->encodingTime);
        validateDouble(json, "encodingMax", encodingMax->encodingTime);
        validateDouble(json, "encodingAvg", encodingAvg);
        validateDouble(json, "renderingMin", renderingMin->renderingTime);
        validateDouble(json, "renderingMax", renderingMax->renderingTime);
        validateDouble(json, "renderingAvg", renderingAvg);
    };

    // accumulate frames in the journal
    for (const auto& frameStats : testStats) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(test.options.renderingStatsReportInterval() * 1000 / 2 / testStats.size()));
        test.map->getImpl().onDidFinishRenderingFrame(RendererObserver::RenderMode::Partial, false, false, frameStats);

        test.map->getActionJournal()->impl->flush();
        const auto& log = test.map->getActionJournal()->getLog();
        EXPECT_EQ(log.size(), 0);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(test.options.renderingStatsReportInterval() * 1000 / 2));

    // trigger the journal event using a frame with average times to keep the state unchanged
    validateEvent(test,
                  "renderingStats",
                  validateStats,
                  onDidFinishRenderingFrame,
                  RendererObserver::RenderMode::Partial,
                  false,
                  false,
                  gfx::RenderingStats{.encodingTime = encodingAvg, .renderingTime = renderingAvg});
}
