#include <mln/storage/file_source.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/sources/image_source.hpp>
#include <mln/style/sources/image_source_impl.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/async_request.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/premultiply.hpp>

namespace mln {
namespace style {

ImageSource::ImageSource(std::string id, const std::array<LatLng, 4> coords_)
    : Source(makeMutable<Impl>(std::move(id), coords_)) {}

ImageSource::~ImageSource() = default;

const ImageSource::Impl& ImageSource::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

void ImageSource::setCoordinates(const std::array<LatLng, 4>& coords_) {
    baseImpl = makeMutable<Impl>(impl(), coords_);
    observer->onSourceChanged(*this);
}

std::array<LatLng, 4> ImageSource::getCoordinates() const {
    return impl().getCoordinates();
}

void ImageSource::setURL(const std::string& url_) {
    url = url_;
    // Signal that the source description needs a reload
    if (loaded || req) {
        loaded = false;
        req.reset();
        observer->onSourceDescriptionChanged(*this);
    }
}

void ImageSource::setImage(PremultipliedImage&& image_) {
    url = {};
    if (req) {
        req.reset();
    }
    loaded = true;
    baseImpl = makeMutable<Impl>(impl(), std::move(image_));
    observer->onSourceChanged(*this);
}

std::optional<std::string> ImageSource::getURL() const {
    return url;
}

void ImageSource::loadDescription(FileSource& fileSource) {
    if (!url) {
        loaded = true;
    }

    if (req || loaded) {
        return;
    }
    const Resource imageResource{Resource::Image, *url, {}};

    req = fileSource.request(imageResource, [this](const Response& res) {
        if (res.error) {
            observer->onSourceError(*this, std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            observer->onSourceError(*this, std::make_exception_ptr(std::runtime_error("unexpectedly empty image url")));
        } else {
            try {
                baseImpl = makeMutable<Impl>(impl(), decodeImage(*res.data));
            } catch (...) {
                observer->onSourceError(*this, std::current_exception());
            }
            loaded = true;
            observer->onSourceLoaded(*this);
        }
    });
}

bool ImageSource::supportsLayerType(const mln::style::LayerTypeInfo* info) const {
    return mln::underlying_type(Tile::Kind::Raster) == mln::underlying_type(info->tileKind);
}

Mutable<Source::Impl> ImageSource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

} // namespace style
} // namespace mln
