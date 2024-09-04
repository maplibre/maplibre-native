#include <mbgl/actor/scheduler.hpp>
#include <mbgl/renderer/sources/render_tile_source.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <mbgl/tile/tile.hpp>

#include <gmock/gmock.h>

namespace source_test {

using namespace mbgl;

class FakeTileSource;

class FakeTile : public Tile {
public:
    FakeTile(FakeTileSource& source_, const OverscaledTileID& tileID, TileObserver* observer_ = nullptr)
        : Tile(Tile::Kind::Geometry, tileID, "FakeTileSource", observer_),
          source(source_) {
        renderable = true;
    }
    void setNecessity(TileNecessity necessity) override;
    void setUpdateParameters(const TileUpdateParameters&) override;
    bool layerPropertiesUpdated(const Immutable<style::LayerProperties>&) override { return true; }

    std::unique_ptr<TileRenderData> createRenderData() override { return nullptr; }

    void cancel() override {}

private:
    FakeTileSource& source;
};

class FakeTileSource : public RenderTileSetSource {
public:
    MOCK_METHOD1(tileSetNecessity, void(TileNecessity));
    MOCK_METHOD1(tileSetMinimumUpdateInterval, void(Duration));

    explicit FakeTileSource(Immutable<style::Source::Impl> impl_, const TaggedScheduler& threadPool_)
        : RenderTileSetSource(std::move(impl_), threadPool_) {}
    void updateInternal(const Tileset& tileset,
                        const std::vector<Immutable<style::LayerProperties>>& layers,
                        const bool needsRendering,
                        const bool needsRelayout,
                        const TileParameters& parameters) override {
        tilePyramid.update(layers,
                           needsRendering,
                           needsRelayout,
                           parameters,
                           *baseImpl,
                           util::tileSize_I,
                           tileset.zoomRange,
                           tileset.bounds,
                           [&](const OverscaledTileID& tileID, TileObserver* observer_) {
                               return std::make_unique<FakeTile>(*this, tileID, observer_);
                           });
    }

    const std::optional<Tileset>& getTileset() const override {
        return static_cast<const style::VectorSource::Impl&>(*baseImpl).tileset;
    }
};

inline void FakeTile::setNecessity(TileNecessity necessity) {
    source.tileSetNecessity(necessity);
}

inline void FakeTile::setUpdateParameters(const TileUpdateParameters& params) {
    source.tileSetMinimumUpdateInterval(params.minimumUpdateInterval);
}

} // namespace source_test
