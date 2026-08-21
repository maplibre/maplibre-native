#include <mbgl/test/util.hpp>

#include <mbgl/gfx/renderable.hpp>

namespace {

class TestRenderable final : public mln::gfx::Renderable {
public:
    explicit TestRenderable(mln::Size size_)
        : Renderable(size_, nullptr) {}

    void resize(const mln::Size& size_) { setRenderableSize(size_); }
};

} // namespace

TEST(Renderable, MaintainsCanonicalPhysicalSize) {
    TestRenderable renderable({256, 128});
    EXPECT_EQ(renderable.getSize(), (mln::Size{256, 128}));

    renderable.resize({512, 384});
    EXPECT_EQ(renderable.getSize(), (mln::Size{512, 384}));
}
