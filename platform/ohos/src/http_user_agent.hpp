#pragma once

#include <mln/util/client_options.hpp>
#include <mln/util/version.hpp>

#include <string>

namespace mln {
namespace ohos {

inline std::string buildUserAgent(const ClientOptions& clientOptions) {
    std::string userAgent;
    if (!clientOptions.name().empty()) {
        userAgent += clientOptions.name();
        if (!clientOptions.version().empty()) {
            userAgent += "/";
            userAgent += clientOptions.version();
        }
        userAgent += " ";
    }

    userAgent += "MapLibreNative/";
    userAgent += version::revision;
    userAgent += " (HarmonyOS)";
    return userAgent;
}

} // namespace ohos
} // namespace mln
