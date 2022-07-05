#include "mapwindow.hpp"

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QString>

int kAnimationDuration = 10000;

MapWindow::MapWindow(const QMapLibreGL::Settings &settings)
    : m_settings(settings)
{
    setWindowIcon(QIcon(":icon.png"));
}

MapWindow::~MapWindow()
{
    // Make sure we have a valid context so we
    // can delete the QMapLibreGL::Map.
    makeCurrent();
}

void MapWindow::selfTest()
{
    if (m_bearingAnimation) {
        m_bearingAnimation->setDuration(kAnimationDuration);
        m_bearingAnimation->setEndValue(m_map->bearing() + 360 * 4);
        m_bearingAnimation->start();
    }

    if (m_zoomAnimation) {
        m_zoomAnimation->setDuration(kAnimationDuration);
        m_zoomAnimation->setEndValue(m_map->zoom() + 3);
        m_zoomAnimation->start();
    }
}

qreal MapWindow::pixelRatio() {
    return devicePixelRatioF();
}


void MapWindow::animationFinished()
{
    qDebug() << "Animation ticks/s: " <<  m_animationTicks / static_cast<float>(kAnimationDuration) * 1000.;
    qDebug() << "Frame draws/s: " <<  m_frameDraws / static_cast<float>(kAnimationDuration) * 1000.;

    qApp->quit();
}

void MapWindow::animationValueChanged()
{
    m_animationTicks++;
}

void MapWindow::changeStyle()
{
    static uint8_t currentStyleIndex;

    auto& styles = m_map->defaultStyles();

    m_map->setStyleUrl(styles[currentStyleIndex].first);
    setWindowTitle(QString("MapLibre GL: ") + styles[currentStyleIndex].second);

    if (++currentStyleIndex == styles.size()) {
        currentStyleIndex = 0;
    }

    m_sourceAdded = false;
}

