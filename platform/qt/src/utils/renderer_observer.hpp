#pragma once

#include <mbgl/actor/actor.hpp>
#include <mbgl/actor/actor_ref.hpp>
#include <mbgl/actor/mailbox.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/util/run_loop.hpp>

#include <memory>

namespace QMapLibre {

// Forwards RendererObserver signals to the given
// Delegate RendererObserver on the given RunLoop
class RendererObserver final : public mbgl::RendererObserver {
public:
    RendererObserver(mbgl::util::RunLoop &mapRunLoop, mbgl::RendererObserver &delegate_)
        : mailbox(std::make_shared<mbgl::Mailbox>(mapRunLoop)),
          delegate(delegate_, mailbox) {}

    ~RendererObserver() final { mailbox->close(); }

    void onInvalidate() final { delegate.invoke(&mbgl::RendererObserver::onInvalidate); }

    void onResourceError(std::exception_ptr err) final {
        delegate.invoke(&mbgl::RendererObserver::onResourceError, err);
    }

    void onWillStartRenderingMap() final { delegate.invoke(&mbgl::RendererObserver::onWillStartRenderingMap); }

    void onWillStartRenderingFrame() final { delegate.invoke(&mbgl::RendererObserver::onWillStartRenderingFrame); }

    void onDidFinishRenderingFrame(RenderMode mode,
                                   bool repaintNeeded,
                                   bool placementChanged,
                                   double frameEncodingTime,
                                   double frameRenderingTime) final {
        void (mbgl::RendererObserver::*f)(
            RenderMode, bool, bool, double, double) = &mbgl::RendererObserver::onDidFinishRenderingFrame;
        delegate.invoke(f, mode, repaintNeeded, placementChanged, frameEncodingTime, frameRenderingTime);
    }

    void onDidFinishRenderingMap() final { delegate.invoke(&mbgl::RendererObserver::onDidFinishRenderingMap); }

private:
    std::shared_ptr<mbgl::Mailbox> mailbox{};
    mbgl::ActorRef<mbgl::RendererObserver> delegate;
};

} // namespace QMapLibre
