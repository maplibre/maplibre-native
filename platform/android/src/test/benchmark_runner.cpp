#include <mln/benchmark.hpp>
#include "test_runner_common.hpp"

#include <unistd.h>
#include <thread>

using namespace mln;
using namespace mln::android;

bool running = false;
bool done = false;
ALooper* looper = NULL;

void runner(const std::string& storagePath, const std::string& benchmarkFilter) {
    std::vector<std::string> arguments = {"mbgl-benchmark-runner",
                                          "--benchmark_format=json",
                                          "--benchmark_out=" + storagePath + "/benchmark/results/results.json"};
    if (!benchmarkFilter.empty()) {
        arguments.emplace_back("--benchmark_filter=" + benchmarkFilter);
    }
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back(const_cast<char*>(arg.data()));
    }
    argv.push_back(nullptr);

    mln::Log::Info(mln::Event::General, "Start BenchmarkRunner");
    int status = mln::runBenchmark(argv.size(), argv.data());
    mln::Log::Info(mln::Event::General, "BenchmarkRunner finished with status: '" + std::to_string(status) + "'");
    running = false;
    ALooper_wake(looper);
}

void android_main(struct android_app* app) {
    mln::android::theJVM = app->activity->vm;
    JNIEnv* env = nullptr;
    std::thread benchmarkThread;
    app->activity->vm->AttachCurrentThread(&env, NULL);
    looper = ALooper_forThread();

    std::string storagePath(app->activity->internalDataPath);
    std::string benchmarkFilter = getIntentExtra(env, app, "benchmark_filter");
    std::string zipFile = storagePath + "/data.zip";

    if (copyFile(env, app->activity->assetManager, zipFile, storagePath, "data.zip")) {
        if (chdir(storagePath.c_str())) {
            mln::Log::Error(mln::Event::General, "Failed to change the directory to " + storagePath);
            done = true;
            changeState(env, app, false);
        } else {
            unZipFile(env, zipFile, storagePath);
            running = true;
            benchmarkThread = std::thread(runner, storagePath, benchmarkFilter);
        }
    } else {
        mln::Log::Error(mln::Event::General, "Failed to copy zip file '" + zipFile + "' to app storage");
        done = true;
        changeState(env, app, false);
    }

    int outFd, outEvents;
    struct android_poll_source* source = nullptr;

    while (true) {
        ALooper_pollOnce(-1, &outFd, &outEvents, reinterpret_cast<void**>(&source));
        if (source != nullptr) {
            source->process(app, source);
        }

        if (!running && !done) {
            mln::Log::Info(mln::Event::General, "BenchmarkRunner done");
            done = true;
            benchmarkThread.join();
            changeState(env, app, true);
        }

        if (app->destroyRequested != 0) {
            app->activity->vm->DetachCurrentThread();
            mln::Log::Info(mln::Event::General, "Close the App!");
            return;
        }
    }
}
