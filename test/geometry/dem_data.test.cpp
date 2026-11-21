#include <mln/test/util.hpp>

#include <mln/util/image.hpp>
#include <mln/util/tileset.hpp>
#include <mln/geometry/dem_data.hpp>

using namespace mln;

auto fakeImage = [](Size s) {
    PremultipliedImage img = PremultipliedImage(s);

    for (size_t i = 0; i < img.bytes(); i++) {
        img.data[i] = (i + 1) % 4 == 0 ? 1 : std::rand() % 255;
    }
    return img;
};

TEST(DEMData, ConstructorMapbox) {
    PremultipliedImage image = fakeImage({16, 16});
    DEMData demdata(image, Tileset::RasterEncoding::Mapbox);

    EXPECT_EQ(demdata.dim, 16);
    EXPECT_EQ(demdata.stride, 20);
    EXPECT_EQ(demdata.getImage()->bytes(), size_t(20 * 20 * 4));
};

TEST(DEMData, ConstructorTerrarium) {
    PremultipliedImage image = fakeImage({16, 16});
    DEMData demdata(image, Tileset::RasterEncoding::Terrarium);

    EXPECT_EQ(demdata.dim, 16);
    EXPECT_EQ(demdata.stride, 20);
    EXPECT_EQ(demdata.getImage()->bytes(), size_t(20 * 20 * 4));
};

TEST(DEMData, InitialBackfill) {
    PremultipliedImage image1 = fakeImage({4, 4});
    DEMData dem1(image1, Tileset::RasterEncoding::Mapbox);

    // both border columns of DEM data are initially equal to the nearest column of data
    bool verticalBorderMatch = true;
    for (int y = 0; y < 4; y++) {
        for (int x : {-2, -1}) {
            if (dem1.get(x, y) != dem1.get(0, y)) {
                verticalBorderMatch = false;
            }
        }
        for (int x : {4, 5}) {
            if (dem1.get(x, y) != dem1.get(3, y)) {
                verticalBorderMatch = false;
            }
        }
    }
    EXPECT_TRUE(verticalBorderMatch);

    // both border rows of DEM data are initially equal to the nearest row of data
    bool horizontalBorderMatch = true;
    for (int x = 0; x < 4; x++) {
        for (int y : {-2, -1}) {
            if (dem1.get(x, y) != dem1.get(x, 0)) {
                horizontalBorderMatch = false;
            }
        }
        for (int y : {4, 5}) {
            if (dem1.get(x, y) != dem1.get(x, 3)) {
                horizontalBorderMatch = false;
            }
        }
    }
    EXPECT_TRUE(horizontalBorderMatch);

    // every corner cell is initially equal to the closest corner pixel
    for (int x : {-2, -1}) {
        for (int y : {-2, -1}) {
            EXPECT_EQ(dem1.get(x, y), dem1.get(0, 0));
        }
        for (int y : {4, 5}) {
            EXPECT_EQ(dem1.get(x, y), dem1.get(0, 3));
        }
    }
    for (int x : {4, 5}) {
        for (int y : {-2, -1}) {
            EXPECT_EQ(dem1.get(x, y), dem1.get(3, 0));
        }
        for (int y : {4, 5}) {
            EXPECT_EQ(dem1.get(x, y), dem1.get(3, 3));
        }
    }
};

TEST(DEMData, BackfillNeighbor) {
    PremultipliedImage image1 = fakeImage({4, 4});
    DEMData dem0(image1, Tileset::RasterEncoding::Mapbox);

    PremultipliedImage image2 = fakeImage({4, 4});
    DEMData dem1(image2, Tileset::RasterEncoding::Mapbox);

    // Each neighbour fills two pixels deep, so the border carries the neighbour's two
    // nearest columns/rows rather than just its edge.
    dem0.backfillBorder(dem1, -1, 0);
    for (int y = 0; y < 4; y++) {
        // dx = -1, dy = 0: the left border of dem0 is the right edge of dem1
        EXPECT_EQ(dem0.get(-1, y), dem1.get(3, y));
        EXPECT_EQ(dem0.get(-2, y), dem1.get(2, y));
    }

    dem0.backfillBorder(dem1, 0, -1);
    // backfills TopCenter neighbor
    for (int x = 0; x < 4; x++) {
        EXPECT_EQ(dem0.get(x, -1), dem1.get(x, 3));
        EXPECT_EQ(dem0.get(x, -2), dem1.get(x, 2));
    }

    dem0.backfillBorder(dem1, 1, 0);
    // backfills Right neighbor
    for (int y = 0; y < 4; y++) {
        EXPECT_EQ(dem0.get(4, y), dem1.get(0, y));
        EXPECT_EQ(dem0.get(5, y), dem1.get(1, y));
    }

    dem0.backfillBorder(dem1, 0, 1);
    // backfills BottomCenter neighbor
    for (int x = 0; x < 4; x++) {
        EXPECT_EQ(dem0.get(x, 4), dem1.get(x, 0));
        EXPECT_EQ(dem0.get(x, 5), dem1.get(x, 1));
    }

    dem0.backfillBorder(dem1, -1, 1);
    // backfills BottomLeft neighbor
    EXPECT_EQ(dem0.get(-1, 4), dem1.get(3, 0));
    EXPECT_EQ(dem0.get(-2, 5), dem1.get(2, 1));

    dem0.backfillBorder(dem1, 1, 1);
    // backfills BottomRight neighbor
    EXPECT_EQ(dem0.get(4, 4), dem1.get(0, 0));
    EXPECT_EQ(dem0.get(5, 5), dem1.get(1, 1));

    dem0.backfillBorder(dem1, -1, -1);
    // backfills TopLeft neighbor
    EXPECT_EQ(dem0.get(-1, -1), dem1.get(3, 3));
    EXPECT_EQ(dem0.get(-2, -2), dem1.get(2, 2));

    dem0.backfillBorder(dem1, 1, -1);
    // backfills TopRight neighbor
    EXPECT_EQ(dem0.get(4, -1), dem1.get(0, 3));
    EXPECT_EQ(dem0.get(5, -2), dem1.get(1, 2));
};

TEST(DEMData, TerrariumNoData) {
    // A fresh image is all zeroes, i.e. terrarium no-data, bar one pixel put at a real
    // 256 m (129 * 256 - 32768). 128 would decode to 0 m and prove nothing.
    PremultipliedImage image({4, 4});
    for (size_t i = 3; i < image.bytes(); i += 4) {
        image.data[i] = 1; // alpha, as fakeImage does
    }
    image.data[(1 * 4 + 1) * 4] = 129;

    DEMData terrarium(image, Tileset::RasterEncoding::Terrarium);
    EXPECT_EQ(terrarium.get(0, 0), 0);
    EXPECT_EQ(terrarium.get(1, 1), 256);
    // No-data must not drag the tile's elevation range below sea level.
    EXPECT_EQ(terrarium.getMinElevation(), 0);
    EXPECT_EQ(terrarium.getMaxElevation(), 256);

    // Mapbox encoding has no no-data value; the same pixels decode as they always did.
    DEMData mapbox(image, Tileset::RasterEncoding::Mapbox);
    EXPECT_EQ(mapbox.get(0, 0), -10000);
};
