#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/renderer/render_target.hpp>
#include <optional>
#include <set>

namespace mln {
class TexturePool {
public:
    /// Drape targets come in two sizes, in px (square): `fullSize` for the tiles nearest the
    /// camera, where the ground is magnified most, and `halfSize` for the rest. A full-size target
    /// costs about four times the GPU memory of a half-size one.
    TexturePool(uint32_t fullSize, uint32_t halfSize);
    ~TexturePool();

    std::shared_ptr<RenderTarget> getRenderTarget(const UnwrappedTileID& id) const;
    std::shared_ptr<RenderTarget> getRenderTargetAncestorOrDescendant(
        const UnwrappedTileID& id, std::optional<UnwrappedTileID>& terrainTileID) const;

    /// What createRenderTargets did to the targets it already had.
    struct Resizes {
        /// The tiles that got a new target for the other size.
        std::set<UnwrappedTileID> replaced;
        /// Whether some were left at their size for a later call.
        bool deferred = false;
    };

    /// Gives each tile of `cover` a target: full size for those in `fullSizeTiles`, half size for
    /// the rest. A target already of its size is kept. One of the other size is replaced by a new
    /// target, which bakes at once whatever the drape budget (it has no earlier bake to keep
    /// showing); so at most `maxResizes` (0 = no limit) are replaced per call, the ones going to
    /// half size first so that the memory is freed before more is asked for, and the others keep
    /// their size for a later call.
    Resizes createRenderTargets(gfx::Context& context,
                                const std::set<UnwrappedTileID>& cover,
                                const std::set<UnwrappedTileID>& fullSizeTiles,
                                const Color& backgroundColor,
                                size_t maxResizes);

    /// How many frames (calls of createRenderTargets) a memory failure holds back new full-size
    /// targets for. It is also the window in which further failures go unreported.
    static constexpr uint32_t failureWindowFrames = 120;

    /// The tiles whose target is full size.
    std::set<UnwrappedTileID> tilesAtFullSize() const;

    /// The device ran out of memory for this tile's target. Drops it, so the next call to
    /// createRenderTargets gives the tile a new one to bake again (the failed one recorded a bake it
    /// never made), and for the next `failureWindowFrames` frames no tile newly gets a full-size
    /// target: asking for the memory that just ran out would fail again. Returns whether this is
    /// the first failure since that window began, the one worth reporting. If even a half-size
    /// target fails, the tile fails again each frame it is drawn, and that must not log, or ask
    /// for another frame, every time; a failure that goes on is reported once per window, and a
    /// failure inside it does not extend it.
    bool renderTargetFailed(const UnwrappedTileID& id);

    /// Remove render targets for tiles that are no longer part of the given set
    void removeStaleRenderTargets(const std::set<UnwrappedTileID>& currentTiles);

    template <typename Func /* void(std::shared_ptr<RenderTarget>&) */>
    void visitRenderTargets(Func f) {
        for (auto& pair : renderTargets) {
            if (pair.second) {
                f(pair.second);
            }
        }
    }

    template <typename Func /* void(std::shared_ptr<RenderTarget>&) */>
    void visitRenderTargets(Func f) const {
        for (const auto& pair : renderTargets) {
            if (pair.second) {
                f(pair.second);
            }
        }
    }

private:
    uint32_t fullSize;
    uint32_t halfSize;
    /// Calls of createRenderTargets still to go in the window a failure began.
    uint32_t fullSizeHold = 0;

    std::map<UnwrappedTileID, std::shared_ptr<RenderTarget>> renderTargets;
};

} // namespace mln
