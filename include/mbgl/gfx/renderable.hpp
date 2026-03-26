#pragma once

#include <mbgl/util/size.hpp>

#include <memory>
#include <cassert>

namespace mbgl {
namespace gfx {

class RenderableResource {
protected:
    explicit RenderableResource() = default;

public:
    virtual ~RenderableResource() = default;
    RenderableResource(const RenderableResource&) = delete;
    RenderableResource& operator=(const RenderableResource&) = delete;

    virtual void bind() = 0;
};

class Renderable {
public:
    enum class SwapBehaviour {
        NoFlush,
        Flush
    };

protected:
    Renderable(const Size size_, std::unique_ptr<RenderableResource> resource_)
        : size(size_),
          resource(std::move(resource_)) {}
    virtual ~Renderable() = default;

public:
    Size getSize() const { return size; }

    template <typename T>
    T& getResource() const {
        assert(resource);
        return static_cast<T&>(*resource);
    }

    bool hasResource() const { return resource != nullptr; }

    void setResource(std::unique_ptr<RenderableResource> resource_) { resource = std::move(resource_); }

    virtual void wait() {}

    bool operator!=(const Renderable& other) const { return resource.get() != other.resource.get(); }

protected:
    Size size;
    std::unique_ptr<RenderableResource> resource;
};

} // namespace gfx
} // namespace mbgl
