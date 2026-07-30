#pragma once

#include <mbgl/util/size.hpp>

#include <cstdint>

namespace mbgl {
namespace gl {

class Context;

/// A GL_TEXTURE_2D_ARRAY of RGBA8 layers, all the same size. Used to pack many
/// per-tile textures (e.g. terrain DEM) into one texture object so that a single
/// instanced draw can sample a different layer per instance (sampler2DArray),
/// collapsing N per-tile draws into one.
///
/// Intentionally minimal and GL-only (not part of the gfx abstraction): it exists
/// to back terrain instancing. Storage is immutable (glTexStorage3D); allocate()
/// recreates the object if the size or layer count changes.
class Texture2DArray {
public:
    explicit Texture2DArray(Context& context_)
        : context(context_) {}
    ~Texture2DArray();

    Texture2DArray(const Texture2DArray&) = delete;
    Texture2DArray& operator=(const Texture2DArray&) = delete;

    /// Ensure immutable RGBA8 storage for `layers` layers of `size`. Recreates the
    /// underlying texture when the requested size or layer count differs.
    void allocate(Size size, uint32_t layers);

    /// Upload one layer's RGBA8 pixels (row-major, size.width*size.height*4 bytes).
    void uploadLayer(uint32_t layer, const void* rgbaData);

    /// Bind to `textureUnit` and point the sampler at `location` to it.
    void bind(int32_t location, int32_t textureUnit);

    uint32_t getLayerCount() const noexcept { return layers; }
    Size getSize() const noexcept { return size; }
    bool valid() const noexcept { return id != 0; }

private:
    void destroy() noexcept;

    Context& context;
    uint32_t id = 0;
    Size size{0, 0};
    uint32_t layers = 0;
};

} // namespace gl
} // namespace mbgl
