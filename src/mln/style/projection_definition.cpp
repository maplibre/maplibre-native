#include <mln/style/projection_definition.hpp>

#include <cassert>

namespace mln {

namespace {

constexpr std::string_view mercatorName = "mercator";
constexpr std::string_view verticalPerspectiveName = "vertical-perspective";
constexpr std::string_view globeName = "globe";

ProjectionType typeFromNameOrMercator(std::string_view name) {
    const auto type = projectionTypeFromName(name);
    assert(type);
    return type.value_or(ProjectionType::Mercator);
}

} // namespace

std::string_view projectionTypeName(ProjectionType type) {
    switch (type) {
        case ProjectionType::Mercator:
            return mercatorName;
        case ProjectionType::VerticalPerspective:
            return verticalPerspectiveName;
        case ProjectionType::Globe:
            return globeName;
    }
    assert(false);
    return mercatorName;
}

std::optional<ProjectionType> projectionTypeFromName(std::string_view name) {
    if (name == mercatorName) return ProjectionType::Mercator;
    if (name == verticalPerspectiveName) return ProjectionType::VerticalPerspective;
    if (name == globeName) return ProjectionType::Globe;
    return std::nullopt;
}

ProjectionDefinition::ProjectionDefinition(ProjectionType type)
    : from(type),
      to(type) {}

ProjectionDefinition::ProjectionDefinition(ProjectionType from_, ProjectionType to_, double transition_)
    : from(from_),
      to(to_),
      transition(transition_) {}

ProjectionDefinition::ProjectionDefinition(std::string_view name)
    : ProjectionDefinition(typeFromNameOrMercator(name)) {}

ProjectionDefinition::ProjectionDefinition(std::string_view from_, std::string_view to_, double transition_)
    : ProjectionDefinition(typeFromNameOrMercator(from_), typeFromNameOrMercator(to_), transition_) {}

mln::Value ProjectionDefinition::serialize() const {
    if (from == to && transition == 1) {
        return std::string(projectionTypeName(from));
    }
    return std::vector<mln::Value>{
        std::string(projectionTypeName(from)), std::string(projectionTypeName(to)), transition};
}

} // namespace mln
