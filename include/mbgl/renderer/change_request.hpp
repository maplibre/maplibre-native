#pragma once

#include <mbgl/util/identity.hpp>

#include <memory>
#include <vector>

namespace mbgl {

class RenderOrchestrator;

namespace gfx {
    class Drawable;
    using DrawablePtr = std::shared_ptr<Drawable>;
}   // namespace gfx


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
    Add a new drawable to the scene
 */
class AddDrawableRequest : public ChangeRequest {
public:
    AddDrawableRequest(gfx::DrawablePtr drawable_)
        : drawable(std::move(drawable_)) {
    }
    AddDrawableRequest(AddDrawableRequest&& other)
        : drawable(std::move(other.drawable)) {
    }

    void execute(RenderOrchestrator &) override;

protected:
    gfx::DrawablePtr drawable;
};


/**
    Remove a drawable from the scene
 */
class RemoveDrawableRequest : public ChangeRequest {
public:
    RemoveDrawableRequest(util::SimpleIdentity drawableId_) : drawableId(drawableId_) { }
    RemoveDrawableRequest(const RemoveDrawableRequest& other) : drawableId(other.drawableId) { }

    void execute(RenderOrchestrator &) override;

protected:
    util::SimpleIdentity drawableId;
};


} // namespace mbgl
