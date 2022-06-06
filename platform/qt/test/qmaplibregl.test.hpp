#include <QMapLibreGL>

#include <QGLWidget>
#include <QGLFramebufferObject>

#include <gtest/gtest.h>

class QMapLibreGLTest : public QObject, public ::testing::Test {
    Q_OBJECT

public:
    QMapLibreGLTest();

    void runUntil(QMapLibreGL::MapChange);

private:
    QGLWidget widget;
    const QSize size;
    QGLFramebufferObject fbo;

protected:
    QMapLibreSettings settings;
    QMapLibreGL map;

    std::function<void(QMapLibreGL::MapChange)> changeCallback;

private slots:
    void onMapChanged(QMapLibreGL::MapChange);
    void onNeedsRendering();
};
