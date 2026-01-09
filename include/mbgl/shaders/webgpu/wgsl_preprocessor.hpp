#pragma once

#include <string>
#include <unordered_map>

namespace mbgl {
namespace webgpu {
namespace detail {

bool isDirective(const std::string& line, const std::string& directive);
std::string getDirectiveArgument(const std::string& line);
std::string preprocessWGSL(const std::string& source, const std::unordered_map<std::string, bool>& defines);

} // namespace detail
} // namespace webgpu
} // namespace mbgl
