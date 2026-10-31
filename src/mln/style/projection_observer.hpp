#pragma once

namespace mln {
namespace style {

class Projection;

class ProjectionObserver {
public:
    virtual ~ProjectionObserver() = default;

    virtual void onProjectionChanged(const Projection&) {}
};

} // namespace style
} // namespace mln
