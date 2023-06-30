#pragma once

#include <string>
#include <memory>
#include <map>

namespace mbgl {

namespace gfx {
class Texture2D;
class UploadPass;
using Texture2DPtr = std::shared_ptr<Texture2D>;
} // namespace gfx

class LayerGroupBase;
class PaintParameters;
class RenderOrchestrator;
class RenderTree;

using LayerGroupBasePtr = std::shared_ptr<LayerGroupBase>;

class RenderTarget {
public:
    virtual ~RenderTarget() = default;

    void setTexture(const gfx::Texture2DPtr& texture_) { texture = std::move(texture_); };
    const gfx::Texture2DPtr& getTexture() const { return texture; };

    bool addLayerGroup(LayerGroupBasePtr, bool replace);
    bool removeLayerGroup(const int32_t layerIndex);
    size_t numLayerGroups() const noexcept;
    const LayerGroupBasePtr& getLayerGroup(const int32_t layerIndex) const;
    void observeLayerGroups(std::function<void(LayerGroupBase&)>);
    void observeLayerGroups(std::function<void(const LayerGroupBase&)>) const;

    virtual void upload(gfx::UploadPass& uploadPass) = 0;
    virtual void render(RenderOrchestrator&, const RenderTree&, PaintParameters&) = 0;

protected:
    gfx::Texture2DPtr texture;
    using LayerGroupMap = std::map<int32_t, LayerGroupBasePtr>;
    LayerGroupMap layerGroupsByLayerIndex;
};

} // namespace mbgl
