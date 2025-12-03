#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/text/glyph_manager.hpp>
#include <mbgl/text/glyph_manager_observer.hpp>
#include <mbgl/text/glyph_pbf.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/tiny_sdf.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/logging.hpp>

#include <fstream>

namespace mbgl {

namespace {
GlyphManagerObserver nullObserver;
}

GlyphManager::GlyphManager(std::unique_ptr<LocalGlyphRasterizer> localGlyphRasterizer_)
    : observer(&nullObserver),
      localGlyphRasterizer(std::move(localGlyphRasterizer_)) {}

GlyphManager::~GlyphManager() {
    hbShapers.clear(); // clear harfbuzz + freetype face before library;
}

void GlyphManager::getGlyphs(GlyphRequestor& requestor, GlyphDependencies glyphDependencies, FileSource& fileSource) {
    auto dependencies = std::make_shared<GlyphDependencies>(std::move(glyphDependencies));
    {
        std::lock_guard<std::recursive_mutex> readWriteLock(rwLock);
        // Figure out which glyph ranges need to be fetched. For each range that
        // does need to be fetched, record an entry mapping the requestor to a
        // shared pointer containing the dependencies. When the shared pointer
        // becomes unique, we know that all the dependencies for that requestor have
        // been fetched, and can notify it of completion.
        for (const auto& dependency : dependencies->glyphs) {
            const FontStack& fontStack = dependency.first;
            Entry& entry = entries[fontStack];

            const GlyphIDs& glyphIDs = dependency.second;
            std::unordered_set<GlyphRange> ranges;
            for (const auto& glyphID : glyphIDs) {
                if (localGlyphRasterizer->canRasterizeGlyph(fontStack, glyphID)) {
                    if (entry.glyphs.find(glyphID) == entry.glyphs.end()) {
                        entry.glyphs.emplace(glyphID, makeMutable<Glyph>(generateLocalSDF(fontStack, glyphID)));
                    }
                } else {
                    ranges.insert(getGlyphRange(glyphID));
                }
            }

            for (const auto& range : ranges) {
                auto it = entry.ranges.find(range);
                if (it == entry.ranges.end() || !it->second.parsed) {
                    GlyphRequest& request = entry.ranges[range];
                    request.requestors[&requestor] = dependencies;
                    requestRange(request, fontStack, range, fileSource);
                }
            }
        }
    }

    // If the shared dependencies pointer is already unique, then all dependent
    // glyph ranges have already been loaded. Send a notification immediately.
    if (dependencies.use_count() == 1) {
        notify(requestor, *dependencies);
    }
}

Glyph GlyphManager::generateLocalSDF(const FontStack& fontStack, GlyphID glyphID) {
    Glyph local = localGlyphRasterizer->rasterizeGlyph(fontStack, glyphID);
    local.bitmap = util::transformRasterToSDF(local.bitmap, 8, .25);
    return local;
}

void GlyphManager::requestRange(GlyphRequest& request,
                                const FontStack& fontStack,
                                const GlyphRange& range,
                                FileSource& fileSource) {
    if (request.req) {
        return;
    }
    Resource res(Resource::Kind::Unknown, "");
    switch (range.type) {
        case GlyphIDType::FontPBF:
            res = Resource::glyphs(glyphURL, fontStack, std::pair<uint16_t, uint16_t>{range.first, range.second});
            break;
        default: {
            std::string url = getFontFaceURL(range.type);
            if (url.size()) {
                res = Resource::fontFace(url);
            } else {
                Log::Error(Event::Style, "Try download a glyph doesn't in current faces");
            }

        } break;
    }

    observer->onGlyphsRequested(fontStack, range);

    request.req = fileSource.request(
        res, [this, fontStack, range](const Response& response) { processResponse(response, fontStack, range); });
}

void GlyphManager::processResponse(const Response& res, const FontStack& fontStack, const GlyphRange& range) {
    if (res.error) {
        observer->onGlyphsError(fontStack, range, std::make_exception_ptr(std::runtime_error(res.error->message)));
        return;
    }

    if (res.notModified) {
        return;
    }

    {
        std::lock_guard<std::recursive_mutex> readWriteLock(rwLock);

        Entry& entry = entries[fontStack];
        GlyphRequest& request = entry.ranges[range];

        if (!res.noContent) {
            std::vector<Glyph> glyphs;

            try {
                if (range.type == GlyphIDType::FontPBF) {
                    glyphs = parseGlyphPBF(range, *res.data);
                } else {
                    if (loadHBShaper(fontStack, range.type, *res.data)) {
                        Glyph temp;
                        temp.id = GlyphID(0, range.type);
                        glyphs.emplace_back(std::move(temp));
                    }
                }
            } catch (...) {
                observer->onGlyphsError(fontStack, range, std::current_exception());
                return;
            }

            for (auto& glyph : glyphs) {
                auto id = glyph.id;
                if (!localGlyphRasterizer->canRasterizeGlyph(fontStack, id)) {
                    entry.glyphs.erase(id);
                    entry.glyphs.emplace(id, makeMutable<Glyph>(std::move(glyph)));
                }
            }
        }

        request.parsed = true;

        for (auto& pair : request.requestors) {
            GlyphRequestor& requestor = *pair.first;
            const std::shared_ptr<GlyphDependencies>& dependencies = pair.second;
            if (dependencies.use_count() == 1) {
                notify(requestor, *dependencies);
            }
        }

        request.requestors.clear();
    }

    observer->onGlyphsLoaded(fontStack, range);
}

void GlyphManager::setObserver(GlyphManagerObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver;
}

void GlyphManager::notify(GlyphRequestor& requestor, const GlyphDependencies& glyphDependencies) {
    GlyphMap response;

    for (const auto& dependency : glyphDependencies.glyphs) {
        const FontStack& fontStack = dependency.first;
        const GlyphIDs& glyphIDs = dependency.second;

        Glyphs& glyphs = response[FontStackHasher()(fontStack)];
        Entry& entry = entries[fontStack];

        for (const auto& glyphID : glyphIDs) {
            auto it = entry.glyphs.find(glyphID);
            if (it != entry.glyphs.end()) {
                glyphs.emplace(*it);
            } else {
                glyphs.emplace(glyphID, std::nullopt);
            }
        }
    }

    requestor.onGlyphsAvailable(response, glyphDependencies.shapes);
}

void GlyphManager::removeRequestor(GlyphRequestor& requestor) {
    std::lock_guard<std::recursive_mutex> readWriteLock(rwLock);
    for (auto& entry : entries) {
        for (auto& range : entry.second.ranges) {
            range.second.requestors.erase(&requestor);
        }
    }
}

void GlyphManager::evict(const std::set<FontStack>& keep) {
    std::lock_guard<std::recursive_mutex> readWriteLock(rwLock);
    util::erase_if(entries, [&](const auto& entry) { return keep.count(entry.first) == 0; });
}

std::shared_ptr<HBShaper> GlyphManager::getHBShaper(FontStack fontStack, GlyphIDType type) {
    if (hbShapers.find(fontStack) != hbShapers.end()) {
        auto& glyphs = hbShapers[fontStack];
        if (glyphs.find(type) != glyphs.end()) {
            return glyphs[type];
        }
    }

    return nullptr;
}

bool GlyphManager::loadHBShaper(const FontStack& fontStack, GlyphIDType type, const std::string& data) {
    auto shaper = std::make_shared<HBShaper>(type, data, ftLibrary);
    if (!shaper->valid()) return false;
    hbShapers[fontStack][type] = shaper;
    return true;
}

Immutable<Glyph> GlyphManager::getGlyph(const FontStack& fontStack, GlyphID glyphID) {
    auto& entry = entries[fontStack];
    if (entry.glyphs.find(glyphID) != entry.glyphs.end()) return entry.glyphs.at(glyphID);

    if (glyphID.complex.type != FontPBF) {
        auto shaper = getHBShaper(fontStack, glyphID.complex.type);
        if (shaper) {
            auto glyph = shaper->rasterizeGlyph(glyphID);

            glyph.bitmap = util::transformRasterToSDF(glyph.bitmap, 8, .25);
            entry.glyphs.emplace(glyphID, makeMutable<Glyph>(std::move(glyph)));
            return entry.glyphs.at(glyphID);
        }
    }

    Glyph empty;

    return makeMutable<Glyph>(std::move(empty));
}

void GlyphManager::hbShaping(const std::u16string& text,
                             const FontStack& font,
                             GlyphIDType type,
                             std::vector<GlyphID>& glyphIDs,
                             std::vector<HBShapeAdjust>& adjusts) {
    auto shaper = getHBShaper(font, type);
    if (shaper) {
        shaper->createComplexGlyphIDs(text, glyphIDs, adjusts);
    }
}

std::string GlyphManager::getFontFaceURL(GlyphIDType type) {
    std::string url;

    if (fontFaces) {
        for (auto& face : *fontFaces) {
            if (face.type == type) {
                url = face.url;
                break;
            }
        }
    }

    return url;
}

} // namespace mbgl
