#include <mbgl/test/util.hpp>
#include <mbgl/test/fixture_log_observer.hpp>
#include <mbgl/test/stub_file_source.hpp>

#include <mbgl/sprite/sprite_loader.hpp>
#include <mbgl/sprite/sprite_loader_observer.hpp>
#include <mbgl/sprite/sprite_parser.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>

#include <utility>

using namespace mbgl;
using namespace mbgl::style;

class StubSpriteLoaderObserver : public SpriteLoaderObserver {
public:
    void onSpriteLoaded(std::optional<Sprite>, std::vector<Immutable<style::Image::Impl>> images) override {
        if (spriteLoaded) spriteLoaded(std::move(images));
    }

    void onSpriteError(std::optional<Sprite>, std::exception_ptr error) override {
        if (spriteError) spriteError(error);
    }

    std::function<void(std::vector<Immutable<style::Image::Impl>>)> spriteLoaded;
    std::function<void(std::exception_ptr)> spriteError;
};

std::string defaultSpritePath = "test/fixtures/resources/sprite";

class SpriteLoaderTest {
public:
    SpriteLoaderTest(float pixelRatio = 1)
        : threadPool(Scheduler::GetBackground(), uniqueID),
          spriteLoader(pixelRatio, threadPool) {}

    ~SpriteLoaderTest() {
        threadPool.waitForEmpty();
        threadPool.runRenderJobs(true);
    }

    util::SimpleIdentity uniqueID;
    util::RunLoop loop;
    StubFileSource fileSource;
    StubSpriteLoaderObserver observer;
    TaggedScheduler threadPool;
    SpriteLoader spriteLoader;
    std::string spritePath = defaultSpritePath; // default

    void run() {
        // Squelch logging.
        Log::setObserver(std::make_unique<Log::NullObserver>());

        spriteLoader.setObserver(&observer);

        Sprite sprite = Sprite("default", spritePath);
        spriteLoader.load(sprite, fileSource);

        loop.run();
    }

    void end() { loop.stop(); }
};

auto successfulSpriteImageResponse(const std::string& spritePath = defaultSpritePath, float pixelRatio = 1.0f) {
    return [spritePath, pixelRatio](const Resource& resource) {
        EXPECT_EQ(spritePath + (pixelRatio == 2.0 ? "@2x" : "") + ".png", resource.url);
        Response response;
        response.data = std::make_unique<std::string>(util::read_file(resource.url));
        return response;
    };
}

auto successfulSpriteJSONResponse(const std::string& spritePath = defaultSpritePath, float pixelRatio = 1.0f) {
    return [spritePath, pixelRatio](const Resource& resource) {
        EXPECT_EQ(spritePath + (pixelRatio == 2.0 ? "@2x" : "") + ".json", resource.url);
        Response response;
        response.data = std::make_unique<std::string>(util::read_file(resource.url));
        return response;
    };
}

Response failedSpriteResponse(const Resource&) {
    Response response;
    response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other, "Failed by the test case");
    return response;
}

Response corruptSpriteResponse(const Resource&) {
    Response response;
    response.data = std::make_unique<std::string>("CORRUPT");
    return response;
}

class SpriteLoaderParametrized : public testing::TestWithParam<std::tuple<std::string, size_t, float>> {};

TEST_P(SpriteLoaderParametrized, LoadingSuccess) {
    const auto [spritePath, expectedImages_, pixelRatio_] = GetParam();
    const auto expectedImages = expectedImages_; // lambda cannot capture structured binding yet
    const auto pixelRatio = pixelRatio_;

    SpriteLoaderTest test{pixelRatio};
    test.spritePath = spritePath;

    test.fileSource.spriteImageResponse = successfulSpriteImageResponse(spritePath, pixelRatio);
    test.fileSource.spriteJSONResponse = successfulSpriteJSONResponse(spritePath, pixelRatio);

    test.observer.spriteError = [&](std::exception_ptr error) {
        FAIL() << util::toString(error);
        test.end();
    };

    test.observer.spriteLoaded = [&, expectedImages](std::vector<Immutable<style::Image::Impl>> images) {
        EXPECT_EQ(images.size(), expectedImages);
        test.end();
    };

    test.run();
}

INSTANTIATE_TEST_SUITE_P(
    SpriteLoaderSprites,
    SpriteLoaderParametrized,
    ::testing::Values(std::make_tuple("test/fixtures/resources/sprite", 418, 1.0f),
                      std::make_tuple("test/fixtures/resources/versatiles-sprite/sprite", 112, 1.0f),
                      std::make_tuple("test/fixtures/resources/versatiles-sprite/sprite", 112, 2.0f)));

TEST(SpriteLoader, JSONLoadingFail) {
    SpriteLoaderTest test;

    test.fileSource.spriteImageResponse = successfulSpriteImageResponse();
    test.fileSource.spriteJSONResponse = failedSpriteResponse;

    test.observer.spriteError = [&](std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ("Failed by the test case", util::toString(error));
        test.end();
    };

    test.run();
}

TEST(SpriteLoader, ImageLoadingFail) {
    SpriteLoaderTest test;

    test.fileSource.spriteImageResponse = failedSpriteResponse;
    test.fileSource.spriteJSONResponse = successfulSpriteJSONResponse();

    test.observer.spriteError = [&](std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ("Failed by the test case", util::toString(error));
        test.end();
    };

    test.run();
}

TEST(SpriteLoader, JSONLoadingCorrupted) {
    SpriteLoaderTest test;

    test.fileSource.spriteImageResponse = successfulSpriteImageResponse();
    test.fileSource.spriteJSONResponse = corruptSpriteResponse;

    test.observer.spriteError = [&](std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ("Failed to parse JSON: Invalid value. at offset 0", util::toString(error));
        test.end();
    };

    test.run();
}

TEST(SpriteLoader, ImageLoadingCorrupted) {
    SpriteLoaderTest test;

    test.fileSource.spriteImageResponse = corruptSpriteResponse;
    test.fileSource.spriteJSONResponse = successfulSpriteJSONResponse();

    test.observer.spriteError = [&](std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        // Not asserting on platform-specific error text.
        test.end();
    };

    test.run();
}

TEST(SpriteLoader, LoadingCancel) {
    SpriteLoaderTest test;

    test.fileSource.spriteImageResponse = test.fileSource.spriteJSONResponse = [&](const Resource&) {
        test.end();
        return std::optional<Response>();
    };

    test.observer.spriteLoaded = [&](std::vector<Immutable<style::Image::Impl>>) {
        FAIL() << "Should never be called";
    };

    test.run();
}
