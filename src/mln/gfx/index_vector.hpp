#pragma once

#include <mln/gfx/draw_mode.hpp>
#include <mln/util/ignore.hpp>

#include <memory>
#include <vector>

namespace mln {
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
          released(other.released),
          rawFreed(other.rawFreed),
          freedCount(other.freedCount) {}
    virtual ~IndexVectorBase() = default;

    IndexBufferBase* getBuffer() const { return buffer.get(); }
    void setBuffer(std::unique_ptr<IndexBufferBase>&& value) { buffer = std::move(value); }

    bool getDirty() const { return dirty; }
    void setDirty(bool value = true) {
        dirty = value;
        // Clean and uploaded: a released vector will not be modified or re-uploaded.
        if (!dirty && released && buffer) {
            freeRawData();
        }
    }

    bool isReleased() const { return released; }

    bool hasRawData() const { return !rawFreed; }

    /// Throw if a caller is about to read or write the CPU copy after it was discarded.
    void requireRawData() const {
        if (!hasRawData()) {
            throw std::logic_error("CPU index data was discarded after upload");
        }
    }

    void reserve(std::size_t count) { v.reserve(count); }

    void extend(std::size_t n, const uint16_t val) {
        requireRawData();
        assert(!released);
        v.resize(v.size() + n, val);
        dirty = true;
    }

    uint16_t& at(std::size_t n) {
        requireRawData();
        assert(n < v.size());
        assert(!released);
        dirty = true;
        return v.at(n);
    }
    const uint16_t& at(std::size_t n) const {
        requireRawData();
        assert(n < v.size());
        return v.at(n);
    }

    std::size_t elements() const { return v.size() + freedCount; }

    std::size_t bytes() const { return elements() * sizeof(uint16_t); }

    bool empty() const { return elements() == 0; }

    void clear() {
        dirty = true;
        v.clear();
        freedCount = 0;
        rawFreed = false;
        buffer.reset();
    }

    /// Indicate that this shared index vector will no longer be updated.
    void release() {
        released = true;
        // If we've already created a buffer, we don't need the raw data any more.
        if (buffer && !dirty) {
            freeRawData();
        }
    }

    const uint16_t* data() const {
        requireRawData();
        return v.data();
    }

    const std::vector<uint16_t>& vector() const { return v; }

protected:
    void freeRawData() {
        freedCount += v.size();
        std::vector<uint16_t>().swap(v);
        rawFreed = true;
    }

    std::unique_ptr<IndexBufferBase> buffer;
    bool dirty = true;
    bool released = false;
    bool rawFreed = false;
    std::size_t freedCount = 0;
};

using IndexVectorBasePtr = std::shared_ptr<IndexVectorBase>;

template <class DrawMode>
class IndexVector final : public IndexVectorBase {
public:
    static constexpr std::size_t groupSize = BufferGroupSizeOf<DrawMode>::value;

    template <class... Args>
    void emplace_back(Args&&... args) {
        static_assert(sizeof...(args) % groupSize == 0, "wrong buffer element count");
        requireRawData();
        assert(!released);
        util::ignore({(v.emplace_back(std::forward<Args>(args)), 0)...});
        dirty = true;
    }
};

using IndexVectorBasePtr = std::shared_ptr<IndexVectorBase>;

} // namespace gfx
} // namespace mln
