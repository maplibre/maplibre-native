#pragma once

#include "qmaplibregl.hpp"

#include <mbgl/map/map_observer.hpp>
#include <mbgl/style/style.hpp>

#include <QObject>

#include <exception>
#include <memory>

class QMapLibreGLPrivate;

class QMapLibreMapObserver : public QObject, public mbgl::MapObserver
{
    Q_OBJECT

public:
    explicit QMapLibreMapObserver(QMapLibreGLPrivate *);
    virtual ~QMapLibreMapObserver();

    // mbgl::MapObserver implementation.
    void onCameraWillChange(mbgl::MapObserver::CameraChangeMode) final;
    void onCameraIsChanging() final;
    void onCameraDidChange(mbgl::MapObserver::CameraChangeMode) final;
    void onWillStartLoadingMap() final;
    void onDidFinishLoadingMap() final;
    void onDidFailLoadingMap(mbgl::MapLoadError, const std::string&) final;
    void onWillStartRenderingFrame() final;
    void onDidFinishRenderingFrame(mbgl::MapObserver::RenderFrameStatus) final;
    void onWillStartRenderingMap() final;
    void onDidFinishRenderingMap(mbgl::MapObserver::RenderMode) final;
    void onDidFinishLoadingStyle() final;
    void onSourceChanged(mbgl::style::Source&) final;

signals:
    void mapChanged(QMapLibreGL::MapChange);
    void mapLoadingFailed(QMapLibreGL::MapLoadingFailure, const QString &reason);
    void copyrightsChanged(const QString &copyrightsHtml);

private:
    Q_DISABLE_COPY(QMapLibreMapObserver)

    QMapLibreGLPrivate *d_ptr;
};
