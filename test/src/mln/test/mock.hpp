#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <set>
#include <map>

#include <mln/tile/tile_id.hpp>
#include <mln/util/range.hpp>

struct MockTileData;

struct MockSource {
    mln::Range<uint8_t> zoomRange{0, 16};
    std::map<mln::OverscaledTileID, std::unique_ptr<MockTileData>> dataTiles;
    std::set<mln::OverscaledTileID> idealTiles;

    // Test API
    inline MockTileData* createTileData(const mln::OverscaledTileID& tileID);
};

struct MockBucket {};

struct MockTileData {
    MockTileData(const mln::OverscaledTileID& tileID_)
        : tileID(tileID_) {}

    bool hasTriedCache() const { return triedOptional; }

    bool isRenderable() const { return renderable; }

    bool isLoaded() const { return loaded; }

    bool renderable = false;
    bool triedOptional = false;
    bool loaded = false;
    const mln::OverscaledTileID tileID;
};

MockTileData* MockSource::createTileData(const mln::OverscaledTileID& tileID) {
    // Replace the existing MockTileData object, if any.
    return (dataTiles[tileID] = std::make_unique<MockTileData>(tileID)).get();
}
