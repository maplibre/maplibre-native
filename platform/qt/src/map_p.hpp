#pragma once

#include <QMapLibreGL/Map>

#include "utils/map_observer.hpp"
#include "utils/map_renderer.hpp"

#include <mbgl/actor/actor.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/renderer/renderer_frontend.hpp>
#include <mbgl/storage/resource_transform.hpp>
#include <mbgl/util/geo.hpp>

#include <QObject>
#include <QSize>

#include <atomic>
#include <memory>

namespace QMapLibreGL {

class MapPrivate : public QObject, public mbgl::RendererFrontend
{
    Q_OBJECT

public:
    explicit MapPrivate(Map *, const Settings &, const QSize &size, qreal pixelRatio);
    virtual ~MapPrivate();

    // mbgl::RendererFrontend implementation.
    void reset() final {}
    void setObserver(mbgl::RendererObserver &) final;
    void update(std::shared_ptr<mbgl::UpdateParameters>) final;

    // These need to be called on the same thread.
    void createRenderer();
    void destroyRenderer();
    void render();
    void setFramebufferObject(quint32 fbo, const QSize& size);

    using PropertySetter = mbgl::optional<mbgl::style::conversion::Error> (mbgl::style::Layer::*)(const std::string&, const mbgl::style::conversion::Convertible&);
    bool setProperty(const PropertySetter& setter, const QString& layer, const QString& name, const QVariant& value);

    mbgl::EdgeInsets margins;
    std::unique_ptr<mbgl::Map> mapObj{};
    QVector<QPair<QString, QString>> defaultStyles;

public slots:
    void requestRendering();

signals:
    void needsRendering();

private:
    Q_DISABLE_COPY(MapPrivate)

    std::recursive_mutex m_mapRendererMutex;
    std::shared_ptr<mbgl::RendererObserver> m_rendererObserver{};
    std::shared_ptr<mbgl::UpdateParameters> m_updateParameters{};

    std::unique_ptr<MapObserver> m_mapObserver{};
    std::unique_ptr<MapRenderer> m_mapRenderer{};
    std::unique_ptr<mbgl::Actor<mbgl::ResourceTransform::TransformCallback>> m_resourceTransform{};

    Settings::GLContextMode m_mode;
    qreal m_pixelRatio;

    QString m_localFontFamily;

    std::atomic_flag m_renderQueued = ATOMIC_FLAG_INIT;
};

} // namespace QMapLibreGL
