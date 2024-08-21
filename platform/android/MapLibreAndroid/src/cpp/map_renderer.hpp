#pragma once

#include <mbgl/actor/actor_ref.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/image.hpp>

#include <memory>
#include <mutex>
#include <utility>
#include <optional>

#include <jni/jni.hpp>
#include <android/native_window.h>

namespace mbgl {

template <class>
class ActorRef;
class Mailbox;
class Renderer;
class RendererBackend;
class RendererObserver;
class ThreadPool;
class UpdateParameters;

namespace android {

class AndroidRendererBackend;

class AndroidSurface {
public:
    static constexpr auto Name() { return "android/view/Surface"; };
};

/**
 * The MapRenderer is a peer class that encapsulates the actions
 * performed on the GL Thread.
 *
 * The public methods are safe to call from the main thread, others are not.
 */
class MapRenderer : public Scheduler {
public:
    static constexpr auto Name() { return "org/maplibre/android/maps/renderer/MapRenderer"; };

    static void registerNative(jni::JNIEnv&);

    static MapRenderer& getNativePeer(JNIEnv&, const jni::Object<MapRenderer>&);

    MapRenderer(jni::JNIEnv& _env,
                const jni::Object<MapRenderer>&,
                jni::jfloat pixelRatio,
                const jni::String& localIdeographFontFamily);

    ~MapRenderer() override;

    // Resets the renderer to clean up on the calling thread
    void reset();

    // Takes the RendererObserver by shared_ptr so we
    // don't have to make the header public. Use
    // this instead of Renderer#setObserver directly
    void setObserver(std::shared_ptr<RendererObserver>);

    // Sets the new update parameters to use on subsequent
    // renders. Be sure to trigger a render with
    // requestRender().
    void update(std::shared_ptr<UpdateParameters>);

    // Gives a handle to the Renderer to enable actions on
    // any thread.
    ActorRef<Renderer> actor() const;

    // From Scheduler. Schedules by using callbacks to the
    // JVM to process the mailbox on the right thread.
    void schedule(std::function<void()>&& scheduled) override;
    void schedule(const util::SimpleIdentity, std::function<void()>&& fn) override { schedule(std::move(fn)); };

    mapbox::base::WeakPtr<Scheduler> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

    // Wait for the queue to be empty
    void waitForEmpty(const util::SimpleIdentity tag) override;

    void requestRender();

    // Snapshot - requires a RunLoop on the calling thread
    using SnapshotCallback = std::function<void(PremultipliedImage)>;
    void requestSnapshot(SnapshotCallback);

    AndroidRendererBackend& getRendererBackend() const { return *backend; }
    const TaggedScheduler& getThreadPool() const { return threadPool; }

protected:
    // Called from the GL Thread //

    void scheduleSnapshot(std::unique_ptr<SnapshotCallback>);

private:
    struct MailboxData {
        explicit MailboxData(Scheduler*);
        std::shared_ptr<Mailbox> getMailbox() const noexcept;

    private:
        Scheduler* scheduler;
        mutable std::shared_ptr<Mailbox> mailbox;
    };
    // Called from the GL Thread //

    // Resets the renderer
    void resetRenderer();

    // Renders a frame.
    void render(JNIEnv&);

    void onSurfaceCreated(JNIEnv&, const jni::Object<AndroidSurface>& surface);

    void onSurfaceChanged(JNIEnv&, jint width, jint height);

    void onSurfaceDestroyed(JNIEnv&);

private:
    // Called on either Main or GL thread //
    void onRendererReset(JNIEnv&);

    void setSwapBehaviorFlush(JNIEnv&, jboolean flush);

private:
    jni::WeakReference<jni::Object<MapRenderer>, jni::EnvAttachingDeleter> javaPeer;

    float pixelRatio;
    std::optional<std::string> localIdeographFontFamily;

    TaggedScheduler threadPool;
    const MailboxData mailboxData;

    std::mutex initialisationMutex;
    std::shared_ptr<RendererObserver> rendererObserver;

    std::unique_ptr<AndroidRendererBackend> backend;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<ActorRef<Renderer>> rendererRef;
    std::unique_ptr<ANativeWindow, std::function<void(ANativeWindow*)>> window;

    std::shared_ptr<UpdateParameters> updateParameters;
    std::mutex updateMutex;

    bool framebufferSizeChanged = false;
    std::atomic<bool> destroyed{false};
    bool swapBehaviorFlush{true};

    std::unique_ptr<SnapshotCallback> snapshotCallback;
    mapbox::base::WeakPtrFactory<Scheduler> weakFactory{this};
};

} // namespace android
} // namespace mbgl
