#include <mln/storage/resource.hpp>
#include <mln/util/string.hpp>
#include <mln/util/token.hpp>
#include <mln/util/url.hpp>

#include <cmath>

namespace mln {

namespace {
std::string getQuadKey(int32_t x, int32_t y, int8_t z) {
    std::string quadKey;
    quadKey.reserve(z);
    int32_t mask;
    for (int8_t i = z; i > 0; i--) {
        mask = 1 << (i - 1);
        quadKey += static_cast<char>('0' + ((x & mask ? 1 : 0) + (y & mask ? 2 : 0)));
    }
    return quadKey;
}

// The box of the tile in row `y` from the top, in the tile matrix set's units, y up.
std::string getTileBBox(int32_t x, int32_t y, int8_t z, const TileMatrixSet& tileMatrixSet) {
    const double tilesAtZoom = std::pow(2, z);
    const double tileExtent = tileMatrixSet.extentAtZoom0 / tilesAtZoom;
    const double gridMinX = tileMatrixSet.origin[0];
    const double gridMinY = tileMatrixSet.origin[1] - tileMatrixSet.extentAtZoom0;
    const double rowFromBottom = tilesAtZoom - y - 1;
    return util::toString(gridMinX + x * tileExtent) + "," + util::toString(gridMinY + rowFromBottom * tileExtent) +
           "," + util::toString(gridMinX + (x + 1) * tileExtent) + "," +
           util::toString(gridMinY + (rowFromBottom + 1) * tileExtent);
}
} // namespace

Resource Resource::style(const std::string& url) {
    return Resource{Resource::Kind::Style, url};
}

Resource Resource::source(const std::string& url) {
    return Resource{Resource::Kind::Source, url};
}

Resource Resource::image(const std::string& url) {
    return Resource{Resource::Kind::Image, url};
}

Resource Resource::spriteImage(const std::string& base, float pixelRatio) {
    util::URL url(base);
    return Resource{Resource::Kind::SpriteImage,
                    base.substr(0, url.path.first + url.path.second) + (pixelRatio > 1 ? "@2x" : "") + ".png" +
                        base.substr(url.query.first, url.query.second)};
}

Resource Resource::spriteJSON(const std::string& base, float pixelRatio) {
    util::URL url(base);
    return Resource{Resource::Kind::SpriteJSON,
                    base.substr(0, url.path.first + url.path.second) + (pixelRatio > 1 ? "@2x" : "") + ".json" +
                        base.substr(url.query.first, url.query.second)};
}

Resource Resource::glyphs(const std::string& urlTemplate,
                          const FontStack& fontStack,
                          const std::pair<uint16_t, uint16_t>& glyphRange) {
    return Resource{Resource::Kind::Glyphs,
                    util::replaceTokens(urlTemplate, [&](const std::string& token) -> std::optional<std::string> {
                        if (token == "fontstack") {
                            return util::percentEncode(fontStackToString(fontStack));
                        } else if (token == "range") {
                            return util::toString(glyphRange.first) + "-" + util::toString(glyphRange.second);
                        } else {
                            return {};
                        }
                    })};
}

Resource Resource::fontFace(const std::string& url) {
    return Resource{Resource::Kind::Glyphs, url};
}

Resource Resource::tile(const std::string& urlTemplate,
                        float pixelRatio,
                        int32_t x,
                        int32_t y,
                        int8_t z,
                        Tileset::Scheme scheme,
                        LoadingMethod loadingMethod,
                        const TileMatrixSet& tileMatrixSet) {
    bool supportsRatio = urlTemplate.find("{ratio}") != std::string::npos;
    // The scheme flips `{y}`, as in GL JS; `{bbox}` is the box of the tile itself.
    const int32_t tileY = y;
    if (scheme == Tileset::Scheme::TMS) {
        y = (1 << z) - y - 1;
    }
    return Resource{Resource::Kind::Tile,
                    util::replaceTokens(urlTemplate,
                                        [&](const std::string& token) -> std::optional<std::string> {
                                            if (token == "z") {
                                                return util::toString(z);
                                            } else if (token == "x") {
                                                return util::toString(x);
                                            } else if (token == "y") {
                                                return util::toString(y);
                                            } else if (token == "quadkey") {
                                                return getQuadKey(x, y, z);
                                            } else if (token == "bbox-epsg-3857") {
                                                return getTileBBox(x, y, z, mercatorTileMatrixSet);
                                            } else if (token == "bbox") {
                                                return getTileBBox(x, tileY, z, tileMatrixSet);
                                            } else if (token == "prefix") {
                                                return {{
                                                    "0123456789abcdef"[x % 16],
                                                    "0123456789abcdef"[y % 16],
                                                }};
                                            } else if (token == "ratio") {
                                                return std::string(pixelRatio > 1.0 ? "@2x" : "");
                                            } else {
                                                return {};
                                            }
                                        }),
                    Resource::TileData{.urlTemplate = urlTemplate,
                                       .pixelRatio = uint8_t(supportsRatio && pixelRatio > 1.0 ? 2 : 1),
                                       .x = x,
                                       .y = y,
                                       .z = z},
                    loadingMethod};
}

} // namespace mln
