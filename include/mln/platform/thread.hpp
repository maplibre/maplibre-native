#pragma once

namespace mln {
namespace platform {

/// Called when a thread is created
void attachThread();

/// Called when a thread is destroyed
void detachThread();

} // namespace platform
} // namespace mln
