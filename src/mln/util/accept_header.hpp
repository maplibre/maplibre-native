#pragma once

#include <mln/util/image.hpp>

#include <array>
#include <cstddef>
#include <string_view>

namespace mln {
namespace http {

/// MIME types of the tile formats we know how to request.
inline constexpr std::string_view MIME_TYPE_MVT = "application/vnd.mapbox-vector-tile";
inline constexpr std::string_view MIME_TYPE_MLT = "application/vnd.maplibre-tile";
inline constexpr std::string_view MIME_TYPE_WEBP = "image/webp";
inline constexpr std::string_view MIME_TYPE_JPEG = "image/jpeg";
inline constexpr std::string_view MIME_TYPE_PNG = "image/png";

namespace detail {

/// Null terminated backing storage for `acceptHeader`.
template <const std::string_view& first, const std::string_view&... rest>
inline constexpr auto acceptHeaderStorage = [] {
    std::array<char, first.size() + (rest.size() + ... + 0) + 2 * sizeof...(rest) + 1> buffer{};
    std::size_t offset = 0;
    const auto append = [&](std::string_view type) {
        for (const char c : type) {
            buffer[offset++] = c;
        }
    };
    append(first);
    ((append(", "), append(rest)), ...);
    return buffer;
}();

} // namespace detail

/// The value of an HTTP `Accept` header offering the given MIME types, most preferred first.
/// Assembled at compile time and null terminated, so it can be handed to C APIs as well.
template <const std::string_view& first, const std::string_view&... rest>
inline constexpr std::string_view acceptHeader{detail::acceptHeaderStorage<first, rest...>.data(),
                                               detail::acceptHeaderStorage<first, rest...>.size() - 1};

/// The `Accept` header for raster tile requests, limited to what this platform can decode.
inline std::string_view rasterAcceptHeader() {
    return supportsWebPDecoding() ? acceptHeader<MIME_TYPE_WEBP, MIME_TYPE_JPEG, MIME_TYPE_PNG>
                                  : acceptHeader<MIME_TYPE_JPEG, MIME_TYPE_PNG>;
}

} // namespace http
} // namespace mln
