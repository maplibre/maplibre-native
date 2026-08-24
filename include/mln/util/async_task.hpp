#pragma once

#include <mln/util/noncopyable.hpp>

#include <memory>
#include <functional>

namespace mln {
namespace util {

class AsyncTask : private util::noncopyable {
public:
    AsyncTask(std::function<void()>&&);
    ~AsyncTask();

    void send();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace util
} // namespace mln
