#if MLN_RENDER_BACKEND_WEBGPU
#include <mln/test/util.hpp>

#include <mln/gfx/context_observer.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/render_pass.hpp>
#include <mln/webgpu/context.hpp>
#include <mln/webgpu/drawable.hpp>
#include <mln/webgpu/renderer_backend.hpp>

#include <cstdlib>
#include <stdexcept>
#include <vector>

using namespace mln;

namespace {

// Layer groups don't touch the device, so the backend needs none.
class StubRendererBackend final : public webgpu::RendererBackend {
public:
    StubRendererBackend()
        : webgpu::RendererBackend(gfx::ContextMode::Unique) {}

    gfx::Renderable& getDefaultRenderable() override { std::abort(); }
};

class RecordingObserver final : public gfx::ContextObserver {
public:
    void onRenderError(std::exception_ptr error) override { errors.push_back(std::move(error)); }

    std::vector<std::exception_ptr> errors;
};

gfx::UniqueDrawable makeDrawable(const char* name) {
    return std::make_unique<webgpu::Drawable>(name);
}

std::string message(const std::exception_ptr& error) {
    try {
        std::rethrow_exception(error);
    } catch (const std::exception& e) {
        return e.what();
    }
}

// Fails the first drawable it visits and counts the visits.
struct FailFirst {
    std::size_t& visits;

    void operator()(gfx::Drawable& drawable) {
        if (visits++ == 0) {
            throw std::runtime_error(drawable.getName() + " failed");
        }
    }
};

} // namespace

// An exception thrown while a layer group visits its drawables, such as an
// upload's std::bad_alloc, reaches the context observer instead of a null one.
TEST(WebGPULayerGroup, TileLayerGroupReportsDrawableErrors) {
    StubRendererBackend backend;
    webgpu::Context context(backend);
    RecordingObserver observer;
    context.setObserver(&observer);

    auto group = context.createTileLayerGroup(0, 2, "tiles");
    group->addDrawable(RenderPass::Opaque, OverscaledTileID(0, 0, 0), makeDrawable("first"));
    group->addDrawable(RenderPass::Opaque, OverscaledTileID(1, 0, 0), makeDrawable("second"));

    std::size_t visits = 0;
    group->visitDrawables(FailFirst{visits});

    EXPECT_EQ(2u, visits);
    ASSERT_EQ(1u, observer.errors.size());
    EXPECT_EQ("first failed", message(observer.errors[0]));
    EXPECT_EQ(1u, group->getDrawableCount());
}

TEST(WebGPULayerGroup, LayerGroupReportsDrawableErrors) {
    StubRendererBackend backend;
    webgpu::Context context(backend);
    RecordingObserver observer;
    context.setObserver(&observer);

    auto group = context.createLayerGroup(0, 2, "layer");
    group->addDrawable(makeDrawable("first"));
    group->addDrawable(makeDrawable("second"));

    std::size_t visits = 0;
    group->visitDrawables(FailFirst{visits});

    EXPECT_EQ(2u, visits);
    ASSERT_EQ(1u, observer.errors.size());
    EXPECT_EQ("first failed", message(observer.errors[0]));
    EXPECT_EQ(1u, group->getDrawableCount());
}

#endif
