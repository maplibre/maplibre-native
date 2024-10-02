#pragma once

#include <mbgl/util/string.hpp>

namespace mbgl {

template <typename>
class ActorRef;
class FileSourceRequest;

void requestLocalFile(const std::string& path,
                      const ActorRef<FileSourceRequest>& req,
                      const std::optional<std::pair<uint64_t, uint64_t>>& dataRange = std::nullopt);

} // namespace mbgl
