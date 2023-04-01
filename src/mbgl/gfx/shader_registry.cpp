#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/shader.hpp>

namespace mbgl {
namespace gfx {

ShaderRegistry::ShaderRegistry() {}

bool ShaderRegistry::isShader(const std::string& shaderName)
    const noexcept
{
    std::shared_lock<std::shared_mutex> readerLock(programLock);
    return programs.find(shaderName) != programs.end();
}

const std::shared_ptr<gfx::Shader> ShaderRegistry::getShader(const std::string& shaderName)
    const noexcept
{
    std::shared_lock<std::shared_mutex> readerLock(programLock);
    const auto it = programs.find(shaderName);
    if (it == programs.end()) {
        return nullptr;
    }

    return it->second;
}

bool ShaderRegistry::replaceShader(
    std::shared_ptr<gfx::Shader>&& shader) noexcept
{
    std::unique_lock<std::shared_mutex> writerLock(programLock);
    const std::string programName{shader->name()}; // TODO: C++ 20 heterogenous lookup
    if (programs.find(programName) == programs.end()) {
        return false;
    }

    programs[programName] = std::move(shader);
    return true;
}

bool ShaderRegistry::registerShader(
    std::shared_ptr<gfx::Shader>&& shader) noexcept
{
    std::unique_lock<std::shared_mutex> writerLock(programLock);
    const std::string programName{shader->name()};
    if (programs.find(programName) != programs.end()) {
        return false;
    }

    programs.emplace(programName, std::move(shader));
    return true;
}

} // namespace gl
} // namespace mbgl
