#pragma once

#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/util/ignore.hpp>

#include <memory>
#include <vector>

namespace mbgl {
namespace gfx {

struct IndexBufferBase {
    virtual ~IndexBufferBase() = default;
};

class IndexVectorBase {
protected:
    std::vector<uint16_t> v;

public:
    using value_type = decltype(v)::value_type;

    IndexVectorBase() = default;
    IndexVectorBase(std::vector<uint16_t>&& indexes)
        : v(std::move(indexes)) {}
    IndexVectorBase(const IndexVectorBase& other)
        : v(other.v) {} // buffer is not copied
    IndexVectorBase(IndexVectorBase&& other)
        : v(std::move(other.v)),
          buffer(std::move(other.buffer)),
          dirty(other.dirty),
          released(other.released) {}
    virtual ~IndexVectorBase() = default;

    IndexBufferBase* getBuffer() const { return buffer.get(); }
    void setBuffer(std::unique_ptr<IndexBufferBase>&& value) { buffer = std::move(value); }

    bool getDirty() const { return dirty; }
    void setDirty(bool value = true) { dirty = value; }

    bool isReleased() const { return released; }

    void reserve(std::size_t count) { v.reserve(count); }

    void extend(std::size_t n, const uint16_t val) {
        assert(!released);
        v.resize(v.size() + n, val);
        dirty = true;
    }

    uint16_t& at(std::size_t n) {
        assert(n < v.size());
        assert(!released);
        dirty = true;
        return v.at(n);
    }
    const uint16_t& at(std::size_t n) const {
        assert(n < v.size());
        return v.at(n);
    }

    std::size_t elements() const { return v.size(); }

    std::size_t bytes() const { return v.size() * sizeof(uint16_t); }

    bool empty() const { return v.empty(); }

    void clear() {
        dirty = true;
        v.clear();
        buffer.reset();
    }

    /// Indicate that this shared index vector will no longer be updated.
    void release() {
        // If we've already created a buffer, we don't need the raw data any more.
        if (buffer) {
            v.clear();
        }
        released = true;
    }

    const uint16_t* data() const { return v.data(); }

    const std::vector<uint16_t>& vector() const { return v; }

protected:
    std::unique_ptr<IndexBufferBase> buffer;
    bool dirty = true;
    bool released = false;
};

using IndexVectorBasePtr = std::shared_ptr<IndexVectorBase>;

template <class DrawMode>
class IndexVector final : public IndexVectorBase {
public:
    static constexpr std::size_t groupSize = BufferGroupSizeOf<DrawMode>::value;

    template <class... Args>
    void emplace_back(Args&&... args) {
        static_assert(sizeof...(args) % groupSize == 0, "wrong buffer element count");
        assert(!released);
        util::ignore({(v.emplace_back(std::forward<Args>(args)), 0)...});
        dirty = true;
    }
};

using IndexVectorBasePtr = std::shared_ptr<IndexVectorBase>;

} // namespace gfx
} // namespace mbgl
