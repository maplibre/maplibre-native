#include <mln/gl/attribute.hpp>
#include <mln/gl/context.hpp>

namespace mln {
namespace gl {

using namespace platform;

std::optional<AttributeLocation> queryLocation(ProgramID id, const char* name) {
    GLint attributeLocation = MBGL_CHECK_ERROR(glGetAttribLocation(id, name));
    if (attributeLocation != -1) {
        return attributeLocation;
    } else {
        return {};
    }
}

} // namespace gl
} // namespace mln
