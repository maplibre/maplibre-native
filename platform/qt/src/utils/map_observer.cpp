#include "map_observer.hpp"

#include "../qmaplibregl_p.hpp"

#include <mbgl/util/exception.hpp>

#include <exception>

QMapLibreMapObserver::QMapLibreMapObserver(QMapLibreGLPrivate *d)
    : d_ptr(d)
{
}

QMapLibreMapObserver::~QMapLibreMapObserver()
{
}

void QMapLibreMapObserver::onCameraWillChange(mbgl::MapObserver::CameraChangeMode mode)
{
    if (mode == mbgl::MapObserver::CameraChangeMode::Immediate) {
        emit mapChanged(QMapLibreGL::MapChangeRegionWillChange);
    } else {
        emit mapChanged(QMapLibreGL::MapChangeRegionWillChangeAnimated);
    }
}

void QMapLibreMapObserver::onCameraIsChanging()
{
    emit mapChanged(QMapLibreGL::MapChangeRegionIsChanging);
}

void QMapLibreMapObserver::onCameraDidChange(mbgl::MapObserver::CameraChangeMode mode)
{
    if (mode == mbgl::MapObserver::CameraChangeMode::Immediate) {
        emit mapChanged(QMapLibreGL::MapChangeRegionDidChange);
    } else {
        emit mapChanged(QMapLibreGL::MapChangeRegionDidChangeAnimated);
    }
}

void QMapLibreMapObserver::onWillStartLoadingMap()
{
    emit mapChanged(QMapLibreGL::MapChangeWillStartLoadingMap);
}

void QMapLibreMapObserver::onDidFinishLoadingMap()
{
    emit mapChanged(QMapLibreGL::MapChangeDidFinishLoadingMap);
}

void QMapLibreMapObserver::onDidFailLoadingMap(mbgl::MapLoadError error, const std::string& what)
{
    emit mapChanged(QMapLibreGL::MapChangeDidFailLoadingMap);

    QMapLibreGL::MapLoadingFailure type;
    QString description(what.c_str());

    switch (error) {
        case mbgl::MapLoadError::StyleParseError:
            type = QMapLibreGL::MapLoadingFailure::StyleParseFailure;
            break;
        case mbgl::MapLoadError::StyleLoadError:
            type = QMapLibreGL::MapLoadingFailure::StyleLoadFailure;
            break;
        case mbgl::MapLoadError::NotFoundError:
            type = QMapLibreGL::MapLoadingFailure::NotFoundFailure;
            break;
        default:
            type = QMapLibreGL::MapLoadingFailure::UnknownFailure;
    }

    emit mapLoadingFailed(type, description);
}

void QMapLibreMapObserver::onWillStartRenderingFrame()
{
    emit mapChanged(QMapLibreGL::MapChangeWillStartRenderingFrame);
}

void QMapLibreMapObserver::onDidFinishRenderingFrame(mbgl::MapObserver::RenderFrameStatus status)
{
    if (status.mode == mbgl::MapObserver::RenderMode::Partial) {
        emit mapChanged(QMapLibreGL::MapChangeDidFinishRenderingFrame);
    } else {
        emit mapChanged(QMapLibreGL::MapChangeDidFinishRenderingFrameFullyRendered);
    }
}

void QMapLibreMapObserver::onWillStartRenderingMap()
{
    emit mapChanged(QMapLibreGL::MapChangeWillStartRenderingMap);
}

void QMapLibreMapObserver::onDidFinishRenderingMap(mbgl::MapObserver::RenderMode mode)
{
    if (mode == mbgl::MapObserver::RenderMode::Partial) {
        emit mapChanged(QMapLibreGL::MapChangeDidFinishRenderingMap);
    } else {
        emit mapChanged(QMapLibreGL::MapChangeDidFinishRenderingMapFullyRendered);
    }
}

void QMapLibreMapObserver::onDidFinishLoadingStyle()
{
    emit mapChanged(QMapLibreGL::MapChangeDidFinishLoadingStyle);
}

void QMapLibreMapObserver::onSourceChanged(mbgl::style::Source&)
{
    std::string attribution;
    for (const auto& source : d_ptr->mapObj->getStyle().getSources()) {
        // Avoid duplicates by using the most complete attribution HTML snippet.
        if (source->getAttribution() && (attribution.size() < source->getAttribution()->size()))
            attribution = *source->getAttribution();
    }
    emit copyrightsChanged(QString::fromStdString(attribution));
    emit mapChanged(QMapLibreGL::MapChangeSourceDidChange);
}
