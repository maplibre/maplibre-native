#include <mbgl/geometry/dem_data.hpp>
#include <mbgl/math/clamp.hpp>

namespace mbgl {

DEMData::DEMData(const PremultipliedImage& _image, Tileset::RasterEncoding _encoding)
    : dim(_image.size.height),
      // `border` extra pixels per side around the interior, used to backfill
      // the slow-path neighbour data the prepare pass / contour algorithm
      // needs at tile boundaries. See `DEMData::border` for the rationale.
      stride(dim + 2 * border),
      encoding(_encoding) {
    image = std::make_shared<PremultipliedImage>(Size(static_cast<uint32_t>(stride), static_cast<uint32_t>(stride)));
    if (_image.size.height != _image.size.width) {
        throw std::runtime_error("raster-dem tiles must be square.");
    }

    auto* dest = reinterpret_cast<uint32_t*>(image->data.get()) + stride * border + border;
    auto* source = reinterpret_cast<uint32_t*>(_image.data.get());
    for (int32_t y = 0; y < dim; y++) {
        memcpy(dest, source, dim * 4);
        dest += stride;
        source += dim;
    }

    // To avoid flashing seams between tiles, populate the `border`-wide
    // ring around the interior with the nearest-edge pixel value. This
    // gets overwritten with real neighbour data once the surrounding tiles
    // arrive and `backfillBorder` runs.

    auto* data = reinterpret_cast<uint32_t*>(image->data.get());
    for (int32_t y = 0; y < dim; y++) {
        auto rowOffset = stride * (y + border);
        for (int32_t b = 0; b < border; b++) {
            // left vertical border (columns 0..border-1)
            data[rowOffset + b] = data[rowOffset + border];
            // right vertical border (columns dim+border..stride-1)
            data[rowOffset + dim + border + b] = data[rowOffset + dim + border - 1];
        }
    }

    // Top + bottom horizontal borders, including the four corners. Each row
    // of the border ring is a copy of the nearest interior row.
    for (int32_t b = 0; b < border; b++) {
        // top
        memcpy(data + b * stride, data + border * stride, stride * 4);
        // bottom
        memcpy(data + (dim + border + b) * stride, data + (dim + border - 1) * stride, stride * 4);
    }
}

// This function takes the DEMData from a neighboring tile and backfills the
// `border`-wide ring of image data around the tile. Necessary because the
// hillshade formula calculates dx/dz, dy/dz derivatives at each pixel by
// querying its 8 neighbours, and without the buffered ring we get seams at
// tile boundaries.
//
// At overzoom (display zoom > raster-dem maxzoom) the prepare pass extends
// one pixel past the tile interior so adjacent tiles share an identical
// edge value. That requires `border` to be at least 2 — the outermost
// boundary-aligned output pixel on tile A reads into B's first two
// interior columns, and vice versa. With `border == 1` the boundary
// outputs would have to fall back to a different kernel and would no
// longer be exactly equal across the seam.
void DEMData::backfillBorder(const DEMData& borderTileData, int8_t dx, int8_t dy) {
    auto& o = borderTileData;

    // Tiles from the same source should always be of the same dimensions.
    assert(dim == o.dim);

    // We determine the pixel range to backfill based on which corner/edge
    // `borderTileData` represents. For an axis-aligned neighbour the range
    // is the full tile's worth on the matching axis and `border` pixels
    // wide on the orthogonal axis. For a diagonal neighbour the range is
    // a `border × border` square at the relevant corner.
    int32_t xMin = dx * dim;
    int32_t xMax = dx * dim + dim;
    int32_t yMin = dy * dim;
    int32_t yMax = dy * dim + dim;

    if (dx == -1)
        xMin = xMax - border;
    else if (dx == 1)
        xMax = xMin + border;

    if (dy == -1)
        yMin = yMax - border;
    else if (dy == 1)
        yMax = yMin + border;

    int32_t ox = -dx * dim;
    int32_t oy = -dy * dim;

    auto* dest = reinterpret_cast<uint32_t*>(image->data.get());
    auto* source = reinterpret_cast<uint32_t*>(o.image->data.get());

    for (int32_t y = yMin; y < yMax; y++) {
        for (int32_t x = xMin; x < xMax; x++) {
            dest[idx(x, y)] = source[idx(x + ox, y + oy)];
        }
    }
}

int32_t DEMData::get(const int32_t x, const int32_t y) const {
    const auto& unpack = getUnpackVector();
    const uint8_t* value = image->data.get() + idx(x, y) * 4;
    return static_cast<int32_t>(value[0] * unpack[0] + value[1] * unpack[1] + value[2] * unpack[2] - unpack[3]);
}

const std::array<float, 4>& DEMData::getUnpackVector() const {
    // https://www.mapbox.com/help/access-elevation-data/#mapbox-terrain-rgb
    static const std::array<float, 4> unpackMapbox = {{6553.6f, 25.6f, 0.1f, 10000.0f}};
    // https://aws.amazon.com/public-datasets/terrain/
    static const std::array<float, 4> unpackTerrarium = {{256.0f, 1.0f, 1.0f / 256.0f, 32768.0f}};

    return encoding == Tileset::RasterEncoding::Terrarium ? unpackTerrarium : unpackMapbox;
}

} // namespace mbgl
