#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/tileset.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/style/sources/tile_source.hpp>
#include <mbgl/style/sources/tile_source_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/mapbox.hpp>

namespace mbgl {
namespace style {

TileSource::TileSource(std::string id,
                       variant<std::string, Tileset> urlOrTileset_,
                       uint16_t tileSize,
                       SourceType sourceType)
    : Source(makeMutable<Impl>(sourceType, std::move(id), tileSize)),
      urlOrTileset(std::move(urlOrTileset_)) {}

TileSource::~TileSource() = default;

const TileSource::Impl& TileSource::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

const variant<std::string, Tileset>& TileSource::getURLOrTileset() const {
    return urlOrTileset;
}

std::optional<std::string> TileSource::getURL() const {
    if (urlOrTileset.is<Tileset>()) {
        return {};
    }

    return urlOrTileset.get<std::string>();
}

uint16_t TileSource::getTileSize() const {
    return impl().getTileSize();
}

void TileSource::loadDescription(FileSource& fileSource) {
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

            this->setTilesetOverrides(*tileset);

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

void TileSource::setTilesetOverrides(Tileset&) {
    // Default empty implementation, subclasses can override
}

Mutable<Source::Impl> TileSource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

} // namespace style
} // namespace mbgl
