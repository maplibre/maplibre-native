#pragma once

#include <mbgl/util/color.hpp>
#include <mbgl/util/identity.hpp>

#include <memory>
#include <vector>

namespace mbgl {
class ChangeRequest;
class LayerGroup;
class RenderOrchestrator;

using ChangeRequestPtr = std::shared_ptr<ChangeRequest>;
using UniqueChangeRequest = std::unique_ptr<ChangeRequest>;
using UniqueChangeRequestVec = std::vector<UniqueChangeRequest>;
using UniqueLayerGroup = std::unique_ptr<LayerGroup>;

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

protected:
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
    Add a new drawable to the scene
 */
class AddDrawableRequest : public ChangeRequest {
public:
    AddDrawableRequest(gfx::DrawablePtr drawable_)
        : drawable(std::move(drawable_)) {}
    AddDrawableRequest(AddDrawableRequest &&other)
        : drawable(std::move(other.drawable)) {}

    void execute(RenderOrchestrator &) override;

protected:
    gfx::DrawablePtr drawable;
};

/**
    Remove a drawable from the scene
 */
class RemoveDrawableRequest : public RefChangeRequest {
public:
    RemoveDrawableRequest(util::SimpleIdentity id_)
        : RefChangeRequest(id_) {}
    RemoveDrawableRequest(const RemoveDrawableRequest &) = default;

    void execute(RenderOrchestrator &) override;
};

/**
    Change the color of all vertexes
 */
class ResetColorRequest : public RefChangeRequest {
public:
    ResetColorRequest(util::SimpleIdentity id_, Color color)
        : RefChangeRequest(id_),
          newColor(color) {}
    ResetColorRequest(const ResetColorRequest &other) = default;

    void execute(RenderOrchestrator &) override;

protected:
    Color newColor;
};

/**
    Add a new layer group to the scene
 */
class AddLayerGroupRequest : public ChangeRequest {
public:
    AddLayerGroupRequest(UniqueLayerGroup &&layerGroup_, bool canReplace);
    AddLayerGroupRequest(AddLayerGroupRequest &&other);

    void execute(RenderOrchestrator &) override;

protected:
    UniqueLayerGroup layerGroup;
    bool replace;
};

/**
    Remove a layer group from the scene
 */
class RemoveLayerGroupRequest : public RefChangeRequest {
public:
    RemoveLayerGroupRequest(util::SimpleIdentity id_)
        : RefChangeRequest(id_) {}
    RemoveLayerGroupRequest(const RemoveLayerGroupRequest &) = default;

    void execute(RenderOrchestrator &) override;
};

} // namespace mbgl
