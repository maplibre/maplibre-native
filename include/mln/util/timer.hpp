#pragma once

#include <mln/util/noncopyable.hpp>
#include <mln/util/chrono.hpp>

#include <memory>
#include <functional>

namespace mln {
namespace util {

class Timer : private util::noncopyable {
public:
    Timer();
    ~Timer();

    void start(Duration timeout, Duration repeat, std::function<void()>&&);
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace util
} // namespace mln
