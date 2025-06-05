#include <mbgl/shaders/gl/legacy/programs.hpp>
#include <mbgl/shaders/gl/legacy/clipping_mask_program.hpp>
#include <mbgl/util/logging.hpp>
#include <exception>

namespace mbgl {

Programs::Programs(const ProgramParameters& programParameters_)
    : programParameters(programParameters_) {}

Programs::~Programs() = default;

/// @brief Register a list of types with a shader registry instance
/// @tparam ...T Type list parameter pack
/// @param registry A shader registry instance
/// programParameters_ ProgramParameters used to initialize each instance
template <typename... T>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters_) {
    /// The following fold expression will create a shared_ptr for every type
    /// in the parameter pack and register it with the shader registry.

    /// Registration calls are wrapped in a lambda that throws on registration
    /// failure, we shouldn't expect registration to faill unless the shader
    /// registry instance provided already has conflicting programs present.
    (
        [](bool expr) {
            if (!expr) {
                throw std::runtime_error("Failed to register " + std::string(T::Name) + " with shader registry!");
            }
        }(registry.getLegacyGroup().registerShader(std::make_shared<T>(programParameters_))),
        ...);
}

void Programs::registerWith(gfx::ShaderRegistry& registry) {
    /// The following types will be registered
    registerTypes<ClippingMaskProgram>(registry, programParameters);
}

} // namespace mbgl
