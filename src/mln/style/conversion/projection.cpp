#include <mln/style/conversion/projection.hpp>
#include <mln/style/conversion_impl.hpp>

namespace mln {
namespace style {
namespace conversion {

std::optional<Projection> Converter<Projection>::operator()(const Convertible& value, Error& error) const {
    if (!isObject(value)) {
        error.message = "projection must be an object";
        return std::nullopt;
    }

    Projection projection;

    if (const auto type = objectMember(value, "type")) {
        if (auto typeError = projection.setProperty("type", *type)) {
            error = std::move(*typeError);
            return std::nullopt;
        }
    }

    return projection;
}

} // namespace conversion
} // namespace style
} // namespace mln
