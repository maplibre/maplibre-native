#include <mln/style/conversion/position.hpp>
#include <mln/style/conversion/constant.hpp>
#include <mln/style/conversion_impl.hpp>

#include <array>

namespace mln {
namespace style {
namespace conversion {

std::optional<Position> Converter<Position>::operator()(const Convertible& value, Error& error) const {
    std::optional<std::array<float, 3>> spherical = convert<std::array<float, 3>>(value, error);

    if (!spherical) {
        return std::nullopt;
    }

    return Position(*spherical);
}

} // namespace conversion
} // namespace style
} // namespace mln
