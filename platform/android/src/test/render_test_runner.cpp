#include <mbgl/render_test.hpp>
#include "test_runner_common.hpp"

#include <functional>

using namespace mbgl;
using namespace mbgl::android;

void android_main(struct android_app* app) {
    mbgl::android::theJVM = app->activity->vm;
    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, NULL);

    const char* storage_chars = app->activity->internalDataPath;
    std::string storagePath(storage_chars);
    std::string zipFile = storagePath + "/data.zip";

    int outFd, outEvents;
    struct android_poll_source* source = nullptr;
    if (!copyFile(env, app->activity->assetManager, zipFile, storagePath, "data.zip")) {
        mbgl::Log::Error(mbgl::Event::General,
                         "Failed to copy zip File '" + zipFile + "' to external storage for upzipping");
    } else {
        unZipFile(env, zipFile, storagePath);

        auto runTestWithManifest =
            [&storagePath, &app, &outFd, &outEvents, &source](const std::string& manifest) -> bool {
            const std::string configFile = storagePath + manifest;
            std::vector<std::string> arguments = {"mbgl-render-test-runner", "-p", configFile, "-u", "rebaseline"};
            std::vector<char*> argv;
            for (const auto& arg : arguments) {
                argv.push_back(const_cast<char*>(arg.data()));
            }
            argv.push_back(nullptr);

            int finishedTestCount = 0;
            std::function<void()> testStatus = [&]() {
                auto result = ALooper_pollOnce(0, &outFd, &outEvents, reinterpret_cast<void**>(&source));
                if (result == ALOOPER_POLL_ERROR) {
                    throw std::runtime_error("ALooper_pollOnce returned an error");
                }

                if (source != nullptr) {
                    source->process(app, source);
                }

                mbgl::Log::Info(mbgl::Event::General,
                                "Current finished tests number is '" + std::to_string(++finishedTestCount) + "'");
            };
            mbgl::Log::Info(mbgl::Event::General, "Start running RenderTestRunner with manifest: '" + manifest + "'");
            bool result = mbgl::runRenderTests(argv.size() - 1, argv.data(), testStatus) == 0;
            mbgl::Log::Info(mbgl::Event::General, "End running RenderTestRunner with manifest: '" + manifest + "'");
            return result;
        };

#if MLN_RENDER_BACKEND_VULKAN
        auto result = runTestWithManifest("/metrics/android-vulkan-render-test-runner-metrics.json");
        result = runTestWithManifest("/metrics/android-vulkan-render-test-runner-style.json") && result;
#else
        auto result = runTestWithManifest("/metrics/android-render-test-runner-metrics.json");
        result = runTestWithManifest("/metrics/android-render-test-runner-style.json") && result;
#endif
        mbgl::Log::Info(mbgl::Event::General, "All tests are finished!");
        changeState(env, app, result);
    }
    while (true) {
        auto result = ALooper_pollOnce(0, &outFd, &outEvents, reinterpret_cast<void**>(&source));
        if (result == ALOOPER_POLL_ERROR) {
            throw std::runtime_error("ALooper_pollOnce returned an error");
        }

        if (source != nullptr) {
            source->process(app, source);
        }
        if (app->destroyRequested != 0) {
            app->activity->vm->DetachCurrentThread();
            mbgl::Log::Info(mbgl::Event::General, "Close the App!");
            return;
        }
    }
}
