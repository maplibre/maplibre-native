#include "url_or_tileset.hpp"
#include "../android_conversion.hpp"

#include <mln/style/conversion.hpp>
#include <mln/style/conversion/tileset.hpp>

namespace mln {
namespace android {

// This conversion is expected not to fail because it's used only in contexts where
// the value was originally a String or TileSet object on the Java side. If it fails
// to convert, it's a bug in our serialization or Java-side static typing.
variant<std::string, Tileset> convertURLOrTileset(mln::android::Value&& value) {
    using namespace mln::style::conversion;

    const Convertible convertible(std::move(value));
    if (isObject(convertible)) {
        Error error;
        std::optional<Tileset> tileset = convert<Tileset>(convertible, error);
        if (!tileset) {
            throw std::logic_error(error.message);
        }
        return {*tileset};
    } else {
        return {*toString(convertible)};
    }
}

} // namespace android
} // namespace mln
