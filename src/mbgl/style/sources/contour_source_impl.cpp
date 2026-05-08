#include <mbgl/style/sources/contour_source_impl.hpp>

namespace mbgl {
namespace style {

ContourSource::Impl::Impl(std::string id_, ContourSourceOptions options_)
    : Source::Impl(SourceType::Contour, std::move(id_)),
      options(std::move(options_)) {}

ContourSource::Impl::~Impl() = default;

std::optional<std::string> ContourSource::Impl::getAttribution() const {
    // Contour features are derived from an upstream `raster-dem` source;
    // attribution flows through the upstream source, not this one.
    return {};
}

} // namespace style
} // namespace mbgl
