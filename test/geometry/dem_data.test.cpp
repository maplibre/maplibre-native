#include <mbgl/test/util.hpp>

#include <mbgl/util/image.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/geometry/dem_data.hpp>

using namespace mbgl;

auto fakeImage = [](Size s) {
    PremultipliedImage img = PremultipliedImage(s);

    for (size_t i = 0; i < img.bytes(); i++) {
        img.data[i] = (i + 1) % 4 == 0 ? 1 : std::rand() % 255;
    }
    return img;
};

// DEMData uses a 2-pixel border on each side so the hillshade prepare pass
// can reach one pixel past the tile interior at the world boundary, which
// is what makes adjacent prepare outputs share an identical edge value at
// overzoom (no seam). The tests below pin both the storage layout and the
// border content.

TEST(DEMData, ConstructorMapbox) {
    PremultipliedImage image = fakeImage({16, 16});
    DEMData demdata(image, Tileset::RasterEncoding::Mapbox);

    EXPECT_EQ(demdata.dim, 16);
    EXPECT_EQ(demdata.stride, 16 + 2 * DEMData::border);
    EXPECT_EQ(demdata.getImage()->bytes(), size_t(demdata.stride * demdata.stride * 4));
};

TEST(DEMData, ConstructorTerrarium) {
    PremultipliedImage image = fakeImage({16, 16});
    DEMData demdata(image, Tileset::RasterEncoding::Terrarium);

    EXPECT_EQ(demdata.dim, 16);
    EXPECT_EQ(demdata.stride, 16 + 2 * DEMData::border);
    EXPECT_EQ(demdata.getImage()->bytes(), size_t(demdata.stride * demdata.stride * 4));
};

TEST(DEMData, InitialBackfill) {
    PremultipliedImage image1 = fakeImage({4, 4});
    DEMData dem1(image1, Tileset::RasterEncoding::Mapbox);

    constexpr int border = DEMData::border;
    constexpr int dim = 4;

    bool nonempty = true;
    // checking that the `border`-wide ring around the fake image has been
    // populated with a non-empty pixel value
    for (int x = -border; x < dim + border; x++) {
        for (int y = -border; y < dim + border; y++) {
            if (dem1.get(x, y) == -65536) {
                nonempty = false;
                break;
            }
        }
    }
    EXPECT_TRUE(nonempty);

    // Vertical border ring: every column at x ∈ [-border, -1] is initially
    // equal to the leftmost interior column; every column at x ∈ [dim, dim
    // + border − 1] is equal to the rightmost interior column.
    for (int b = 1; b <= border; b++) {
        for (int y = 0; y < dim; y++) {
            EXPECT_EQ(dem1.get(-b, y), dem1.get(0, y));
            EXPECT_EQ(dem1.get(dim + b - 1, y), dem1.get(dim - 1, y));
        }
    }

    // Horizontal border ring: same shape, transposed.
    for (int b = 1; b <= border; b++) {
        for (int x = 0; x < dim; x++) {
            EXPECT_EQ(dem1.get(x, -b), dem1.get(x, 0));
            EXPECT_EQ(dem1.get(x, dim + b - 1), dem1.get(x, dim - 1));
        }
    }

    // Corners: the entire border × border square at each corner is seeded
    // from the nearest interior corner pixel.
    for (int dy = 1; dy <= border; dy++) {
        for (int dx = 1; dx <= border; dx++) {
            EXPECT_EQ(dem1.get(-dx, -dy), dem1.get(0, 0));
            EXPECT_EQ(dem1.get(dim + dx - 1, -dy), dem1.get(dim - 1, 0));
            EXPECT_EQ(dem1.get(-dx, dim + dy - 1), dem1.get(0, dim - 1));
            EXPECT_EQ(dem1.get(dim + dx - 1, dim + dy - 1), dem1.get(dim - 1, dim - 1));
        }
    }
};

TEST(DEMData, BackfillNeighbor) {
    PremultipliedImage image1 = fakeImage({4, 4});
    DEMData dem0(image1, Tileset::RasterEncoding::Mapbox);

    PremultipliedImage image2 = fakeImage({4, 4});
    DEMData dem1(image2, Tileset::RasterEncoding::Mapbox);

    constexpr int border = DEMData::border;
    constexpr int dim = 4;

    // West neighbour: dem1 sits to the left of dem0, so dem1's right
    // `border` columns become dem0's left border.
    dem0.backfillBorder(dem1, -1, 0);
    for (int b = 1; b <= border; b++) {
        for (int y = 0; y < dim; y++) {
            EXPECT_EQ(dem0.get(-b, y), dem1.get(dim - b, y));
        }
    }

    // North neighbour
    dem0.backfillBorder(dem1, 0, -1);
    for (int b = 1; b <= border; b++) {
        for (int x = 0; x < dim; x++) {
            EXPECT_EQ(dem0.get(x, -b), dem1.get(x, dim - b));
        }
    }

    // East neighbour
    dem0.backfillBorder(dem1, 1, 0);
    for (int b = 0; b < border; b++) {
        for (int y = 0; y < dim; y++) {
            EXPECT_EQ(dem0.get(dim + b, y), dem1.get(b, y));
        }
    }

    // South neighbour
    dem0.backfillBorder(dem1, 0, 1);
    for (int b = 0; b < border; b++) {
        for (int x = 0; x < dim; x++) {
            EXPECT_EQ(dem0.get(x, dim + b), dem1.get(x, b));
        }
    }

    // Diagonals — `border × border` square at each corner.
    dem0.backfillBorder(dem1, -1, 1);
    for (int dy = 0; dy < border; dy++) {
        for (int dx = 1; dx <= border; dx++) {
            EXPECT_EQ(dem0.get(-dx, dim + dy), dem1.get(dim - dx, dy));
        }
    }

    dem0.backfillBorder(dem1, 1, 1);
    for (int dy = 0; dy < border; dy++) {
        for (int dx = 0; dx < border; dx++) {
            EXPECT_EQ(dem0.get(dim + dx, dim + dy), dem1.get(dx, dy));
        }
    }

    dem0.backfillBorder(dem1, -1, -1);
    for (int dy = 1; dy <= border; dy++) {
        for (int dx = 1; dx <= border; dx++) {
            EXPECT_EQ(dem0.get(-dx, -dy), dem1.get(dim - dx, dim - dy));
        }
    }

    dem0.backfillBorder(dem1, 1, -1);
    for (int dy = 1; dy <= border; dy++) {
        for (int dx = 0; dx < border; dx++) {
            EXPECT_EQ(dem0.get(dim + dx, -dy), dem1.get(dx, dim - dy));
        }
    }
};
