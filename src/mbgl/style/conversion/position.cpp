#include <mbgl/style/conversion/position.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>

#include <array>

namespace mbgl {
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
} // namespace mbgl
