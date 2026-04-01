#include <QApplication>
#include <spdlog/spdlog.h>
#include "widgets/mainwindow.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    spdlog::set_level(
#ifdef LOG_UNTIL_DEBUG
        spdlog::level::debug
#else
        spdlog::level::info
#endif
    );

    MainWindow window;
    window.show();

    QObject::connect(&app, &QApplication::aboutToQuit, [] {
        spdlog::info("Quitting...");
    });

    return app.exec();
}