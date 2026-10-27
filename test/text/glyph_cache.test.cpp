#include <mln/test/util.hpp>

#include <mln/text/glyph_cache.hpp>
#include <mln/text/glyph_pbf.hpp>
#include <mln/util/io.hpp>

#include <cstring>
#include <thread>
#include <vector>

using namespace mln;

namespace {

Glyph glyph(char16_t id, uint8_t value) {
    Glyph glyph;
    glyph.id = id;
    glyph.metrics.width = 4;
    glyph.metrics.height = 4;
    glyph.bitmap = AlphaImage({4, 4});
    std::memset(glyph.bitmap.data.get(), value, glyph.bitmap.bytes());
    return glyph;
}

std::vector<Immutable<Glyph>> internRange(GlyphCache& cache, const std::string& data) {
    std::vector<Immutable<Glyph>> interned;
    for (auto& parsed : parseGlyphPBF(GlyphRange(0, 255, GlyphIDType::FontPBF), data)) {
        interned.push_back(cache.intern(std::move(parsed)));
    }
    return interned;
}

} // namespace

TEST(GlyphCache, InternsEqualGlyphsOnce) {
    GlyphCache cache;

    EXPECT_EQ(cache.intern(glyph(u'a', 1)), cache.intern(glyph(u'a', 1)));
}

TEST(GlyphCache, KeepsGlyphsWithTheSameIdAndDifferentBitmapsApart) {
    GlyphCache cache;

    EXPECT_NE(cache.intern(glyph(u'a', 1)), cache.intern(glyph(u'a', 2)));
}

TEST(GlyphCache, ReleasesAGlyphWithItsLastOwner) {
    GlyphCache cache;
    const Immutable<Glyph> kept = cache.intern(glyph(u'a', 1));
    cache.intern(glyph(u'b', 1));

    EXPECT_EQ(1u, cache.size());
}

TEST(GlyphCache, InternsTheSameRangeParsedOnTwoThreads) {
    GlyphCache cache;
    const std::string data = util::read_file("test/fixtures/resources/glyphs.pbf");
    std::vector<Immutable<Glyph>> first;
    std::vector<Immutable<Glyph>> second;

    std::thread a([&] { first = internRange(cache, data); });
    std::thread b([&] { second = internRange(cache, data); });
    a.join();
    b.join();

    ASSERT_FALSE(first.empty());
    EXPECT_EQ(first, second);
}
