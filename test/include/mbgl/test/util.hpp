#pragma once

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#define TEST_READ_ONLY 0

// iOS simulator server can work if port 3000 is available
#if !ANDROID && !TARGET_OS_SIMULATOR
#ifndef TEST_HAS_SERVER
#define TEST_HAS_SERVER 1
#endif
#endif

#if TARGET_OS_SIMULATOR
#define TEST_IS_SIMULATOR 1
#else
#define TEST_IS_SIMULATOR 0
#endif

#if !TEST_IS_SIMULATOR && !CI_BUILD
#define TEST_REQUIRES_ACCURATE_TIMING(name) name
#else
#define TEST_REQUIRES_ACCURATE_TIMING(name) DISABLED_##name
#endif

#if !TEST_READ_ONLY
#define TEST_REQUIRES_WRITE(name) name
#else
#define TEST_REQUIRES_WRITE(name) DISABLED_##name
#endif

#if TEST_HAS_SERVER
#define TEST_REQUIRES_SERVER(name) name
#else
#define TEST_REQUIRES_SERVER(name) DISABLED_##name
#endif

#if !CI_BUILD
#define TEST_DISABLED_ON_CI(name) name
#else
#define TEST_DISABLED_ON_CI(name) DISABLED_##name
#endif

#include <mbgl/util/image.hpp>
#include <mbgl/util/chrono.hpp>

#include <cstdint>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

namespace httplib {
class Server;
} // namespace httplib

namespace mbgl {
namespace test {

class HttpServer {
public:
    HttpServer();
    ~HttpServer();

private:
    std::unique_ptr<httplib::Server> server;
    std::thread serverThread;
};

void checkImage(const std::string& base,
                const PremultipliedImage& actual,
                double imageThreshold = 0,
                double pixelThreshold = 0);

void checkImages(const std::vector<std::string>& possibleExpected,
                 const PremultipliedImage& actual,
                 double imageThreshold = 0,
                 double pixelThreshold = 0);

double getImageDiff(const std::string& base, const PremultipliedImage& actual, double pixelThreshold = 0);

} // namespace test
} // namespace mbgl
