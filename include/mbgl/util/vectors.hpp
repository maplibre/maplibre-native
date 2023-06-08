#pragma once

#include <array>

namespace mbgl {

using vec2 = std::array<double, 2>;
using vec3 = std::array<double, 3>;
using vec3f = std::array<float, 3>;
using vec3i = std::array<int, 3>;
using vec4 = std::array<double, 4>;

template <typename Type, std::size_t... sizes>
auto concatenate(const std::array<Type, sizes>&... arrays)
{
    std::array<Type, (sizes + ...)> result;
    std::size_t index{};
 
    ((std::copy_n(arrays.begin(), sizes, result.begin() + index), index += sizes), ...);
 
    return result;
}

} // namespace mbgl