void MapWindow::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_S:
        changeStyle();
        break;
    case Qt::Key_L: {
            if (m_sourceAdded) {
                return;
            }

            m_sourceAdded = true;

            // Not in all styles, but will work on streets
            QString before = "waterway-label";

            QFile geojson(":source1.geojson");
            geojson.open(QIODevice::ReadOnly);

            // The data source for the route line and markers
            QVariantMap routeSource;
            routeSource["type"] = "geojson";
            routeSource["data"] = geojson.readAll();
            m_map->addSource("routeSource", routeSource);

            // The route case, painted before the route
            QVariantMap routeCase;
            routeCase["id"] = "routeCase";
            routeCase["type"] = "line";
            routeCase["source"] = "routeSource";
            m_map->addLayer(routeCase, before);

            m_map->setPaintProperty("routeCase", "line-color", QColor("white"));
            m_map->setPaintProperty("routeCase", "line-width", 20.0);
            m_map->setLayoutProperty("routeCase", "line-join", "round");
            m_map->setLayoutProperty("routeCase", "line-cap", "round");

            // The route, painted on top of the route case
            QVariantMap route;
            route["id"] = "route";
            route["type"] = "line";
            route["source"] = "routeSource";
            m_map->addLayer(route, before);

            m_map->setPaintProperty("route", "line-color", QColor("blue"));
            m_map->setPaintProperty("route", "line-width", 8.0);
            m_map->setLayoutProperty("route", "line-join", "round");
            m_map->setLayoutProperty("route", "line-cap", "round");

            QVariantList lineDashArray;
            lineDashArray.append(1);
            lineDashArray.append(2);

            m_map->setPaintProperty("route", "line-dasharray", lineDashArray);

            // Markers at the beginning and end of the route
            m_map->addImage("label-arrow", QImage(":label-arrow.svg"));
            m_map->addImage("label-background", QImage(":label-background.svg"));

            QVariantMap markerArrow;
            markerArrow["id"] = "markerArrow";
            markerArrow["type"] = "symbol";
            markerArrow["source"] = "routeSource";
            m_map->addLayer(markerArrow);

            m_map->setLayoutProperty("markerArrow", "icon-image", "label-arrow");
            m_map->setLayoutProperty("markerArrow", "icon-size", 0.5);
            m_map->setLayoutProperty("markerArrow", "icon-ignore-placement", true);

            QVariantList arrowOffset;
            arrowOffset.append(0.0);
            arrowOffset.append(-15.0);
            m_map->setLayoutProperty("markerArrow", "icon-offset", arrowOffset);

            QVariantMap markerBackground;
            markerBackground["id"] = "markerBackground";
            markerBackground["type"] = "symbol";
            markerBackground["source"] = "routeSource";
            m_map->addLayer(markerBackground);

            m_map->setLayoutProperty("markerBackground", "icon-image", "label-background");
            m_map->setLayoutProperty("markerBackground", "text-field", "{name}");
            m_map->setLayoutProperty("markerBackground", "icon-text-fit", "both");
            m_map->setLayoutProperty("markerBackground", "icon-ignore-placement", true);
            m_map->setLayoutProperty("markerBackground", "text-ignore-placement", true);
            m_map->setLayoutProperty("markerBackground", "text-anchor", "left");
            m_map->setLayoutProperty("markerBackground", "text-size", 16.0);
            m_map->setLayoutProperty("markerBackground", "text-padding", 0.0);
            m_map->setLayoutProperty("markerBackground", "text-line-height", 1.0);
            m_map->setLayoutProperty("markerBackground", "text-max-width", 8.0);

            QVariantList iconTextFitPadding;
            iconTextFitPadding.append(15.0);
            iconTextFitPadding.append(10.0);
            iconTextFitPadding.append(15.0);
            iconTextFitPadding.append(10.0);
            m_map->setLayoutProperty("markerBackground", "icon-text-fit-padding", iconTextFitPadding);

            QVariantList backgroundOffset;
            backgroundOffset.append(-0.5);
            backgroundOffset.append(-1.5);
            m_map->setLayoutProperty("markerBackground", "text-offset", backgroundOffset);

            m_map->setPaintProperty("markerBackground", "text-color", QColor("white"));

            QVariantList filterExpression;
            filterExpression.append("==");
            filterExpression.append("$type");
            filterExpression.append("Point");

            QVariantList filter;
            filter.append(filterExpression);

            m_map->setFilter("markerArrow", filter);
            m_map->setFilter("markerBackground", filter);

            // Tilt the labels when tilting the map and make them larger
            m_map->setLayoutProperty("road-label-large", "text-size", 30.0);
            m_map->setLayoutProperty("road-label-large", "text-pitch-alignment", "viewport");

            m_map->setLayoutProperty("road-label-medium", "text-size", 30.0);
            m_map->setLayoutProperty("road-label-medium", "text-pitch-alignment", "viewport");

            m_map->setLayoutProperty("road-label-small", "text-pitch-alignment", "viewport");
            m_map->setLayoutProperty("road-label-small", "text-size", 30.0);

            // Buildings extrusion
            QVariantMap buildings;
            buildings["id"] = "3d-buildings";
            buildings["source"] = "composite";
            buildings["source-layer"] = "building";
            buildings["type"] = "fill-extrusion";
            buildings["minzoom"] = 15.0;
            m_map->addLayer(buildings);

            QVariantList buildingsFilterExpression;
            buildingsFilterExpression.append("==");
            buildingsFilterExpression.append("extrude");
            buildingsFilterExpression.append("true");

            QVariantList buildingsFilter;
            buildingsFilter.append(buildingsFilterExpression);

            m_map->setFilter("3d-buildings", buildingsFilterExpression);

            QString fillExtrusionColorJSON = R"JSON(
              [
                "interpolate",
                ["linear"],
                ["get", "height"],
                  0.0, "blue",
                 20.0, "royalblue",
                 40.0, "cyan",
                 60.0, "lime",
                 80.0, "yellow",
                100.0, "red"
              ]
            )JSON";

            m_map->setPaintProperty("3d-buildings", "fill-extrusion-color", fillExtrusionColorJSON);
            m_map->setPaintProperty("3d-buildings", "fill-extrusion-opacity", .6);

            QVariantMap extrusionHeight;
            extrusionHeight["type"] = "identity";
            extrusionHeight["property"] = "height";

            m_map->setPaintProperty("3d-buildings", "fill-extrusion-height", extrusionHeight);

            QVariantMap extrusionBase;
            extrusionBase["type"] = "identity";
            extrusionBase["property"] = "min_height";

            m_map->setPaintProperty("3d-buildings", "fill-extrusion-base", extrusionBase);
        }
        break;
    case Qt::Key_1: {
            if (m_symbolAnnotationId.isNull()) {
                QMapLibreGL::Coordinate coordinate = m_map->coordinate();
                QMapLibreGL::SymbolAnnotation symbol { coordinate, "default_marker" };
                m_map->addAnnotationIcon("default_marker", QImage(":default_marker.svg"));
                m_symbolAnnotationId = m_map->addAnnotation(QVariant::fromValue<QMapLibreGL::SymbolAnnotation>(symbol));
            } else {
                m_map->removeAnnotation(m_symbolAnnotationId.toUInt());
                m_symbolAnnotationId.clear();
            }
        }
        break;
    case Qt::Key_2: {
            if (m_lineAnnotationId.isNull()) {
                QMapLibreGL::Coordinates coordinates;
                coordinates.push_back(m_map->coordinateForPixel({ 0, 0 }));
                coordinates.push_back(m_map->coordinateForPixel({ qreal(size().width()), qreal(size().height()) }));

                QMapLibreGL::CoordinatesCollection collection;
                collection.push_back(coordinates);

                QMapLibreGL::CoordinatesCollections lineGeometry;
                lineGeometry.push_back(collection);

                QMapLibreGL::ShapeAnnotationGeometry annotationGeometry(QMapLibreGL::ShapeAnnotationGeometry::LineStringType, lineGeometry);

                QMapLibreGL::LineAnnotation line;
                line.geometry = annotationGeometry;
                line.opacity = 0.5f;
                line.width = 1.0f;
                line.color = Qt::red;
                m_lineAnnotationId = m_map->addAnnotation(QVariant::fromValue<QMapLibreGL::LineAnnotation>(line));
            } else {
                m_map->removeAnnotation(m_lineAnnotationId.toUInt());
                m_lineAnnotationId.clear();
            }
        }
        break;
    case Qt::Key_3: {
            if (m_fillAnnotationId.isNull()) {
                QMapLibreGL::Coordinates coordinates;
                coordinates.push_back(m_map->coordinateForPixel({ qreal(size().width()), 0 }));
                coordinates.push_back(m_map->coordinateForPixel({ qreal(size().width()), qreal(size().height()) }));
                coordinates.push_back(m_map->coordinateForPixel({ 0, qreal(size().height()) }));
                coordinates.push_back(m_map->coordinateForPixel({ 0, 0 }));

                QMapLibreGL::CoordinatesCollection collection;
                collection.push_back(coordinates);

                QMapLibreGL::CoordinatesCollections fillGeometry;
                fillGeometry.push_back(collection);

                QMapLibreGL::ShapeAnnotationGeometry annotationGeometry(QMapLibreGL::ShapeAnnotationGeometry::PolygonType, fillGeometry);

                QMapLibreGL::FillAnnotation fill;
                fill.geometry = annotationGeometry;
                fill.opacity = 0.5f;
                fill.color = Qt::green;
                fill.outlineColor = QVariant::fromValue<QColor>(QColor(Qt::black));
                m_fillAnnotationId = m_map->addAnnotation(QVariant::fromValue<QMapLibreGL::FillAnnotation>(fill));
            } else {
                m_map->removeAnnotation(m_fillAnnotationId.toUInt());
                m_fillAnnotationId.clear();
            }
        }
        break;
    case Qt::Key_5: {
            if (m_map->layerExists("circleLayer")) {
                m_map->removeLayer("circleLayer");
                m_map->removeSource("circleSource");
            } else {
                QMapLibreGL::Coordinates coordinates;
                coordinates.push_back(m_map->coordinate());

                QMapLibreGL::CoordinatesCollection collection;
                collection.push_back(coordinates);

                QMapLibreGL::CoordinatesCollections point;
                point.push_back(collection);

                QMapLibreGL::Feature feature(QMapLibreGL::Feature::PointType, point, {}, {});

                QVariantMap circleSource;
                circleSource["type"] = "geojson";
                circleSource["data"] = QVariant::fromValue<QMapLibreGL::Feature>(feature);
                m_map->addSource("circleSource", circleSource);

                QVariantMap circle;
                circle["id"] = "circleLayer";
                circle["type"] = "circle";
                circle["source"] = "circleSource";
                m_map->addLayer(circle);

                m_map->setPaintProperty("circleLayer", "circle-radius", 10.0);
                m_map->setPaintProperty("circleLayer", "circle-color", QColor("black"));
            }
        }
        break;
    case Qt::Key_6: {
            if (m_map->layerExists("innerCirclesLayer") || m_map->layerExists("outerCirclesLayer")) {
                m_map->removeLayer("innerCirclesLayer");
                m_map->removeLayer("outerCirclesLayer");
                m_map->removeSource("innerCirclesSource");
                m_map->removeSource("outerCirclesSource");
            } else {
                auto makePoint = [&] (double dx, double dy, const QString &color) {
                    auto coordinate = m_map->coordinate();
                    coordinate.first += dx;
                    coordinate.second += dy;
                    return QMapLibreGL::Feature{QMapLibreGL::Feature::PointType,
                        {{{coordinate}}}, {{"color", color}}, {}};
                };

                // multiple features by QVector<QMapLibreGL::Feature>
                QVector<QMapLibreGL::Feature> inner{
                    makePoint(0.001,  0, "red"),
                    makePoint(0,  0.001, "green"),
                    makePoint(0, -0.001, "blue")
                };

                m_map->addSource("innerCirclesSource",
                    {{"type", "geojson"}, {"data", QVariant::fromValue(inner)}});
                m_map->addLayer({
                    {"id", "innerCirclesLayer"},
                    {"type", "circle"},
                    {"source", "innerCirclesSource"}
                });

                // multiple features by QList<QMapLibreGL::Feature>
                QList<QMapLibreGL::Feature> outer{
                    makePoint( 0.002,  0.002, "cyan"),
                    makePoint(-0.002,  0.002, "magenta"),
                    makePoint( 0.002, -0.002, "yellow"),
                    makePoint(-0.002, -0.002, "black")
                };

                m_map->addSource("outerCirclesSource",
                    {{"type", "geojson"}, {"data", QVariant::fromValue(outer)}});
                m_map->addLayer({
                    {"id", "outerCirclesLayer"},
                    {"type", "circle"},
                    {"source", "outerCirclesSource"}
                });

                QVariantList getColor{"get", "color"};
                m_map->setPaintProperty("innerCirclesLayer", "circle-radius", 10.0);
                m_map->setPaintProperty("innerCirclesLayer", "circle-color", getColor);
                m_map->setPaintProperty("outerCirclesLayer", "circle-radius", 15.0);
                m_map->setPaintProperty("outerCirclesLayer", "circle-color", getColor);
            }
        }
        break;
    default:
        break;
    }

    ev->accept();
}

