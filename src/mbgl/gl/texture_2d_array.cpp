#include <mbgl/gl/texture_2d_array.hpp>

#include <mbgl/gl/context.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/util/instrumentation.hpp>

namespace mln {
namespace gl {

using namespace platform;

Texture2DArray::~Texture2DArray() {
    destroy();
}

void Texture2DArray::destroy() noexcept {
    if (id != 0) {
        GLuint tex = id;
        MBGL_CHECK_ERROR(glDeleteTextures(1, &tex));
        id = 0;
    }
}

void Texture2DArray::allocate(Size size_, uint32_t layers_) {
    MLN_TRACE_FUNC();

    // Immutable storage can only be specified once, so recreate on any change.
    if (id != 0 && (size_ != size || layers_ != layers)) {
        destroy();
    }
    if (id != 0) {
        return; // already allocated at this size/layer count
    }

    size = size_;
    layers = layers_;
    if (size.width == 0 || size.height == 0 || layers == 0) {
        return;
    }

    GLuint tex = 0;
    MBGL_CHECK_ERROR(glGenTextures(1, &tex));
    id = tex;

    // Bind the array on a scratch unit; the context tracks only GL_TEXTURE_2D
    // bindings, so binding GL_TEXTURE_2D_ARRAY here does not disturb that state.
    context.activeTextureUnit = 0;
    MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, id));
    MBGL_CHECK_ERROR(glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                                    1,
                                    GL_RGBA8,
                                    static_cast<GLsizei>(size.width),
                                    static_cast<GLsizei>(size.height),
                                    static_cast<GLsizei>(layers)));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    context.renderingStats().numCreatedTextures++;
}

void Texture2DArray::uploadLayer(uint32_t layer, const void* rgbaData) {
    MLN_TRACE_FUNC();
    if (id == 0 || layer >= layers || rgbaData == nullptr) {
        return;
    }

    context.activeTextureUnit = 0;
    MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, id));
    context.pixelStoreUnpack = {1};
    MBGL_CHECK_ERROR(glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                                     0,
                                     0,
                                     0,
                                     static_cast<GLint>(layer),
                                     static_cast<GLsizei>(size.width),
                                     static_cast<GLsizei>(size.height),
                                     1,
                                     GL_RGBA,
                                     GL_UNSIGNED_BYTE,
                                     rgbaData));

    context.renderingStats().numTextureUpdates++;
    context.renderingStats().textureUpdateBytes += static_cast<std::size_t>(4 * size.width * size.height);
}

void Texture2DArray::bind(int32_t location, int32_t textureUnit) {
    if (id == 0) {
        return;
    }
    context.activeTextureUnit = static_cast<uint8_t>(textureUnit);
    MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, id));
    MBGL_CHECK_ERROR(glUniform1i(location, textureUnit));
}

} // namespace gl
} // namespace mln
