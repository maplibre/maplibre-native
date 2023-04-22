#include "mapwindow.hpp"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

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
