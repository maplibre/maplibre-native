#pragma once

#include <optional>
#include <algorithm>

namespace mbgl {

template <class T>
class PaintPropertyStatistics {
public:
    std::optional<T> max() const { return std::nullopt; }
    void add(const T&) {}
};

template <>
class PaintPropertyStatistics<float> {
public:
    std::optional<float> max() const { return _max; }

    void add(float value) { _max = _max ? std::max(*_max, value) : value; }

private:
    std::optional<float> _max;
};

} // namespace mbgl
