#pragma once

#include "qmaplibresettings.hpp"
#include "renderer_backend.hpp"

#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/util/util.hpp>

#include <QtGlobal>
#include <QObject>

#include <memory>
#include <mutex>

namespace mbgl {
class Renderer;
class UpdateParameters;
} // namespace mbgl

class QMapLibreRendererBackend;

class QMapLibreMapRenderer : public QObject
{
    Q_OBJECT

public:
    QMapLibreMapRenderer(qreal pixelRatio, QMapLibreSettings::GLContextMode, const QString &localFontFamily);
    virtual ~QMapLibreMapRenderer();

    void render();
    void updateFramebuffer(quint32 fbo, const mbgl::Size &size);
    void setObserver(std::shared_ptr<mbgl::RendererObserver>);

    // Thread-safe, called by the Frontend
    void updateParameters(std::shared_ptr<mbgl::UpdateParameters>);

signals:
    void needsRendering();

private:
    MBGL_STORE_THREAD(tid)

    Q_DISABLE_COPY(QMapLibreMapRenderer)

    std::mutex m_updateMutex;
    std::shared_ptr<mbgl::UpdateParameters> m_updateParameters;

    QMapLibreRendererBackend m_backend;
    std::unique_ptr<mbgl::Renderer> m_renderer{};

    bool m_forceScheduler{};
};