void MapWindow::mousePressEvent(QMouseEvent *ev)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_lastPos = ev->position();
#else
    m_lastPos = ev->localPos();
#endif

    if (ev->type() == QEvent::MouseButtonPress) {
        if (ev->buttons() == (Qt::LeftButton | Qt::RightButton)) {
            changeStyle();
        }
    }

    if (ev->type() == QEvent::MouseButtonDblClick) {
        if (ev->buttons() == Qt::LeftButton) {
            m_map->scaleBy(2.0, m_lastPos);
        } else if (ev->buttons() == Qt::RightButton) {
            m_map->scaleBy(0.5, m_lastPos);
        }
    }

    ev->accept();
}

void MapWindow::mouseMoveEvent(QMouseEvent *ev)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF &position = ev->position();
#else
    const QPointF &position = ev->localPos();
#endif

    QPointF delta = position - m_lastPos;
    if (!delta.isNull()) {
        if (ev->buttons() == Qt::LeftButton && ev->modifiers() & Qt::ShiftModifier) {
            m_map->pitchBy(delta.y());
        } else if (ev->buttons() == Qt::LeftButton) {
            m_map->moveBy(delta);
        } else if (ev->buttons() == Qt::RightButton) {
            m_map->rotateBy(m_lastPos, position);
        }
    }

    m_lastPos = position;
    ev->accept();
}

