#include <mln/style/conversion/sprite.hpp>
#include <mln/style/conversion/constant.hpp>
#include <mln/style/conversion_impl.hpp>

#include <array>

namespace mln {
namespace style {
namespace conversion {

std::optional<Sprite> Converter<Sprite>::operator()(const Convertible& value, Error& error) const {
    std::optional<std::string> id;
    auto idValue = objectMember(value, "id");
    if (!idValue) {
        error.message = "id must be defined for sprite object";
        return std::nullopt;
    }
    id = toString(*idValue);

    std::optional<std::string> spriteURL;
    auto urlValue = objectMember(value, "url");
    if (!urlValue) {
        error.message = "url must be defined for sprite object";
        return std::nullopt;
    }
    spriteURL = toString(*urlValue);

    return Sprite(*id, *spriteURL);
}

} // namespace conversion
} // namespace style
} // namespace mln
