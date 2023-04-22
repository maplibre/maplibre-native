#ifndef QMAPLIBREGL_MAP_H
#define QMAPLIBREGL_MAP_H

#include <QMapLibreGL/Settings>
#include <QMapLibreGL/Types>

#include <QImage>
#include <QMargins>
#include <QObject>
#include <QPointF>
#include <QSize>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>

namespace QMapLibreGL {

class MapPrivate;

// This header follows the Qt coding style: https://wiki.qt.io/Qt_Coding_Style

class Q_MAPLIBREGL_EXPORT Map : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude)
    Q_PROPERTY(double zoom READ zoom WRITE setZoom)
    Q_PROPERTY(double bearing READ bearing WRITE setBearing)
    Q_PROPERTY(double pitch READ pitch WRITE setPitch)
    Q_PROPERTY(QString styleJson READ styleJson WRITE setStyleJson)
    Q_PROPERTY(QString styleUrl READ styleUrl WRITE setStyleUrl)
    Q_PROPERTY(double scale READ scale WRITE setScale)
    Q_PROPERTY(QMapLibreGL::Coordinate coordinate READ coordinate WRITE setCoordinate)
    Q_PROPERTY(QMargins margins READ margins WRITE setMargins)

public:
    enum MapChange {
        MapChangeRegionWillChange = 0,
        MapChangeRegionWillChangeAnimated,
        MapChangeRegionIsChanging,
        MapChangeRegionDidChange,
        MapChangeRegionDidChangeAnimated,
        MapChangeWillStartLoadingMap,
        MapChangeDidFinishLoadingMap,
        MapChangeDidFailLoadingMap,
        MapChangeWillStartRenderingFrame,
        MapChangeDidFinishRenderingFrame,
        MapChangeDidFinishRenderingFrameFullyRendered,
        MapChangeWillStartRenderingMap,
        MapChangeDidFinishRenderingMap,
        MapChangeDidFinishRenderingMapFullyRendered,
        MapChangeDidFinishLoadingStyle,
        MapChangeSourceDidChange
    };

    enum MapLoadingFailure {
        StyleParseFailure,
        StyleLoadFailure,
        NotFoundFailure,
        UnknownFailure
    };

    // Determines the orientation of the map.
    enum NorthOrientation {
        NorthUpwards, // Default
        NorthRightwards,
        NorthDownwards,
        NorthLeftwards,
    };

    Map(QObject* parent = 0,
        const Settings& = Settings(),
        const QSize& size = QSize(),
        qreal pixelRatio = 1);
    virtual ~Map();

    QString styleJson() const;
    QString styleUrl() const;

    void setStyleJson(const QString &);
    void setStyleUrl(const QString &);

    double latitude() const;
    void setLatitude(double latitude);

    double longitude() const;
    void setLongitude(double longitude);

    double scale() const;
    void setScale(double scale, const QPointF &center = QPointF());

    double zoom() const;
    void setZoom(double zoom);

    double minimumZoom() const;
    double maximumZoom() const;

    double bearing() const;
    void setBearing(double degrees);
    void setBearing(double degrees, const QPointF &center);

    double pitch() const;
    void setPitch(double pitch);
    void pitchBy(double pitch);

    NorthOrientation northOrientation() const;
    void setNorthOrientation(NorthOrientation);

    Coordinate coordinate() const;
    void setCoordinate(const Coordinate &);
    void setCoordinateZoom(const Coordinate &, double zoom);

    void jumpTo(const CameraOptions&);

    void setGestureInProgress(bool inProgress);

    void setTransitionOptions(qint64 duration, qint64 delay = 0);

    void addAnnotationIcon(const QString &name, const QImage &sprite);

    AnnotationID addAnnotation(const Annotation &);
    void updateAnnotation(AnnotationID, const Annotation &);
    void removeAnnotation(AnnotationID);

    bool setLayoutProperty(const QString &layer, const QString &property, const QVariant &value);
    bool setPaintProperty(const QString &layer, const QString &property, const QVariant &value);

    bool isFullyLoaded() const;

    void moveBy(const QPointF &offset);
    void scaleBy(double scale, const QPointF &center = QPointF());
    void rotateBy(const QPointF &first, const QPointF &second);

    void resize(const QSize &size);

    QPointF pixelForCoordinate(const Coordinate &) const;
    Coordinate coordinateForPixel(const QPointF &) const;

    CoordinateZoom coordinateZoomForBounds(const Coordinate &sw, const Coordinate &ne) const;
    CoordinateZoom coordinateZoomForBounds(const Coordinate &sw,
                                           const Coordinate &ne,
                                           double bearing,
                                           double pitch);

    void setMargins(const QMargins &margins);
    QMargins margins() const;

    void addSource(const QString &sourceID, const QVariantMap& params);
    bool sourceExists(const QString &sourceID);
    void updateSource(const QString &sourceID, const QVariantMap& params);
    void removeSource(const QString &sourceID);

    void addImage(const QString &name, const QImage &sprite);
    void removeImage(const QString &name);

    void addCustomLayer(const QString &id,
        std::unique_ptr<CustomLayerHostInterface> host,
        const QString &before = QString());
    void addLayer(const QVariantMap &params, const QString& before = QString());
    bool layerExists(const QString &id);
    void removeLayer(const QString &id);

    QVector<QString> layerIds() const;

    void setFilter(const QString &layer, const QVariant &filter);
    QVariant getFilter(const QString &layer) const;
    // When rendering on a different thread,
    // should be called on the render thread.
    void createRenderer();
    void destroyRenderer();
    void setFramebufferObject(quint32 fbo, const QSize &size);

    const QVector<QPair<QString, QString>> &defaultStyles() const;

public slots:
    void render();
    void connectionEstablished();

    // Commit changes, load all the resources
    // and renders the map when completed.
    void startStaticRender();

signals:
    void needsRendering();
    void mapChanged(Map::MapChange);
    void mapLoadingFailed(Map::MapLoadingFailure, const QString &reason);
    void copyrightsChanged(const QString &copyrightsHtml);

    void staticRenderFinished(const QString &error);

private:
    Q_DISABLE_COPY(Map)

    MapPrivate *d_ptr;
};

} // namespace QMapLibreGL

Q_DECLARE_METATYPE(QMapLibreGL::Map::MapChange);
Q_DECLARE_METATYPE(QMapLibreGL::Map::MapLoadingFailure);

#endif // QMAPLIBREGL_MAP_H
