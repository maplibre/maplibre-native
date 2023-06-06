#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/tileset.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/style/sources/raster_source_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/mapbox.hpp>

namespace mbgl {
namespace style {

RasterSource::RasterSource(std::string id,
                           variant<std::string, Tileset> urlOrTileset_,
                           uint16_t tileSize,
                           SourceType sourceType)
    : Source(makeMutable<Impl>(sourceType, std::move(id), tileSize)),
      urlOrTileset(std::move(urlOrTileset_)) {}

RasterSource::~RasterSource() = default;

const RasterSource::Impl& RasterSource::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

const variant<std::string, Tileset>& RasterSource::getURLOrTileset() const {
    return urlOrTileset;
}

std::optional<std::string> RasterSource::getURL() const {
    if (urlOrTileset.is<Tileset>()) {
        return {};
    }

    return urlOrTileset.get<std::string>();
}

uint16_t RasterSource::getTileSize() const {
    return impl().getTileSize();
}

void RasterSource::loadDescription(FileSource& fileSource) {
    if (urlOrTileset.is<Tileset>()) {
        baseImpl = makeMutable<Impl>(impl(), urlOrTileset.get<Tileset>());
        loaded = true;
        observer->onSourceLoaded(*this);
        return;
    }

    if (req) {
        return;
    }

    const auto& rawURL = urlOrTileset.get<std::string>();
    const auto& url = util::mapbox::canonicalizeSourceURL(fileSource.getResourceOptions().tileServerOptions(), rawURL);

    req = fileSource.request(Resource::source(url), [this, url, &fileSource](const Response& res) {
        if (res.error) {
            observer->onSourceError(*this, std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            observer->onSourceError(*this, std::make_exception_ptr(std::runtime_error("unexpectedly empty TileJSON")));
        } else {
            conversion::Error error;
            std::optional<Tileset> tileset = conversion::convertJSON<Tileset>(*res.data, error);
            if (!tileset) {
                observer->onSourceError(*this, std::make_exception_ptr(util::StyleParseException(error.message)));
                return;
            }
            const auto& tileServerOptions = fileSource.getResourceOptions().tileServerOptions();
            if (tileServerOptions.uriSchemeAlias() == "mapbox") {
                util::mapbox::canonicalizeTileset(tileServerOptions, *tileset, url, getType(), getTileSize());
            }
            bool changed = impl().tileset != *tileset;

            baseImpl = makeMutable<Impl>(impl(), *tileset);
            loaded = true;

            observer->onSourceLoaded(*this);

            if (changed) {
                observer->onSourceChanged(*this);
            }
        }
    });
}

bool RasterSource::supportsLayerType(const mbgl::style::LayerTypeInfo* info) const {
    return mbgl::underlying_type(Tile::Kind::Raster) == mbgl::underlying_type(info->tileKind);
}

Mutable<Source::Impl> RasterSource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

} // namespace style
} // namespace mbgl
