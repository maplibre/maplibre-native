#pragma once

#include <mbgl/util/ignore.hpp>

#include <vector>

namespace mbgl {
namespace gfx {

struct VertexVectorBase {
    virtual const void* getRawData() const = 0;
    virtual std::size_t getRawSize() const = 0;
    virtual std::size_t getRawCount() const = 0;
#if !NDEBUG
    bool locked = false;
#endif
};

template <class V>
class VertexVector : public VertexVectorBase {
public:
    using Vertex = V;
    template <typename Arg>
    void emplace_back(Arg&& vertex) {
        assert(!locked);
        v.emplace_back(std::forward<Arg>(vertex));
    }

    void extend(std::size_t n, const Vertex& val) {
        assert(!locked);
        v.resize(v.size() + n, val);
    }

    Vertex& at(std::size_t n) {
        assert(n < v.size());
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
        assert(!locked);
        v.clear();
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
