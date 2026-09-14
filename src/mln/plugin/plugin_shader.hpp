#pragma once

#include <string>

namespace mln {
class ProgramParameters;
namespace gfx {
class ShaderRegistry;
}
namespace plugin {
struct LayerType;

std::string shaderGroupName(const std::string& pluginID, const std::string& layerType, const std::string& shaderID);
void registerPluginShaderGroups(gfx::ShaderRegistry&, const ProgramParameters&);
void registerPluginShaderGroups(gfx::ShaderRegistry&, const ProgramParameters&, const LayerType&);

} // namespace plugin
} // namespace mln
