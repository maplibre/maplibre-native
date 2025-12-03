#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader.hpp>

namespace mbgl {
namespace gfx {

bool ShaderGroup::isShader(const std::string& shaderName) const noexcept {
    std::shared_lock<std::shared_mutex> readerLock(programLock);
    return programs.contains(shaderName);
}

const std::shared_ptr<gfx::Shader> ShaderGroup::getShader(const std::string& shaderName) const noexcept {
    std::shared_lock<std::shared_mutex> readerLock(programLock);
    const auto it = programs.find(shaderName);
    if (it == programs.end()) {
        return nullptr;
    }

    return it->second;
}

bool ShaderGroup::replaceShader(std::shared_ptr<gfx::Shader>&& shader) noexcept {
    return replaceShader(std::move(shader), std::string{shader->typeName()});
}

bool ShaderGroup::replaceShader(std::shared_ptr<Shader>&& shader, const std::string& shaderName) noexcept {
    std::unique_lock<std::shared_mutex> writerLock(programLock);
    if (!programs.contains(shaderName)) {
        return false;
    }

    programs[shaderName] = std::move(shader);
    return true;
}

bool ShaderGroup::registerShader(std::shared_ptr<gfx::Shader>&& shader) noexcept {
    return registerShader(std::move(shader), std::string{shader->typeName()});
}

bool ShaderGroup::registerShader(std::shared_ptr<Shader>&& shader, const std::string& shaderName) noexcept {
    std::unique_lock<std::shared_mutex> writerLock(programLock);
    if (programs.contains(shaderName)) {
        return false;
    }

    programs.emplace(shaderName, std::move(shader));
    return true;
}

} // namespace gfx
} // namespace mbgl
