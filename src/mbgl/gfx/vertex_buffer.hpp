#pragma once

#include <memory>
#include <cassert>

namespace mbgl {
namespace gfx {

class VertexBufferResource {
protected:
    VertexBufferResource() noexcept = default;

public:
    virtual ~VertexBufferResource() noexcept = default;
};

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

// This class has a template argument that we use to specify the vertex type. It
// is not used by the implementation, but serves type checking purposes during
// build time.
template <class>
class VertexBuffer {
public:
    VertexBuffer(const std::size_t elements_, UniqueVertexBufferResource&& resource_)
        : elements(elements_),
          resource(std::move(resource_)) {}

    std::size_t elements;

    template <typename T = VertexBufferResource>
    T& getResource() const {
        assert(resource);
        return static_cast<T&>(*resource);
    }

protected:
    UniqueVertexBufferResource resource;
};

} // namespace gfx
} // namespace mbgl
