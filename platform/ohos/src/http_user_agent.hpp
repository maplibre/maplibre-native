#pragma once

#include <mbgl/util/client_options.hpp>
#include <mbgl/util/version.hpp>

#include <string>

namespace mbgl {
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

    userAgent += "MapLibreNative/0.0.0 (";
    userAgent += version::revision;
    userAgent += "; HarmonyOS)";
    return userAgent;
}

} // namespace ohos
} // namespace mbgl
