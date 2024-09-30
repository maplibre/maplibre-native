#pragma once

#include <mbgl/util/string.hpp>

namespace mbgl {

template <typename>
class ActorRef;
class FileSourceRequest;

void requestLocalFile(const std::string&,
                      const ActorRef<FileSourceRequest>&,
                      const std::optional<std::pair<uint64_t, uint64_t>>& = std::nullopt);

} // namespace mbgl
