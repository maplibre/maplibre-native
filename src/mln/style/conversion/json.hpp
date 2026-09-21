#pragma once

#include <mln/style/conversion_impl.hpp>
#include <mln/style/rapidjson_conversion.hpp>

#include <string>

namespace mln {
namespace style {
namespace conversion {

template <class T, class... Args>
std::optional<T> convertJSON(const std::string& json, Error& error, Args&&... args) {
    JSDocument document;
    document.Parse<0>(json.c_str());

    if (document.HasParseError()) {
        error = {formatJSONParseError(document)};
        return {};
    }

    return convert<T>(document, error, std::forward<Args>(args)...);
}

} // namespace conversion
} // namespace style
} // namespace mln
