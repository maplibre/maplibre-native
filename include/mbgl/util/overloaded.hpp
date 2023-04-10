#pragma once

namespace mbgl
{
namespace util
{
    
template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
Overload(Ts...) -> Overload<Ts...>;

} // namespace util
} // namespace mbgl