void MapWindow::wheelEvent(QWheelEvent *ev)
{
    if (ev->angleDelta().y() == 0) {
        return;
    }

    float factor = ev->angleDelta().y() / 1200.;
    if (ev->angleDelta().y() < 0) {
        factor = factor > -1 ? factor : 1 / factor;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    m_map->scaleBy(1 + factor, ev->position());
#else
    m_map->scaleBy(1 + factor, ev->pos());
#endif
    ev->accept();
}

void MapWindow::initializeGL()
{
    m_map.reset(new QMapLibreGL::Map(nullptr, m_settings, size(), pixelRatio()));
    connect(m_map.get(), SIGNAL(needsRendering()), this, SLOT(update()));

    // Set default location to Helsinki.
    m_map->setCoordinateZoom(QMapLibreGL::Coordinate(60.170448, 24.942046), 5);

    QString styleUrl = qgetenv("MGL_STYLE_URL");
    if (styleUrl.isEmpty()) {
        changeStyle();
    } else {
        m_map->setStyleUrl(styleUrl);
        setWindowTitle(QString("MapLibre GL: ") + styleUrl);
    }

    m_bearingAnimation = new QPropertyAnimation(m_map.get(), "bearing");
    m_zoomAnimation = new QPropertyAnimation(m_map.get(), "zoom");

    connect(m_zoomAnimation, &QPropertyAnimation::finished, this, &MapWindow::animationFinished);
    connect(m_zoomAnimation, &QPropertyAnimation::valueChanged, this, &MapWindow::animationValueChanged);
}

void MapWindow::paintGL()
{
    m_frameDraws++;
    m_map->resize(size());
    m_map->setFramebufferObject(defaultFramebufferObject(), size() * pixelRatio());
    m_map->render();
}
