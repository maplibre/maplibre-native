#pragma once

#include <mln/gfx/renderbuffer.hpp>
#include <mln/gl/object.hpp>

namespace mln {
namespace gl {

class RenderbufferResource final : public gfx::RenderbufferResource {
public:
    explicit RenderbufferResource(UniqueRenderbuffer renderbuffer_)
        : renderbuffer(std::move(renderbuffer_)) {}

    ~RenderbufferResource() noexcept override;

    UniqueRenderbuffer renderbuffer;
};

} // namespace gl
} // namespace mln
