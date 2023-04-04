#pragma once

#include <mbgl/gfx/backend.hpp>

#include <array>
#include <cassert>
#include <string>

namespace mbgl {

class ProgramParameters {
public:
  struct ProgramSource {
    const gfx::Backend::Type backend;
    const std::string vertex;
    const std::string fragment;

    ProgramSource(gfx::Backend::Type forBackend, const std::string& vertex_,
                  const std::string& fragment_)
        : backend(forBackend), vertex(vertex_), fragment(fragment_)
    {
        assert(gfx::Backend::Type::TYPE_MAX != forBackend);
    }
  };

  ProgramParameters(float pixelRatio, bool overdraw);

  ProgramParameters
  withShaderSource(const ProgramSource& source) const noexcept;

  const std::string& getDefines() const;

  const std::string& vertexSource(gfx::Backend::Type backend) const noexcept;
  const std::string& fragmentSource(gfx::Backend::Type backend) const noexcept;

private:
  std::string defines;

  std::array<std::string, static_cast<size_t>(gfx::Backend::Type::TYPE_MAX)>
      vertexSources;

  std::array<std::string, static_cast<size_t>(gfx::Backend::Type::TYPE_MAX)>
      fragmentSources;
};

} // namespace mbgl
