#pragma once

#include <mln/util/ignore.hpp>
#include <mln/util/monotonic_timer.hpp>

#include <cassert>
#include <memory>
#include <vector>

namespace mln {
namespace gfx {

struct VertexBufferBase {
    virtual ~VertexBufferBase() = default;
};

class VertexVectorBase {
public:
    VertexVectorBase() = default;
    VertexVectorBase(const VertexVectorBase&) {} // buffer is not copied
    VertexVectorBase(VertexVectorBase&& other)
        : buffer(std::move(other.buffer)),
          dirty(other.dirty),
          released(other.released),
          rawFreed(other.rawFreed) {}
    virtual ~VertexVectorBase() = default;

    virtual const void* getRawData() const = 0;
    virtual std::size_t getRawSize() const = 0;
    virtual std::size_t getRawCount() const = 0;

    VertexBufferBase* getBuffer() const { return buffer.get(); }
    void setBuffer(std::unique_ptr<VertexBufferBase>&& value) {
        buffer = std::move(value);
        // The data now lives in the buffer; a released vector will not be modified or re-uploaded.
        if (released && buffer) {
            freeRawData();
        }
    }

    /// False once the CPU copy has been freed after upload. The element count is kept.
    bool hasRawData() const { return !rawFreed; }

    std::chrono::duration<double> getLastModified() const { return lastModified; }
    bool isModifiedAfter(std::chrono::duration<double> t) const { return !rawFreed && t < lastModified; }

    void updateModified(bool force = false) {
        if (dirty || force) {
            lastModified = util::MonotonicTimer::now();
            dirty = false;
        }
    }

    // Indicates that the owner/producer will not modify this again
    bool isReleased() const { return released; }

protected:
    /// Drop the CPU copy of the data. Vectors that keep their data (the plugin vectors) leave it be.
    virtual void freeRawData() {}

    std::unique_ptr<VertexBufferBase> buffer;
    bool dirty = true;
    bool released = false;
    bool rawFreed = false;

    std::chrono::duration<double> lastModified = util::MonotonicTimer::now();
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
        : VertexVectorBase(static_cast<VertexVectorBase&&>(other)),
          v(std::move(other.v)),
          freedCount(other.freedCount) {}
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

    std::size_t elements() const { return v.size() + freedCount; }

    std::size_t bytes() const { return elements() * sizeof(Vertex); }

    bool empty() const { return elements() == 0; }

    void clear() {
        dirty = true;
        v.clear();
        freedCount = 0;
        rawFreed = false;
    }

    void reserve(std::size_t count) { v.reserve(count); }

    /// Indicate that this shared vertex vector instance will no longer be updated.
    void release() {
        released = true;
        // If we've already created a buffer, we don't need the raw data any more.
        if (buffer) {
            freeRawData();
        }
    }

    const Vertex* data() const { return v.data(); }

    const std::vector<Vertex>& vector() const { return v; }

    const void* getRawData() const override { return v.data(); }
    std::size_t getRawSize() const override { return sizeof(Vertex); }
    std::size_t getRawCount() const override { return elements(); }

protected:
    void freeRawData() override {
        freedCount += v.size();
        std::vector<Vertex>().swap(v);
        rawFreed = true;
    }

private:
    std::vector<Vertex> v;
    std::size_t freedCount = 0;
};

template <typename T>
using VertexVectorPtr = std::shared_ptr<VertexVector<T>>;

} // namespace gfx
} // namespace mln
