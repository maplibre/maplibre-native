#pragma once

#include <mln/util/feature.hpp>

#include <string>

namespace mln {

/// The value of the style's `projection.type`: a single projection, or a transition between two.
struct ProjectionDefinition {
    ProjectionDefinition() = default;
    explicit ProjectionDefinition(std::string type);
    ProjectionDefinition(std::string from_, std::string to_, double transition_);

    std::string from = "mercator";
    std::string to = "mercator";
    double transition = 1;

    bool operator==(const ProjectionDefinition&) const = default;

    mln::Value serialize() const;
};

} // namespace mln
