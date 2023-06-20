#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/shader.hpp>

namespace mbgl {
namespace gfx {

ShaderRegistry::ShaderRegistry() {}

bool ShaderRegistry::isShader(const std::string& shaderName) const noexcept {
    std::shared_lock<std::shared_mutex> readerLock(programLock);
    return programs.find(shaderName) != programs.end();
}

const std::shared_ptr<gfx::Shader> ShaderRegistry::getShader(const std::string& shaderName) const noexcept {
    std::shared_lock<std::shared_mutex> readerLock(programLock);
    const auto it = programs.find(shaderName);
    if (it == programs.end()) {
        return nullptr;
    }

    return it->second;
}

bool ShaderRegistry::replaceShader(std::shared_ptr<gfx::Shader>&& shader) noexcept {
    return replaceShader(std::move(shader), std::string{shader->typeName()});
}

bool ShaderRegistry::replaceShader(std::shared_ptr<Shader>&& shader, const std::string& shaderName) noexcept {
    std::unique_lock<std::shared_mutex> writerLock(programLock);
    if (programs.find(shaderName) == programs.end()) {
        return false;
    }

    programs[shaderName] = std::move(shader);
    return true;
}

bool ShaderRegistry::registerShader(std::shared_ptr<gfx::Shader>&& shader) noexcept {
    return registerShader(std::move(shader), std::string{shader->typeName()});
}

bool ShaderRegistry::registerShader(std::shared_ptr<Shader>&& shader, const std::string& shaderName) noexcept {
    std::unique_lock<std::shared_mutex> writerLock(programLock);
    if (programs.find(shaderName) != programs.end()) {
        return false;
    }

    programs.emplace(shaderName, std::move(shader));
    return true;
}

} // namespace gfx
} // namespace mbgl
