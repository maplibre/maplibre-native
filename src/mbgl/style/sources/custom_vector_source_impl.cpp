#include <mbgl/style/sources/custom_vector_source_impl.hpp>

namespace mbgl {
namespace style {

CustomVectorSource::Impl::Impl(std::string id_, const CustomVectorSource::Options& options)
    : Source::Impl(SourceType::CustomMVTVector, std::move(id_)),
      zoomRange(options.zoomRange),
      loaderRef({}) {}

CustomVectorSource::Impl::Impl(const Impl& impl, const ActorRef<CustomVectorTileLoader>& loaderRef_)
    : Source::Impl(impl),
      zoomRange(impl.zoomRange),
      loaderRef(loaderRef_) {}

bool CustomVectorSource::Impl::operator!=(const Impl& other) const noexcept {
    return zoomRange != other.zoomRange || bool(loaderRef) != bool(other.loaderRef);
}

std::optional<std::string> CustomVectorSource::Impl::getAttribution() const {
    return {};
}

Range<uint8_t> CustomVectorSource::Impl::getZoomRange() const {
    return zoomRange;
}

std::optional<ActorRef<CustomVectorTileLoader>> CustomVectorSource::Impl::getTileLoader() const {
    return loaderRef;
}

} // namespace style
} // namespace mbgl
