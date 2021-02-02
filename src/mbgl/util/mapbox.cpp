#include <mbgl/util/mapbox.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/url.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/tile_server_options.hpp>

#include <stdexcept>
#include <vector>
#include <cstring>

namespace mbgl {
namespace util {
namespace mapbox {

bool isAliasedResource(const TileServerOptions& tileServerOptions, const std::string& url) {
    const auto& protocol = tileServerOptions.uriSchemeAlias();
    return url == protocol;
}

// TODO:PP remove
//static bool equals(const std::string& str, const URL::Segment& segment, const char* ref) {
//    return str.compare(segment.first, segment.second, ref) == 0;
//}

std::string normalizeSourceURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& accessToken) {
    if (!isAliasedResource(tileServerOptions, str)) {
        return str;
    }
    if (accessToken.empty()) {
        throw std::runtime_error(
            "You must provide a Mapbox API access token for Mapbox tile sources");
    }

    const URL url(str);
    //TODO:PP
    //const auto tpl = baseURL + "/v4/{domain}.json?access_token=" + accessToken + "&secure";
    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.sourceTemplate() + "?" + tileServerOptions.accessTokenParameterName() +  "=" + accessToken;
    return transformURL(tpl, str, url);
}

std::string normalizeStyleURL(const TileServerOptions& tileServerOptions,
                              const std::string& str,
                              const std::string& accessToken) {
    if (!isAliasedResource(tileServerOptions, str)) {
        return str;
    }

    const URL url(str);
    //TODO:PP
    //const auto tpl = baseURL + "/styles/v1{path}?access_token=" + accessToken;
    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.styleTemplate() + "?" + tileServerOptions.accessTokenParameterName() +  "=" + accessToken;
    return transformURL(tpl, str, url);
}

std::string normalizeSpriteURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& accessToken) {
    if (!isAliasedResource(tileServerOptions, str)) {
        return str;
    }

    const URL url(str);
    //TODO:PP
//    const auto tpl =
//        baseURL + "/styles/v1{directory}{filename}/sprite{extension}?access_token=" + accessToken;
    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.spritesTemplate() + "?" + tileServerOptions.accessTokenParameterName() +  "=" + accessToken;
    return transformURL(tpl, str, url);
}

std::string normalizeGlyphsURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& accessToken) {
    if (!isAliasedResource(tileServerOptions, str)) {
        return str;
    }

    const URL url(str);
    //TODO:PP
    //const auto tpl = baseURL + "/fonts/v1{path}?access_token=" + accessToken;
    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.spritesTemplate() + "?" + tileServerOptions.glyphsTemplate() +  "=" + accessToken;
    return transformURL(tpl, str, url);
}

std::string normalizeTileURL(const TileServerOptions& tileServerOptions,
                             const std::string& str,
                             const std::string& accessToken) {
    if (!isAliasedResource(tileServerOptions, str)) {
        return str;
    }

    const URL url(str);
    //TODO:PP
    //const auto tpl = baseURL + "/v4{path}?access_token=" + accessToken;
    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.spritesTemplate() + "?" + tileServerOptions.glyphsTemplate() +  "=" + accessToken;
    return transformURL(tpl, str, url);
}

//TODO:PP remove?
std::string
canonicalizeTileURL(const TileServerOptions& tileServerOptions, const std::string& str, const style::SourceType type, const uint16_t tileSize) {
    const char* version = "/v4/";
    const size_t versionLen = strlen(version);

    const URL url(str);
    const Path path(str, url.path.first, url.path.second);

    // Make sure that we are dealing with a valid Mapbox tile URL.
    // Has to be /v4/, with a valid filename + extension
    if (str.compare(url.path.first, versionLen, version) != 0 || path.filename.second == 0 ||
        path.extension.second <= 1) {
        // Not a proper Mapbox tile URL.
        return str;
    }

    // Reassemble the canonical URL from the parts we've parsed before.
    std::string result = "mapbox://tiles/";
    result.append(str, path.directory.first + versionLen, path.directory.second - versionLen);
    result.append(str, path.filename.first, path.filename.second);
    if (type == style::SourceType::Raster || type == style::SourceType::RasterDEM) {
        result += tileSize == util::tileSize ? "@2x" : "{ratio}";
    }
    result.append(str, path.extension.first, path.extension.second);

    // Append the query string, minus the access token parameter.
    if (url.query.second > 1) {
        auto idx = url.query.first;
        bool hasQuery = false;
        while (idx != std::string::npos) {
            idx++; // skip & or ?
            auto ampersandIdx = str.find('&', idx);
            const char* accessToken = "access_token=";
            if (str.compare(idx, strlen(accessToken), accessToken) != 0) {
                result.append(1, hasQuery ? '&' : '?');
                result.append(str, idx, ampersandIdx != std::string::npos ? ampersandIdx - idx
                                                                          : std::string::npos);
                hasQuery = true;
            }
            idx = ampersandIdx;
        }
    }

    return result;
}

void canonicalizeTileset(const TileServerOptions& tileServerOptions, Tileset& tileset,
                         const std::string& sourceURL, style::SourceType type, uint16_t tileSize) {
    // TODO: Remove this hack by delivering proper URLs in the TileJSON to begin with.
    if (isAliasedResource(tileServerOptions, sourceURL)) {
        for (auto& url : tileset.tiles) {
            url = canonicalizeTileURL(url, type, tileSize);
        }
    }
}

const uint64_t DEFAULT_OFFLINE_TILE_COUNT_LIMIT = 6000;

} // end namespace mapbox
} // end namespace util
} // end namespace mbgl
