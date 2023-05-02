#pragma once

#include <mbgl/util/color.hpp>
#include <mbgl/util/identity.hpp>

#include <memory>
#include <vector>

namespace mbgl {

class RenderOrchestrator;

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

using ChangeRequestPtr = std::shared_ptr<ChangeRequest>;
using UniqueChangeRequest = std::unique_ptr<ChangeRequest>;
using UniqueChangeRequestVec = std::vector<UniqueChangeRequest>;

/**
    Base for drawable-related change requests
 */
class DrawableRefChangeRequest : public ChangeRequest {
protected:
    DrawableRefChangeRequest(util::SimpleIdentity id)
        : drawableID(id) {}
    DrawableRefChangeRequest(const DrawableRefChangeRequest &) = default;

    util::SimpleIdentity drawableID;
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
class RemoveDrawableRequest : public DrawableRefChangeRequest {
public:
    RemoveDrawableRequest(util::SimpleIdentity id)
        : DrawableRefChangeRequest(id) {}
    RemoveDrawableRequest(const RemoveDrawableRequest &) = default;

    void execute(RenderOrchestrator &) override;
};

/**
    Change the color of all vertexes
 */
class ResetColorRequest : public DrawableRefChangeRequest {
public:
    ResetColorRequest(util::SimpleIdentity id, Color color)
        : DrawableRefChangeRequest(id),
          newColor(color) {}
    ResetColorRequest(const ResetColorRequest &other) = default;

    void execute(RenderOrchestrator &) override;

protected:
    Color newColor;
};

} // namespace mbgl
