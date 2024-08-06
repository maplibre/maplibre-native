#pragma once

#include <mbgl/util/thread_pool.hpp>

#include <cassert>
#include <thread>

namespace mbgl {
namespace gl {

class RendererBackend;

class ResourceUploadThreadPool : public ThreadedScheduler {
public:
    ResourceUploadThreadPool(RendererBackend& backend);
};

} // namespace gl
} // namespace mbgl
