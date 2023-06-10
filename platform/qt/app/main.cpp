#include "mapwindow.hpp"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setAlphaBufferSize(8);
    fmt.setBlueBufferSize(8);
    fmt.setGreenBufferSize(8);
    fmt.setRedBufferSize(8);
    fmt.setStencilBufferSize(0);
    fmt.setSamples(0);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    fmt.setSwapInterval(1);
    fmt.setDepthBufferSize(16);

    // Set proper OGL version now (needed for "cocoa" or "xcb"), but have troubles with "wayland" devices.
    if (app.platformName() != QString("wayland"))
    {
      fmt.setProfile(QSurfaceFormat::CoreProfile);
      fmt.setVersion(3, 3);
    }

    QSurfaceFormat::setDefaultFormat(fmt);

    QMapLibreGL::Settings settings;
    settings.setCacheDatabasePath("/tmp/mbgl-cache.db");
    settings.setCacheDatabaseMaximumSize(20 * 1024 * 1024);

    settings.setClientName("TestApp");
    settings.setClientVersion("1.0");

    MapWindow window(settings);

    window.resize(800, 600);
    window.show();

    if (argc == 2 && QString("--test") == argv[1]) {
        window.selfTest();
    }

    return app.exec();
}
