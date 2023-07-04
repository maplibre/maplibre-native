#pragma once

#include <mbgl/util/ignore.hpp>

#include <vector>

namespace mbgl {
namespace gfx {

struct VertexBufferBase {
    virtual ~VertexBufferBase() = default;
};

class VertexVectorBase {
public:
    virtual const void* getRawData() const = 0;
    virtual std::size_t getRawSize() const = 0;
    virtual std::size_t getRawCount() const = 0;

    VertexBufferBase* getBuffer() const { return buffer.get(); }
    void setBuffer(std::unique_ptr<VertexBufferBase>&& value) { buffer = std::move(value); }

    bool getDirty() const { return dirty; }
    void setDirty(bool value = true) { dirty = value; }

    bool isReleased() const { return released; }

protected:
    std::unique_ptr<VertexBufferBase> buffer;
    bool dirty = true;
    bool released = false;
};
using VertexVectorBasePtr = std::shared_ptr<VertexVectorBase>;

template <class V>
class VertexVector : public VertexVectorBase {
public:
    using Vertex = V;

    template <typename Arg>
    void emplace_back(Arg&& vertex) {
        v.emplace_back(std::forward<Arg>(vertex));
    }

    void extend(std::size_t n, const Vertex& val) {
        v.resize(v.size() + n, val);
        dirty = true;
    }

    Vertex& at(std::size_t n) {
        assert(n < v.size());
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

    void release() {
        // If we've already created a buffer, we don't need the raw data any more.
        if (buffer) {
            clear();
        }
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

} // namespace gfx
} // namespace mbgl
