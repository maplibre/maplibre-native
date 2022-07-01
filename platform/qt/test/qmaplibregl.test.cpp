#include "qmaplibregl.test.hpp"

#include <QMapLibreGL/Map>

#include <QFile>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTextStream>

#include <mbgl/test/util.hpp>

QMapLibreGLTest::QMapLibreGLTest() : size(512, 512), fbo((assert(widget.context()->isValid()), widget.makeCurrent(), size)), map(nullptr, settings, size) {
    connect(&map, &QMapLibreGL::mapChanged, this, &QMapLibreGLTest::onMapChanged);
    connect(&map, &QMapLibreGL::needsRendering, this, &QMapLibreGLTest::onNeedsRendering);
    map.resize(fbo.size());
    map.setFramebufferObject(fbo.handle(), fbo.size());
    map.setCoordinateZoom(QMapLibreGL::Coordinate(60.170448, 24.942046), 14);
}

void QMapLibreGLTest::runUntil(QMapLibreGL::MapChange status) {
    changeCallback = [&](QMapLibreGL::MapChange change) {
        if (change == status) {
            qApp->exit();
            changeCallback = nullptr;
        }
    };

    qApp->exec();
}

void QMapLibreGLTest::onMapChanged(QMapLibreGL::MapChange change) {
    if (changeCallback) {
        changeCallback(change);
    }
}

void QMapLibreGLTest::onNeedsRendering() {
    widget.makeCurrent();
    fbo.bind();
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, fbo.width(), fbo.height());
    map.render();
}


TEST_F(QMapLibreGLTest, TEST_DISABLED_ON_CI(styleJson)) {
    QFile f("test/fixtures/resources/style_vector.json");

    ASSERT_TRUE(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString json = in.readAll();

    map.setStyleJson(json);
    ASSERT_EQ(map.styleJson(), json);
    runUntil(QMapLibreGL::MapChangeDidFinishLoadingMap);

    map.setStyleJson("invalid json");
    runUntil(QMapLibreGL::MapChangeDidFailLoadingMap);

    map.setStyleJson("\"\"");
    runUntil(QMapLibreGL::MapChangeDidFailLoadingMap);

    map.setStyleJson(QString());
    runUntil(QMapLibreGL::MapChangeDidFailLoadingMap);
}

TEST_F(QMapLibreGLTest, TEST_DISABLED_ON_CI(styleUrl)) {
    QString url(QMapLibreGL::defaultStyles()[0].first);

    map.setStyleUrl(url);
    ASSERT_EQ(map.styleUrl(), url);
    runUntil(QMapLibreGL::MapChangeDidFinishLoadingMap);

    map.setStyleUrl("invalid://url");
    runUntil(QMapLibreGL::MapChangeDidFailLoadingMap);

    map.setStyleUrl(QString());
    runUntil(QMapLibreGL::MapChangeDidFailLoadingMap);
}
