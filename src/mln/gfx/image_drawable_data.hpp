#pragma once

#include <mln/gfx/drawable_data.hpp>

#include <memory>
#include <vector>

namespace mln {

namespace gfx {

class ImageDrawableData : public DrawableData {
public:
    ImageDrawableData(std::vector<UnwrappedTileID> cover_)
        : cover(std::move(cover_)) {}

    std::span<const UnwrappedTileID> drapeCover() const override { return cover; }

    /// The tiles the image covers at the zoom of the drawable's own tile
    std::vector<UnwrappedTileID> cover;
};

using UniqueImageDrawableData = std::unique_ptr<ImageDrawableData>;

} // namespace gfx
} // namespace mln
