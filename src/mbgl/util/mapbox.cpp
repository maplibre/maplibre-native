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

inline bool endsWith(const std::string& str, const std::string& search) {
    if (search.size() > str.size()) return false;
    return std::equal(search.rbegin(), search.rend(), str.rbegin());
}

inline int nthOccurrenceFromEnd(const std::string& str, const std::string& search, int nth) {
    size_t pos = str.length();
    int cnt = 0;

    while (cnt != nth){
        pos-=1;
        pos = str.rfind(search, pos);
        if ( pos == std::string::npos )
            return -1;
        cnt++;
    }
    return static_cast<int>(pos);
}

inline void replace(std::string& str, const std::string& search, const std::string& replacement) {
  std::string::size_type pos = 0u;
  while((pos = str.find(search, pos)) != std::string::npos){
     str.replace(pos, search.length(), replacement);
     pos += replacement.length();
  }
}

bool isCanonicalURL(const TileServerOptions& tileServerOptions, const std::string& url) {
    const auto& protocol = tileServerOptions.uriSchemeAlias() + "://";
    return url.compare(0, protocol.length(), protocol) == 0;
}

//std::string baseUrlWithoutSubdomains(const std::string& baseUrl) {
//    const URL url(baseUrl);
//    const auto domainAndScheme = baseUrl.substr(0, url.domain.first + url.domain.second);
//    const auto lastSubdomainIdx = nthOccurrenceFromEnd(domainAndScheme, ".", 2);
//    auto str = baseUrl.substr(lastSubdomainIdx + 1, baseUrl.length() - 1);
//    return str;
//}

bool isNormalizedURL(const TileServerOptions& tileServerOptions, const std::string& str) {
    const URL url(str);
    const Path path(str, url.path.first, url.path.second);

    // Make sure that we are dealing with a valid tile URL.
    // Has to be with the tile template prefix, with a valid filename + extension
    
    if (tileServerOptions.uriSchemeAlias() == "mapbox") {
        const URL baseURL(tileServerOptions.baseURL());
        auto domain = str.substr(url.domain.first, url.domain.second);
    
        replace(domain, ".cn", ".com"); // hack for mapbox china
    
        auto refDomain = tileServerOptions.baseURL().substr(baseURL.domain.first, baseURL.domain.second);
        const auto refDomainIdx = nthOccurrenceFromEnd(refDomain, ".", 2);
        refDomain = refDomain.substr(refDomainIdx + 1, refDomain.length() - refDomainIdx - 1);
        
        if (!endsWith(domain, refDomain) || path.filename.second == 0 || path.extension.second <= 1) {
            // Not a proper tile URL.
            return false;
        }
    }
    else {
        if (path.filename.second == 0 || path.extension.second <= 1) {
            // Not a proper tile URL.
            return false;
        }
    }
    
    return true;
}

std::string makeQueryString(const TileServerOptions& tileServerOptions, const std::string& apiKey) {
    const std::string queryString = (!tileServerOptions.requiresApiKey() || tileServerOptions.apiKeyParameterName().empty()) ?
        "" : "?" + tileServerOptions.apiKeyParameterName() +  "=" + apiKey;
    return queryString;
}

static bool equals(const std::string& str, const URL::Segment& segment, std::string& ref) {
    return str.compare(segment.first, segment.second, ref) == 0;
}

std::string normalizeSourceURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& apiKey) {
    if (!isCanonicalURL(tileServerOptions, str)) {
        return str;
    }
    if (tileServerOptions.requiresApiKey() && apiKey.empty()) {
        throw std::runtime_error(
            "You must provide API key for tile sources");
    }

    const URL url(str);
    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.sourceVersionPrefix().value_or("") + tileServerOptions.sourceTemplate() + makeQueryString(tileServerOptions, apiKey) + "&secure";
    return transformURL(tpl, str, url);
}

