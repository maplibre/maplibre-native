#include <mln/style/projection_definition.hpp>

namespace mln {

ProjectionDefinition::ProjectionDefinition(std::string type)
    : from(type),
      to(std::move(type)) {}

ProjectionDefinition::ProjectionDefinition(std::string from_, std::string to_, double transition_)
    : from(std::move(from_)),
      to(std::move(to_)),
      transition(transition_) {}

mln::Value ProjectionDefinition::serialize() const {
    if (from == to && transition == 1) {
        return from;
    }
    return std::vector<mln::Value>{from, to, transition};
}

} // namespace mln
