#pragma once

#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/style/custom_vector_tile_loader.hpp>
#include <mbgl/actor/actor_ref.hpp>

namespace mbgl {
namespace style {

class CustomVectorSource::Impl : public Source::Impl {
public:
    Impl(std::string id, const CustomVectorSource::Options& options);
    Impl(const Impl&, const ActorRef<CustomVectorTileLoader>&);

    std::optional<std::string> getAttribution() const final;

    Range<uint8_t> getZoomRange() const;
    std::optional<ActorRef<CustomVectorTileLoader>> getTileLoader() const;
    bool operator!=(const Impl&) const noexcept;

private:
    Range<uint8_t> zoomRange;
    std::optional<ActorRef<CustomVectorTileLoader>> loaderRef;
};

} // namespace style
} // namespace mbgl
