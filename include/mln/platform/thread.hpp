#pragma once

#include <functional>

namespace mln {
namespace platform {

/// Called when a thread is created
void attachThread();

/// Called when a thread is destroyed
void detachThread();

void runTask(const std::function<void()>& task);

} // namespace platform
} // namespace mln
