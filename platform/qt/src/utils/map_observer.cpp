#include "map_observer.hpp"

#include "../map_p.hpp"

#include <mbgl/util/exception.hpp>

#include <exception>

namespace QMapLibreGL {

MapObserver::MapObserver(MapPrivate *d)
    : d_ptr(d)
{
}

MapObserver::~MapObserver()
{
}

void MapObserver::onCameraWillChange(mbgl::MapObserver::CameraChangeMode mode)
{
    if (mode == mbgl::MapObserver::CameraChangeMode::Immediate) {
        emit mapChanged(Map::MapChangeRegionWillChange);
    } else {
        emit mapChanged(Map::MapChangeRegionWillChangeAnimated);
    }
}

void MapObserver::onCameraIsChanging()
{
    emit mapChanged(Map::MapChangeRegionIsChanging);
}

void MapObserver::onCameraDidChange(mbgl::MapObserver::CameraChangeMode mode)
{
    if (mode == mbgl::MapObserver::CameraChangeMode::Immediate) {
        emit mapChanged(Map::MapChangeRegionDidChange);
    } else {
        emit mapChanged(Map::MapChangeRegionDidChangeAnimated);
    }
}

void MapObserver::onWillStartLoadingMap()
{
    emit mapChanged(Map::MapChangeWillStartLoadingMap);
}

void MapObserver::onDidFinishLoadingMap()
{
    emit mapChanged(Map::MapChangeDidFinishLoadingMap);
}

void MapObserver::onDidFailLoadingMap(mbgl::MapLoadError error, const std::string& what)
{
    emit mapChanged(Map::MapChangeDidFailLoadingMap);

    Map::MapLoadingFailure type;
    QString description(what.c_str());

    switch (error) {
        case mbgl::MapLoadError::StyleParseError:
            type = Map::MapLoadingFailure::StyleParseFailure;
            break;
        case mbgl::MapLoadError::StyleLoadError:
            type = Map::MapLoadingFailure::StyleLoadFailure;
            break;
        case mbgl::MapLoadError::NotFoundError:
            type = Map::MapLoadingFailure::NotFoundFailure;
            break;
        default:
            type = Map::MapLoadingFailure::UnknownFailure;
    }

    emit mapLoadingFailed(type, description);
}

void MapObserver::onWillStartRenderingFrame()
{
    emit mapChanged(Map::MapChangeWillStartRenderingFrame);
}

void MapObserver::onDidFinishRenderingFrame(mbgl::MapObserver::RenderFrameStatus status)
{
    if (status.mode == mbgl::MapObserver::RenderMode::Partial) {
        emit mapChanged(Map::MapChangeDidFinishRenderingFrame);
    } else {
        emit mapChanged(Map::MapChangeDidFinishRenderingFrameFullyRendered);
    }
}

void MapObserver::onWillStartRenderingMap()
{
    emit mapChanged(Map::MapChangeWillStartRenderingMap);
}

void MapObserver::onDidFinishRenderingMap(mbgl::MapObserver::RenderMode mode)
{
    if (mode == mbgl::MapObserver::RenderMode::Partial) {
        emit mapChanged(Map::MapChangeDidFinishRenderingMap);
    } else {
        emit mapChanged(Map::MapChangeDidFinishRenderingMapFullyRendered);
    }
}

void MapObserver::onDidFinishLoadingStyle()
{
    emit mapChanged(Map::MapChangeDidFinishLoadingStyle);
}

void MapObserver::onSourceChanged(mbgl::style::Source&)
{
    std::string attribution;
    for (const auto& source : d_ptr->mapObj->getStyle().getSources()) {
        // Avoid duplicates by using the most complete attribution HTML snippet.
        if (source->getAttribution() && (attribution.size() < source->getAttribution()->size()))
            attribution = *source->getAttribution();
    }
    emit copyrightsChanged(QString::fromStdString(attribution));
    emit mapChanged(Map::MapChangeSourceDidChange);
}

} // namespace QMapLibreGL
