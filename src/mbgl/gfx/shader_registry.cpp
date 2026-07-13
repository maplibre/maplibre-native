#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/shader_group.hpp>

namespace mbgl {
namespace gfx {

ShaderRegistry::ShaderRegistry()
    : legacyGroup() {}

ShaderGroup& ShaderRegistry::getLegacyGroup() noexcept {
    return legacyGroup;
}

bool ShaderRegistry::isShaderGroup(const std::string& shaderGroupName) const noexcept {
    std::shared_lock<std::shared_mutex> readerLock(shaderGroupLock);
    return shaderGroups.contains(shaderGroupName);
}

const ShaderGroupPtr ShaderRegistry::getShaderGroup(const std::string& shaderGroupName) const noexcept {
    std::shared_lock<std::shared_mutex> readerLock(shaderGroupLock);
    const auto it = shaderGroups.find(shaderGroupName);
    if (it == shaderGroups.end()) {
        return nullptr;
    }

    return it->second;
}

bool ShaderRegistry::replaceShader(ShaderGroupPtr&& shader, const std::string& shaderGroupName) noexcept {
    std::unique_lock<std::shared_mutex> writerLock(shaderGroupLock);
    if (!shaderGroups.contains(shaderGroupName)) {
        return false;
    }

    shaderGroups[shaderGroupName] = std::move(shader);
    return true;
}

bool ShaderRegistry::registerShaderGroup(ShaderGroupPtr&& shader, const std::string& shaderGroupName) noexcept {
    std::unique_lock<std::shared_mutex> writerLock(shaderGroupLock);
    if (shaderGroups.contains(shaderGroupName)) {
        return false;
    }

    shaderGroups.emplace(shaderGroupName, std::move(shader));
    return true;
}

} // namespace gfx
} // namespace mbgl
