#pragma once

#include <mln/style/source_impl.hpp>
#include <mln/style/sources/image_source.hpp>
#include <mln/util/image.hpp>
#include <mln/util/geo.hpp>

#include <array>

namespace mln {

namespace style {

class ImageSource::Impl final : public Source::Impl {
public:
    Impl(std::string id, std::array<LatLng, 4> coords);
    Impl(const Impl& other, std::array<LatLng, 4> coords);
    Impl(const Impl& rhs, PremultipliedImage&& image);

    ~Impl() final;

    std::shared_ptr<PremultipliedImage> getImage() const;
    std::array<LatLng, 4> getCoordinates() const;

    std::optional<std::string> getAttribution() const final;

private:
    std::array<LatLng, 4> coords;
    std::shared_ptr<PremultipliedImage> image;
};

} // namespace style
} // namespace mln
