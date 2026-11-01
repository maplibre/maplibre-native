#pragma once

#include <mln/style/image_impl.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/text/glyph.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/tile/layout_resource_manager.hpp>
#include <mln/util/containers.hpp>

#include <memory>
#include <set>
#include <string>

namespace mln {

class BucketParameters;
class RenderLayer;
class FeatureIndex;
class LayerRenderData;
class GlyphManager;

class Layout {
public:
    virtual ~Layout() = default;

    virtual void createBucket(const ImagePositions&,
                              std::unique_ptr<FeatureIndex>&,
                              mln::unordered_map<std::string, LayerRenderData>&,
                              bool,
                              bool,
                              const CanonicalTileID&) = 0;

    virtual void prepareSymbols(const GlyphMap&, const GlyphPositions&, const ImageMap&, const ImagePositions&) {}

    virtual void finalizeSymbols(HBShapeResults&) {}

    // The resources the tile has received, including the ones this layout added to its dependencies.
    virtual void prepareResources(const LayoutResourceMap&) {}

    virtual bool needFinalizeSymbols() { return false; }

    virtual bool hasSymbolInstances() const { return true; }

    virtual bool hasDependencies() const = 0;
};

class LayoutParameters {
public:
    const BucketParameters& bucketParameters;
    std::shared_ptr<FontFaces> fontFaces;
    GlyphDependencies& glyphDependencies;
    ImageDependencies& imageDependencies;
    const std::set<std::string>& availableImages;
    // URLs a layout needs loaded before its bucket is built (see Layout::prepareResources).
    LayoutResourceDependencies& resourceDependencies;
};

} // namespace mln
