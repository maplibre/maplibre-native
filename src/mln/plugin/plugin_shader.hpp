#pragma once

#include <string>

namespace mln {
class ProgramParameters;
namespace gfx {
class ShaderRegistry;
}
namespace plugin {

std::string shaderGroupName(const std::string& pluginID, const std::string& shaderID);
void registerPluginShaderGroups(gfx::ShaderRegistry&, const ProgramParameters&);

} // namespace plugin
} // namespace mln
