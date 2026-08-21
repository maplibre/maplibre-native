#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion/rotation.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mln {
namespace style {
namespace conversion {

std::optional<style::Rotation> Converter<style::Rotation>::operator()(const Convertible& value, Error& error) const {
    std::optional<double> converted = toDouble(value);
    if (!converted) {
        error.message = "value must be a number";
        return std::nullopt;
    }
    return {*converted};
}

} // namespace conversion
} // namespace style
} // namespace mln
