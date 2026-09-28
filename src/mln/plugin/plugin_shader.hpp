#pragma once

#include <string>
#include <memory>

namespace mln {
class ProgramParameters;
namespace gfx {
class ShaderRegistry;
}
namespace plugin {
struct RegisteredLayer;

std::string shaderGroupName(const std::string& pluginID, const std::string& layerType, const std::string& shaderID);
void registerPluginShaderGroups(gfx::ShaderRegistry&,
                                const ProgramParameters&,
                                const std::shared_ptr<const RegisteredLayer>&);

} // namespace plugin
} // namespace mln
