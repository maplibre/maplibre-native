#include <mln/test/util.hpp>

#include <mln/util/accept_header.hpp>

using namespace mln;
using namespace mln::http;

constexpr std::string_view rasterWithWebP = acceptHeader<MIME_TYPE_WEBP, MIME_TYPE_JPEG, MIME_TYPE_PNG>;
constexpr std::string_view rasterWithoutWebP = acceptHeader<MIME_TYPE_JPEG, MIME_TYPE_PNG>;

static_assert(acceptHeader<MIME_TYPE_MVT> == "application/vnd.mapbox-vector-tile");
static_assert(rasterWithWebP == "image/webp, image/jpeg, image/png");
static_assert(rasterWithoutWebP == "image/jpeg, image/png");

// Some platforms hand the value to a C API as a plain pointer.
static_assert(rasterWithoutWebP.data()[rasterWithoutWebP.size()] == '\0');

TEST(AcceptHeader, Raster) {
    EXPECT_EQ(supportsWebPDecoding() ? rasterWithWebP : rasterWithoutWebP, rasterAcceptHeader());
}
