#include <mbgl/test.hpp>
#include "test_runner_common.hpp"

#include <unistd.h>
#include <mutex>
#include <thread>
#include <gtest/gtest.h>
#include <android/log.h>

using namespace mbgl;
using namespace mbgl::android;

std::string testResultToStr(const ::testing::TestResult* testResult) {
    if (testResult->Passed()) return " (PASSED)";
    if (testResult->Skipped()) return " (SKIPPED)";
    if (testResult->Failed()) return " (FAILED)";
    return "";
}

class GTestAndroidLogger : public ::testing::EmptyTestEventListener {
public:
    void OnTestProgramStart(const ::testing::UnitTest&) override {
        __android_log_print(ANDROID_LOG_INFO, tag, "Test program started.");
    }

    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        __android_log_print(
            ANDROID_LOG_INFO, tag, "Test program ended with %d tests run.", unit_test.total_test_count());
    }

    void OnTestStart(const ::testing::TestInfo& test_info) override {
        __android_log_print(ANDROID_LOG_INFO, tag, "Starting: %s.%s", test_info.test_case_name(), test_info.name());
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        __android_log_print(ANDROID_LOG_INFO,
                            tag,
                            "Finished: %s.%s%s",
                            test_info.test_case_name(),
                            test_info.name(),
                            testResultToStr(test_info.result()).c_str());
    }

    void OnTestPartResult(const ::testing::TestPartResult& test_part_result) override {
        __android_log_print(ANDROID_LOG_INFO,
                            tag,
                            "%s: %s",
                            test_part_result.failed() ? "FAILED" : "PASSED",
                            test_part_result.summary());
    }

    const char* tag = "GTest";
};

std::atomic<bool> running{true};
std::atomic<bool> success{false};
std::once_flag done;
ALooper* looper = NULL;

void runner(const std::string& gtestOutput) {
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new GTestAndroidLogger); // takes ownership

    std::vector<std::string> arguments = {
        "mbgl-test-runner", "--gtest_output=xml:" + gtestOutput, "--gtest_filter=-StringIndexer.GetOOBIdentity"};
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    mbgl::Log::Info(mbgl::Event::General, "Start TestRunner");
    int status = mbgl::runTests(argv.size(), argv.data());
    mbgl::Log::Info(mbgl::Event::General, "TestRunner finished with status: '" + std::to_string(status) + "'");
    running = false;
    success = (status == 0);
    ALooper_wake(looper);
}

void android_main(struct android_app* app) {
    mbgl::android::theJVM = app->activity->vm;
    JNIEnv* env = nullptr;
    std::thread runnerThread;
    app->activity->vm->AttachCurrentThread(&env, NULL);
    looper = ALooper_forThread();

    std::string storagePath(app->activity->internalDataPath);
    std::string zipFile = storagePath + "/data.zip";
    const std::string gtestOutput = storagePath + "/results.xml";

    if (copyFile(env, app->activity->assetManager, zipFile, storagePath, "data.zip")) {
        if (chdir(storagePath.c_str())) {
            mbgl::Log::Error(mbgl::Event::General, "Failed to change the directory to storage dir");
            changeState(env, app, success);
        } else {
            unZipFile(env, zipFile, storagePath);
            runnerThread = std::thread(runner, gtestOutput);
        }
    } else {
        mbgl::Log::Error(mbgl::Event::General, "Failed to copy zip file '" + zipFile + "' to external storage");
        changeState(env, app, success);
    }

    int outFd, outEvents;
    struct android_poll_source* source = nullptr;

    while (true) {
        ALooper_pollAll(-1, &outFd, &outEvents, reinterpret_cast<void**>(&source));
        if (source != nullptr) {
            source->process(app, source);
        }

        if (!running) {
            std::call_once(done, [&] {
                mbgl::Log::Info(mbgl::Event::General, "TestRunner done");
                runnerThread.join();
                changeState(env, app, success);
            });
        }

        if (app->destroyRequested != 0) {
            app->activity->vm->DetachCurrentThread();
            mbgl::Log::Info(mbgl::Event::General, "Close the App!");
            return;
        }
    }
}
