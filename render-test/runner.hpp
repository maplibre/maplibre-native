#pragma once

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map.hpp>
#include <mln/storage/file_source.hpp>

#include "manifest_parser.hpp"

#include <memory>
#include <string>

class TestRunnerMapObserver;
struct TestMetadata;

class TestRunnerMapObserver : public mln::MapObserver {
public:
    TestRunnerMapObserver() = default;
    void onDidFailLoadingMap(mln::MapLoadError, const std::string&) override { mapLoadFailure = true; }

    void onDidFinishRenderingMap(RenderMode mode) override final {
        if (!finishRenderingMap) finishRenderingMap = mode == RenderMode::Full;
    }

    void onDidBecomeIdle() override final { idle = true; }

    void reset() {
        mapLoadFailure = false;
        finishRenderingMap = false;
        idle = false;
    }

    bool mapLoadFailure;
    bool finishRenderingMap;
    bool idle;
};

class TestRunner {
public:
    enum class UpdateResults {
        NO,
        DEFAULT,
        PLATFORM,
        METRICS,
        REBASELINE
    };

    TestRunner(Manifest, UpdateResults);
    void run(TestMetadata&);
    void reset();

    // Manifest
    const Manifest& getManifest() const;
    void doShuffle(uint32_t seed);

private:
    mln::HeadlessFrontend::RenderResult runTest(TestMetadata& metadata, TestContext& ctx);
    void checkQueryTestResults(mln::PremultipliedImage&& actualImage,
                               std::vector<mln::Feature>&& features,
                               TestMetadata&);
    void checkRenderTestResults(mln::PremultipliedImage&& image, TestMetadata&);
    void checkProbingResults(TestMetadata&);
    void appendLabelCutOffResults(TestMetadata&, const std::string&, const std::string&);
    void registerProxyFileSource();

    struct Impl {
        Impl(const TestMetadata&, const mln::ResourceOptions&, const mln::ClientOptions&);
        ~Impl();

        std::unique_ptr<TestRunnerMapObserver> observer;
        mln::HeadlessFrontend frontend;
        std::shared_ptr<mln::FileSource> fileSource;
        mln::Map map;
    };
    std::unordered_map<std::string, std::unique_ptr<Impl>> maps;
    Manifest manifest;
    UpdateResults updateResults;
};
