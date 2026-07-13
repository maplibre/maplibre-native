#pragma once

#include <mbgl/text/glyph.hpp>
#include <mbgl/text/glyph_manager_observer.hpp>
#include <mbgl/text/glyph_range.hpp>
#include <mbgl/text/local_glyph_rasterizer.hpp>
#include <mbgl/util/font_stack.hpp>
#include <mbgl/util/immutable.hpp>

#include <mutex>
#include <string>
#include <unordered_map>

#include "harfbuzz.hpp"

namespace mbgl {

class FileSource;
class AsyncRequest;
class Response;

struct HBShapeResult {
    std::u16string str;

    std::shared_ptr<std::vector<HBShapeAdjust>> adjusts;

    HBShapeResult() {}

    HBShapeResult(const std::u16string &str_, std::shared_ptr<std::vector<HBShapeAdjust>> adjusts_)
        : str(str_),
          adjusts(adjusts_) {}
};
using HBShapeResults = std::map<FontStack, std::map<GlyphIDType, std::map<std::u16string, HBShapeResult>>>;

class GlyphRequestor {
public:
    virtual void onGlyphsAvailable(GlyphMap, HBShapeRequests) = 0;

protected:
    virtual ~GlyphRequestor() = default;
};

class GlyphManager {
public:
    GlyphManager(const GlyphManager &) = delete;
    GlyphManager &operator=(const GlyphManager &) = delete;
    explicit GlyphManager(
        std::unique_ptr<LocalGlyphRasterizer> = std::make_unique<LocalGlyphRasterizer>(std::optional<std::string>()));
    ~GlyphManager();

    // Workers send a `getGlyphs` message to the main thread once they have
    // determined their `GlyphDependencies`. If all glyphs are already locally
    // available, GlyphManager will provide them to the requestor immediately.
    // Otherwise, it makes a request on the FileSource is made for each range
    // needed, and notifies the observer when all are complete.
    void getGlyphs(GlyphRequestor &, GlyphDependencies, FileSource &);
    void removeRequestor(GlyphRequestor &);

    void setURL(const std::string &url) { glyphURL = url; }

    void setObserver(GlyphManagerObserver *);

    // Remove glyphs for all but the supplied font stacks.
    void evict(const std::set<FontStack> &);

    Immutable<Glyph> getGlyph(const FontStack &, GlyphID);

    void setFontFaces(std::shared_ptr<FontFaces> faces) { fontFaces = faces; }

    std::shared_ptr<HBShaper> getHBShaper(FontStack, GlyphIDType);

    void hbShaping(const std::u16string &text,
                   const FontStack &font,
                   GlyphIDType type,
                   std::vector<GlyphID> &glyphIDs,
                   std::vector<HBShapeAdjust> &adjusts);

    std::shared_ptr<FontFaces> getFontFaces() { return fontFaces; }

    std::string getFontFaceURL(GlyphIDType type);

private:
    Glyph generateLocalSDF(const FontStack &fontStack, GlyphID glyphID);
    std::string glyphURL;

    struct GlyphRequest {
        bool parsed = false;
        std::unique_ptr<AsyncRequest> req;
        std::unordered_map<GlyphRequestor *, std::shared_ptr<GlyphDependencies>> requestors;
    };

    struct Entry {
        std::map<GlyphRange, GlyphRequest> ranges;
        std::map<GlyphID, Immutable<Glyph>> glyphs;
    };

    std::unordered_map<FontStack, Entry, FontStackHasher> entries;

    void requestRange(GlyphRequest &, const FontStack &, const GlyphRange &, FileSource &fileSource);
    void processResponse(const Response &, const FontStack &, const GlyphRange &);
    void notify(GlyphRequestor &, const GlyphDependencies &);

    GlyphManagerObserver *observer = nullptr;

    // Shaping objects
    std::unique_ptr<LocalGlyphRasterizer> localGlyphRasterizer;
    std::shared_ptr<FontFaces> fontFaces;

    FreeTypeLibrary ftLibrary;
    std::map<FontStack, std::map<GlyphIDType, std::shared_ptr<HBShaper>>> hbShapers;
    bool loadHBShaper(const FontStack &fontStack, GlyphIDType type, const std::string &data);

    std::recursive_mutex rwLock;
};

} // namespace mbgl
