#include <mbgl/style/sources/contour_source.hpp>
#include <mbgl/style/sources/contour_source_impl.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/tile/tile.hpp>

namespace mbgl {
namespace style {

ContourSource::ContourSource(std::string id, ContourSourceOptions options)
    : Source(makeMutable<Impl>(std::move(id), std::move(options))) {}

ContourSource::~ContourSource() = default;

const ContourSource::Impl& ContourSource::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

const std::string& ContourSource::getDEMSourceID() const {
    return impl().getOptions().sourceID;
}

const algorithm::contour::IntervalSchedule& ContourSource::getIntervals() const {
    return impl().getOptions().intervals;
}

const algorithm::contour::UnitConfig& ContourSource::getUnit() const {
    return impl().getOptions().unit;
}

const algorithm::contour::IntervalSchedule& ContourSource::getMajorMultiplier() const {
    return impl().getOptions().majorMultiplier;
}

std::uint8_t ContourSource::getOverzoom() const {
    return impl().getOptions().overzoom;
}

void ContourSource::loadDescription(FileSource&) {
    // No tiles to fetch — the source-on-source pipeline reads decoded DEM
    // tiles from the upstream raster-dem source's tile pyramid.
    loaded = true;
}

bool ContourSource::supportsLayerType(const mbgl::style::LayerTypeInfo* info) const {
    // Contour features are emitted as line-layer-shaped vector geometry.
    return mbgl::underlying_type(Tile::Kind::Geometry) == mbgl::underlying_type(info->tileKind);
}

Mutable<Source::Impl> ContourSource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

} // namespace style
} // namespace mbgl
