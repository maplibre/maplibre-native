#pragma once

#include <mbgl/util/color.hpp>
#include <mbgl/util/identity.hpp>

#include <memory>
#include <vector>

namespace mbgl {
class ChangeRequest;
class LayerGroup;
class TileLayerGroup;
class RenderOrchestrator;

using ChangeRequestPtr = std::shared_ptr<ChangeRequest>;
using UniqueChangeRequest = std::unique_ptr<ChangeRequest>;
using UniqueChangeRequestVec = std::vector<UniqueChangeRequest>;
using LayerGroupPtr = std::shared_ptr<LayerGroup>;

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
    AddLayerGroupRequest(LayerGroupPtr layerGroup_, bool canReplace);
    AddLayerGroupRequest(AddLayerGroupRequest &&other);

    void execute(RenderOrchestrator &) override;

protected:
    LayerGroupPtr layerGroup;
    bool replace;
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
    UpdateLayerGroupIndexRequest(std::shared_ptr<TileLayerGroup> tileLayerGroup_, int32_t newLayerIndex_);
    UpdateLayerGroupIndexRequest(const UpdateLayerGroupIndexRequest &) = delete;

    void execute(RenderOrchestrator &) override;

protected:
    std::shared_ptr<TileLayerGroup> tileLayerGroup;
    int32_t newLayerIndex;
};

} // namespace mbgl