std::string normalizeStyleURL(const TileServerOptions& tileServerOptions,
                              const std::string& str,
                              const std::string& apiKey) {
    if (!isCanonicalURL(tileServerOptions, str)) {
        return str;
    }

    const URL url(str);
    auto domainName = tileServerOptions.styleDomainName();
    if (!equals(str, url.domain, domainName)) {
        Log::Error(Event::ParseStyle, "Invalid style URL");
        return str;
    }
    
    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.styleVersionPrefix().value_or("") + tileServerOptions.styleTemplate() + makeQueryString(tileServerOptions, apiKey);

        
    return transformURL(tpl, str, url);
}

std::string normalizeSpriteURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& apiKey) {
    if (!isCanonicalURL(tileServerOptions, str)) {
        return str;
    }

    const URL url(str);
    auto domainName = tileServerOptions.spritesDomainName();
    if (!equals(str, url.domain, domainName)) {
        Log::Error(Event::ParseStyle, "Invalid sprite URL");
        return str;
    }

    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.spritesVersionPrefix().value_or("") + tileServerOptions.spritesTemplate() + makeQueryString(tileServerOptions, apiKey);
    return transformURL(tpl, str, url);
}

std::string normalizeGlyphsURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& apiKey) {
    if (!isCanonicalURL(tileServerOptions, str)) {
        return str;
    }

    const URL url(str);
    auto domainName = tileServerOptions.glyphsDomainName();
    if (!equals(str, url.domain, domainName)) {
        Log::Error(Event::ParseStyle, "Invalid glyph URL");
        return str;
    }

    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.glyphsVersionPrefix().value_or("") + tileServerOptions.glyphsTemplate() + makeQueryString(tileServerOptions, apiKey);
    return transformURL(tpl, str, url);
}

std::string normalizeTileURL(const TileServerOptions& tileServerOptions,
                             const std::string& str,
                             const std::string& apiKey) {
    if (!isCanonicalURL(tileServerOptions, str)) {
        return str;
    }
    
    const URL url(str);
    auto domainName = tileServerOptions.tileDomainName();
    if (!equals(str, url.domain, domainName)) {
        Log::Error(Event::ParseStyle, "Invalid tile URL");
        return str;
    }

    const auto tpl = tileServerOptions.baseURL() + tileServerOptions.tileVersionPrefix().value_or("") + tileServerOptions.tileTemplate() + makeQueryString(tileServerOptions, apiKey);
    return transformURL(tpl, str, url);
}

std::string
canonicalizeTileURL(const TileServerOptions& tileServerOptions, const std::string& str, const style::SourceType type, const uint16_t tileSize) {

    if (!isNormalizedURL(tileServerOptions, str)){
        return str;
    }

    std::string result = tileServerOptions.uriSchemeAlias() + "://" + tileServerOptions.tileDomainName();
    
    const URL url(str);
    const Path path(str, url.path.first, url.path.second);
    
    int versionPrefixLen = 0;
    const auto& versionPrefix = tileServerOptions.tileVersionPrefix();
    if (versionPrefix.has_value()){
        versionPrefixLen = static_cast<int>(versionPrefix.value().length());
    }
    result.append(str, path.directory.first + versionPrefixLen, path.directory.second - versionPrefixLen);
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
            const auto& apiKey = tileServerOptions.apiKeyParameterName() + "=";
            if (str.compare(idx, apiKey.length(), apiKey) != 0) {
                result.append(1, hasQuery ? '&' : '?');
                result.append(str, idx, ampersandIdx != std::string::npos ? ampersandIdx - idx
                                                                          : std::string::npos);
                hasQuery = true;
            }
            idx = ampersandIdx;
        }
    }

    //mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf
    return result;
}

void canonicalizeTileset(const TileServerOptions& tileServerOptions, Tileset& tileset,
                         const std::string& sourceURL, style::SourceType type, uint16_t tileSize) {
    // TODO: Remove this hack by delivering proper URLs in the TileJSON to begin with.
    if (isCanonicalURL(tileServerOptions, sourceURL)) {
        for (auto& url : tileset.tiles) {
            url = canonicalizeTileURL(tileServerOptions, url, type, tileSize);
        }
    }
}

const uint64_t DEFAULT_OFFLINE_TILE_COUNT_LIMIT = 6000;

} // end namespace mapbox
} // end namespace util
} // end namespace mbgl
