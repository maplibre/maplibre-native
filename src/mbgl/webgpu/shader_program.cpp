#include <mbgl/webgpu/shader_program.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

ShaderProgram::ShaderProgram(const std::string& name)
    : name_(name) {
    // TODO: Compile WGSL shaders and create pipeline
    Log::Debug(Event::General, "Creating WebGPU shader program: " + name);
}

ShaderProgram::~ShaderProgram() {
    // TODO: Release WebGPU pipeline and shaders
}

} // namespace webgpu
} // namespace mbgl