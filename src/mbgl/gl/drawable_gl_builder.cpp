#include <mbgl/gl/drawable_gl_builder.hpp>

#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gl/drawable_gl.hpp>

namespace mbgl {
namespace gl {

gfx::DrawablePtr DrawableGLBuilder::createDrawable() const {
    return std::make_shared<DrawableGL>();
};

void DrawableGLBuilder::init() {
    const auto n = impl->vertices.elements();
    assert(n <= impl->indexes.elements());
    assert(n == impl->colors.size());

    // temp
    const auto vertSize = sizeof(Impl::VT) + 4 * sizeof(float);
    std::vector<uint8_t> rawVert(n * vertSize);

    auto p = &rawVert[0];
    for (auto i = 0ULL; i < n; ++i) {
        *(Impl::VT*)p = impl->vertices.at(i); p += sizeof(Impl::VT);
        *(float*)p = impl->colors[i].r; p += sizeof(float);
        *(float*)p = impl->colors[i].g; p += sizeof(float);
        *(float*)p = impl->colors[i].b; p += sizeof(float);
        *(float*)p = impl->colors[i].a; p += sizeof(float);
    }

    ((DrawableGL*)currentDrawable.get())->setVertData(std::move(rawVert), impl->indexes.vector());
}

} // namespace gl
} // namespace mbgl

