#pragma once

#include <memory>

namespace mbgl {

namespace gfx {
    class Drawable;
    using DrawablePtr = std::shared_ptr<Drawable>;
}   // namespace gfx

class RenderOrchestrator;

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
    // Lacking a unique identifier, we need an actual reference to the drawable for now.
    RemoveDrawableRequest(gfx::DrawablePtr drawable_)
        : drawable(std::move(drawable_)) {
    }
    RemoveDrawableRequest(RemoveDrawableRequest&& other)
        : drawable(std::move(other.drawable)) {
    }

    void execute(RenderOrchestrator &) override;

protected:
    gfx::DrawablePtr drawable;
};


} // namespace mbgl
