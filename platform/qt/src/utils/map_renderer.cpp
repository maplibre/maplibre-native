#include "map_renderer.hpp"
#include "scheduler.hpp"

#include <mbgl/gfx/backend_scope.hpp>

#include <QThreadStorage>

namespace {

static bool needsToForceScheduler() {
    static QThreadStorage<bool> force;

    if (!force.hasLocalData()) {
        force.setLocalData(mbgl::Scheduler::GetCurrent() == nullptr);
    }

    return force.localData();
};

static auto *getScheduler() {
    static QThreadStorage<std::shared_ptr<QMapLibreScheduler>> scheduler;

    if (!scheduler.hasLocalData()) {
        scheduler.setLocalData(std::make_shared<QMapLibreScheduler>());
    }

    return scheduler.localData().get();
};

} // namespace


QMapLibreMapRenderer::QMapLibreMapRenderer(qreal pixelRatio, QMapLibreSettings::GLContextMode mode, const QString &localFontFamily)
    : m_backend(static_cast<mbgl::gfx::ContextMode>(mode)),
      m_renderer(std::make_unique<mbgl::Renderer>(m_backend, pixelRatio,
                 localFontFamily.isEmpty() ? mbgl::nullopt : mbgl::optional<std::string> { localFontFamily.toStdString() }))
    , m_forceScheduler(needsToForceScheduler())
{
    // If we don't have a Scheduler on this thread, which
    // is usually the case for render threads, use a shared
    // dummy scheduler that needs to be explicitly forced to
    // process events.
    if (m_forceScheduler) {
        auto scheduler = getScheduler();

        if (mbgl::Scheduler::GetCurrent() == nullptr) {
            mbgl::Scheduler::SetCurrent(scheduler);
        }

        connect(scheduler, &QMapLibreScheduler::needsProcessing, this, &QMapLibreMapRenderer::needsRendering);
    }
}

QMapLibreMapRenderer::~QMapLibreMapRenderer()
{
    MBGL_VERIFY_THREAD(tid);
}

void QMapLibreMapRenderer::updateParameters(std::shared_ptr<mbgl::UpdateParameters> newParameters)
{
    std::lock_guard<std::mutex> lock(m_updateMutex);
    m_updateParameters = std::move(newParameters);
}

void QMapLibreMapRenderer::updateFramebuffer(quint32 fbo, const mbgl::Size &size)
{
    MBGL_VERIFY_THREAD(tid);

    m_backend.updateFramebuffer(fbo, size);
}

void QMapLibreMapRenderer::render()
{
    MBGL_VERIFY_THREAD(tid);

    std::shared_ptr<mbgl::UpdateParameters> params;
    {
        // Lock on the parameters
        std::lock_guard<std::mutex> lock(m_updateMutex);

        // UpdateParameters should always be available when rendering.
        assert(m_updateParameters);

        // Hold on to the update parameters during render
        params = m_updateParameters;
    }

    // The OpenGL implementation automatically enables the OpenGL context for us.
    mbgl::gfx::BackendScope scope(m_backend, mbgl::gfx::BackendScope::ScopeType::Implicit);

    m_renderer->render(params);

    if (m_forceScheduler) {
        getScheduler()->processEvents();
    }
}

void QMapLibreMapRenderer::setObserver(std::shared_ptr<mbgl::RendererObserver> observer)
{
    m_renderer->setObserver(observer.get());
}
