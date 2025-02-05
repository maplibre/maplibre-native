#include "map_renderer.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/run_loop.hpp>

#include <string>
#include <android/native_window_jni.h>

#include "attach_env.hpp"
#include "android_renderer_backend.hpp"
#include "map_renderer_runnable.hpp"

namespace mbgl {
namespace android {

MapRenderer::MapRenderer(jni::JNIEnv& _env,
                         const jni::Object<MapRenderer>& obj,
                         jni::jfloat pixelRatio_,
                         const jni::String& localIdeographFontFamily_)
    : javaPeer(_env, obj),
      pixelRatio(pixelRatio_),
      localIdeographFontFamily(localIdeographFontFamily_ ? jni::Make<std::string>(_env, localIdeographFontFamily_)
                                                         : std::optional<std::string>{}),
      threadPool(Scheduler::GetBackground(), {}),
      mailboxData(this) {}

MapRenderer::MailboxData::MailboxData(Scheduler* scheduler_)
    : scheduler(scheduler_) {
    assert(scheduler);
}

std::shared_ptr<Mailbox> MapRenderer::MailboxData::getMailbox() const noexcept {
    if (!mailbox) {
        mailbox = std::make_shared<Mailbox>(*scheduler);
    }
    return mailbox;
}

MapRenderer::~MapRenderer() = default;

void MapRenderer::reset() {
    try {
        destroyed = true;

        if (renderer) {
            // Make sure to destroy the renderer on the GL Thread
            auto self = ActorRef<MapRenderer>(*this, mailboxData.getMailbox());
            self.ask(&MapRenderer::resetRenderer).wait();
        }

        // Lock to make sure there is no concurrent initialisation on the gl thread
        std::lock_guard<std::mutex> lock(initialisationMutex);
        rendererObserver.reset();
    } catch (const std::exception& exception) {
        Log::Error(Event::Android, std::string("MapRenderer::reset failed: ") + exception.what());
    }
}

ActorRef<Renderer> MapRenderer::actor() const {
    return *rendererRef;
}

void MapRenderer::schedule(std::function<void()>&& scheduled) {
    MLN_TRACE_FUNC();
    try {
        // Create a runnable
        android::UniqueEnv _env = android::AttachEnv();
        auto runnable = std::make_unique<MapRendererRunnable>(*_env, std::move(scheduled));

        // Obtain ownership of the peer (gets transferred to the MapRenderer on the JVM for later GC)
        auto peer = runnable->peer();

        // Queue the event on the Java Peer
        static auto& javaClass = jni::Class<MapRenderer>::Singleton(*_env);
        static auto queueEvent = javaClass.GetMethod<void(jni::Object<MapRendererRunnable>)>(*_env, "queueEvent");
        auto weakReference = javaPeer.get(*_env);
        if (weakReference) {
            MLN_TRACE_ZONE(java);
            weakReference.Call(*_env, queueEvent, peer);
        }

        // Release the c++ peer as it will be destroyed on GC of the Java Peer
        runnable.release();
    } catch (const std::exception& exception) {
        Log::Error(Event::Android, std::string("MapRenderer::schedule failed: ") + exception.what());
    }
}

void MapRenderer::waitForEmpty([[maybe_unused]] const util::SimpleIdentity tag) {
    try {
        android::UniqueEnv _env = android::AttachEnv();
        static auto& javaClass = jni::Class<MapRenderer>::Singleton(*_env);
        static auto waitForEmpty = javaClass.GetMethod<void()>(*_env, "waitForEmpty");
        if (auto weakReference = javaPeer.get(*_env)) {
            return weakReference.Call(*_env, waitForEmpty);
        }
    } catch (...) {
        Log::Error(Event::Android, "MapRenderer::waitForEmpty failed");
        jni::ThrowJavaError(*android::AttachEnv(), std::current_exception());
    }
}

void MapRenderer::requestRender() {
    try {
        android::UniqueEnv _env = android::AttachEnv();
        static auto& javaClass = jni::Class<MapRenderer>::Singleton(*_env);
        static auto onInvalidate = javaClass.GetMethod<void()>(*_env, "requestRender");
        auto weakReference = javaPeer.get(*_env);
        if (weakReference) {
            weakReference.Call(*_env, onInvalidate);
        }
    } catch (const std::exception& exception) {
        Log::Error(Event::Android, std::string("MapRenderer::requestRender failed: ") + exception.what());
    }
}

void MapRenderer::update(std::shared_ptr<UpdateParameters> params) {
    try {
        // Lock on the parameters
        std::lock_guard<std::mutex> lock(updateMutex);
        updateParameters = std::move(params);
    } catch (const std::exception& exception) {
        Log::Error(Event::Android, std::string("MapRenderer::update failed: ") + exception.what());
    }
}

void MapRenderer::setObserver(std::shared_ptr<RendererObserver> _rendererObserver) {
    try {
        // Lock as the initialization can come from the main thread or the GL thread first
        std::lock_guard<std::mutex> lock(initialisationMutex);

        rendererObserver = std::move(_rendererObserver);

        // Set the new observer on the Renderer implementation
        if (renderer) {
            renderer->setObserver(rendererObserver.get());
        }
    } catch (const std::exception& exception) {
        Log::Error(Event::Android, std::string("MapRenderer::setObserver failed: ") + exception.what());
    }
}

void MapRenderer::requestSnapshot(SnapshotCallback callback) {
    auto self = ActorRef<MapRenderer>(*this, mailboxData.getMailbox());
    self.invoke(
        &MapRenderer::scheduleSnapshot,
        std::make_unique<SnapshotCallback>(
            [&, callback = std::move(callback), runloop = util::RunLoop::Get()](PremultipliedImage image) {
                runloop->invoke(
                    [callback = std::move(callback), image = std::move(image), renderer = std::move(this)]() mutable {
                        if (renderer && !renderer->destroyed) {
                            callback(std::move(image));
                        }
                    });
                snapshotCallback.reset();
            }));
}

// Called on OpenGL thread //

void MapRenderer::resetRenderer() {
    renderer.reset();
    swapBehaviorFlush = false;
}

void MapRenderer::scheduleSnapshot(std::unique_ptr<SnapshotCallback> callback) {
    snapshotCallback = std::move(callback);
    requestRender();
}

void MapRenderer::render(JNIEnv&) {
    assert(renderer);

    std::shared_ptr<UpdateParameters> params;
    {
        // Lock on the parameters
        std::unique_lock<std::mutex> lock(updateMutex);
        if (!updateParameters) return;

        // Hold on to the update parameters during render
        params = updateParameters;
    }

    // Activate the backend
    assert(backend);
    gfx::BackendScope backendGuard{backend->getImpl()};

    // Ensure that the "current" scheduler on the render thread is
    // this scheduler.
    Scheduler::SetCurrent(this);

    if (framebufferSizeChanged) {
        backend->updateViewPort();
        framebufferSizeChanged = false;
    }

    renderer->render(params);

    // Deliver the snapshot if requested
    if (snapshotCallback) {
        snapshotCallback->operator()(backend->readFramebuffer());
        snapshotCallback.reset();
    }
}

void MapRenderer::onSurfaceCreated(JNIEnv& env, const jni::Object<AndroidSurface>& surface) {
    // Lock as the initialization can come from the main thread or the GL thread first
    std::lock_guard<std::mutex> lock(initialisationMutex);

    // The android system will have already destroyed the underlying
    // GL resources if this is not the first initialization and an
    // attempt to clean them up will fail
    if (backend) backend->markContextLost();
    if (renderer) renderer->markContextLost();

    // Reset in opposite order
    renderer.reset();
    backend.reset();
    window.reset();

    if (surface) {
        window = std::unique_ptr<ANativeWindow, std::function<void(ANativeWindow*)>>(
            ANativeWindow_fromSurface(&env, reinterpret_cast<jobject>(surface.get())),
            [](ANativeWindow* window_) { ANativeWindow_release(window_); });
    }

    // Create the new backend and renderer
    backend = AndroidRendererBackend::Create(window.get());
    renderer = std::make_unique<Renderer>(backend->getImpl(), pixelRatio, localIdeographFontFamily);
    rendererRef = std::make_unique<ActorRef<Renderer>>(*renderer, mailboxData.getMailbox());

    backend->setSwapBehavior(swapBehaviorFlush ? gfx::Renderable::SwapBehaviour::Flush
                                               : gfx::Renderable::SwapBehaviour::NoFlush);

    // Set the observer on the new Renderer implementation
    if (rendererObserver) {
        renderer->setObserver(rendererObserver.get());
    }
}

void MapRenderer::onSurfaceChanged(JNIEnv& env, jint width, jint height) {
    if (!renderer) {
        // In case the surface has been destroyed (due to app back-grounding)
        jni::jobject* nullObj = nullptr;
        onSurfaceCreated(env, jni::Object<AndroidSurface>(nullObj));
    }

    backend->resizeFramebuffer(width, height);
    framebufferSizeChanged = true;
    requestRender();
}

void MapRenderer::onRendererReset(JNIEnv&) {
    // Make sure to destroy the renderer on the GL Thread
    auto self = ActorRef<MapRenderer>(*this, mailboxData.getMailbox());
    self.ask(&MapRenderer::resetRenderer).wait();
}

// needs to be called on GL thread
void MapRenderer::onSurfaceDestroyed(JNIEnv&) {
    resetRenderer();
    window.reset();
}

void MapRenderer::setSwapBehaviorFlush(JNIEnv&, jboolean flush) {
    swapBehaviorFlush = flush;

    if (backend) {
        backend->setSwapBehavior(flush ? gfx::Renderable::SwapBehaviour::Flush
                                       : gfx::Renderable::SwapBehaviour::NoFlush);
    }
}

// Static methods //

void MapRenderer::registerNative(jni::JNIEnv& env) {
    // Lookup the class
    static auto& javaClass = jni::Class<MapRenderer>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    // Register the peer
    jni::RegisterNativePeer<MapRenderer>(
        env,
        javaClass,
        "nativePtr",
        jni::MakePeer<MapRenderer, const jni::Object<MapRenderer>&, jni::jfloat, const jni::String&>,
        "nativeInitialize",
        "finalize",
        METHOD(&MapRenderer::render, "nativeRender"),
        METHOD(&MapRenderer::onRendererReset, "nativeReset"),
        METHOD(&MapRenderer::onSurfaceCreated, "nativeOnSurfaceCreated"),
        METHOD(&MapRenderer::onSurfaceChanged, "nativeOnSurfaceChanged"),
        METHOD(&MapRenderer::onSurfaceDestroyed, "nativeOnSurfaceDestroyed"),
        METHOD(&MapRenderer::setSwapBehaviorFlush, "nativeSetSwapBehaviorFlush"));
}

MapRenderer& MapRenderer::getNativePeer(JNIEnv& env, const jni::Object<MapRenderer>& jObject) {
    static auto& javaClass = jni::Class<MapRenderer>::Singleton(env);
    static auto field = javaClass.GetField<jlong>(env, "nativePtr");
    MapRenderer* mapRenderer = reinterpret_cast<MapRenderer*>(jObject.Get(env, field));
    assert(mapRenderer != nullptr);
    return *mapRenderer;
}

} // namespace android
} // namespace mbgl
