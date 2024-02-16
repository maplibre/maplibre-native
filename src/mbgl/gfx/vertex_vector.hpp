#pragma once

#include <mbgl/util/ignore.hpp>

#include <memory>
#include <vector>

namespace mbgl {
namespace gfx {

struct VertexBufferBase {
    virtual ~VertexBufferBase() = default;
};

class VertexVectorBase {
public:
    VertexVectorBase() = default;
    VertexVectorBase(const VertexVectorBase&) {} // buffer is not copied
    VertexVectorBase(VertexVectorBase&& other)
        :
#if MLN_DRAWABLE_RENDERER
          buffer(std::move(other.buffer)),
#endif // MLN_DRAWABLE_RENDERER
          dirty(other.dirty),
          released(other.released) {
    }
    virtual ~VertexVectorBase() = default;

    virtual const void* getRawData() const = 0;
    virtual std::size_t getRawSize() const = 0;
    virtual std::size_t getRawCount() const = 0;

#if MLN_DRAWABLE_RENDERER
    VertexBufferBase* getBuffer() const { return buffer.get(); }
    void setBuffer(std::unique_ptr<VertexBufferBase>&& value) { buffer = std::move(value); }
#endif // MLN_DRAWABLE_RENDERER

    bool getDirty() const { return dirty; }
    void setDirty(bool value = true) { dirty = value; }

    bool isReleased() const { return released; }

protected:
#if MLN_DRAWABLE_RENDERER
    std::unique_ptr<VertexBufferBase> buffer;
#endif // MLN_DRAWABLE_RENDERER
    bool dirty = true;
    bool released = false;
};
using VertexVectorBasePtr = std::shared_ptr<VertexVectorBase>;

template <class V>
class VertexVector final : public VertexVectorBase {
public:
    using Vertex = V;

    VertexVector() = default;
    VertexVector(const VertexVector<V>& other)
        : VertexVectorBase(other),
          v(other.v) {}
    VertexVector(VertexVector<V>&& other)
        : VertexVectorBase(std::move(other)),
          v(std::move(other.v)) {}
    ~VertexVector() override = default;

    template <class... Args>
    void emplace_back(Args&&... args) {
        assert(!released);
        util::ignore({(v.emplace_back(std::forward<Args>(args)), 0)...});
        dirty = true;
    }

    void extend(std::size_t n, const Vertex& val) {
        assert(!released);
        v.resize(v.size() + n, val);
        dirty = true;
    }

    Vertex& at(std::size_t n) {
        assert(n < v.size());
        assert(!released);
        dirty = true;
        return v.at(n);
    }
    const Vertex& at(std::size_t n) const {
        assert(n < v.size());
        return v.at(n);
    }

    std::size_t elements() const { return v.size(); }

    std::size_t bytes() const { return v.size() * sizeof(Vertex); }

    bool empty() const { return v.empty(); }

    void clear() {
        dirty = true;
        v.clear();
    }

    void reserve(std::size_t count) { v.reserve(count); }

    /// Indicate that this shared vertex vector instance will no longer be updated.
    void release() {
#if MLN_DRAWABLE_RENDERER
        // If we've already created a buffer, we don't need the raw data any more.
        if (buffer) {
            v.clear();
        }
#endif // MLN_DRAWABLE_RENDERER
        released = true;
    }

    const Vertex* data() const { return v.data(); }

    const std::vector<Vertex>& vector() const { return v; }

    const void* getRawData() const override { return v.data(); }
    std::size_t getRawSize() const override { return sizeof(Vertex); }
    std::size_t getRawCount() const override { return v.size(); }

private:
    std::vector<Vertex> v;
};

template <typename T>
using VertexVectorPtr = std::shared_ptr<VertexVector<T>>;

} // namespace gfx
} // namespace mbgl
