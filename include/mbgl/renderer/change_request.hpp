#pragma once

#include <mbgl/util/color.hpp>
#include <mbgl/util/identity.hpp>

#include <memory>
#include <vector>

namespace mbgl {
class ChangeRequest;
class LayerGroupBase;
class TileLayerGroup;
class RenderOrchestrator;
class RenderTarget;

using ChangeRequestPtr = std::shared_ptr<ChangeRequest>;
using UniqueChangeRequest = std::unique_ptr<ChangeRequest>;
using UniqueChangeRequestVec = std::vector<UniqueChangeRequest>;
using LayerGroupBasePtr = std::shared_ptr<LayerGroupBase>;
using RenderTargetPtr = std::shared_ptr<RenderTarget>;

namespace gfx {
class Drawable;
using DrawablePtr = std::shared_ptr<Drawable>;
} // namespace gfx

/**
    Base class for changes to be made to the scene
 */
class ChangeRequest {
protected:
    ChangeRequest() = default;

public:
    virtual ~ChangeRequest() = default;

    virtual void execute(RenderOrchestrator &) = 0;
};

/**
    Base for change requests based on an ID reference
 */
class RefChangeRequest : public ChangeRequest {
protected:
    RefChangeRequest(util::SimpleIdentity id_)
        : id(id_) {}
    RefChangeRequest(const RefChangeRequest &) = default;

    util::SimpleIdentity id;
};

/**
    Add a new layer group to the scene
 */
class AddLayerGroupRequest : public ChangeRequest {
public:
    AddLayerGroupRequest(LayerGroupBasePtr layerGroup_);
    AddLayerGroupRequest(AddLayerGroupRequest &&other);

    void execute(RenderOrchestrator &) override;

protected:
    LayerGroupBasePtr layerGroup;
};

/**
    Remove a layer group from the scene
 */
class RemoveLayerGroupRequest : public ChangeRequest {
public:
    RemoveLayerGroupRequest(int32_t layerIndex_)
        : layerIndex(layerIndex_) {}
    RemoveLayerGroupRequest(const RemoveLayerGroupRequest &) = default;

    void execute(RenderOrchestrator &) override;

protected:
    int32_t layerIndex;
};

class UpdateLayerGroupIndexRequest : public ChangeRequest {
public:
    UpdateLayerGroupIndexRequest(LayerGroupBasePtr, int32_t newLayerIndex_);
    UpdateLayerGroupIndexRequest(const UpdateLayerGroupIndexRequest &) = delete;

    void execute(RenderOrchestrator &) override;

protected:
    LayerGroupBasePtr layerGroup;
    int32_t newLayerIndex;
};

/**
    Add a new render target to the scene
 */
class AddRenderTargetRequest : public ChangeRequest {
public:
    AddRenderTargetRequest(RenderTargetPtr renderTarget_);

    void execute(RenderOrchestrator &) override;

protected:
    RenderTargetPtr renderTarget;
};

/**
    Remove a render target from the scene
 */
class RemoveRenderTargetRequest : public ChangeRequest {
public:
    RemoveRenderTargetRequest(RenderTargetPtr renderTarget_);

    void execute(RenderOrchestrator &) override;

protected:
    RenderTargetPtr renderTarget;
};

} // namespace mbgl
