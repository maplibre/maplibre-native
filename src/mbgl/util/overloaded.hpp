#pragma once

namespace mbgl
{
namespace util
{
    
template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

} // namespace util
} // namespace mbgl
