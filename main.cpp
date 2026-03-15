#include <QApplication>
#include <spdlog/spdlog.h>
#include "widgets/mainwindow.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    spdlog::set_level(spdlog::level::debug);

    MainWindow window;
    window.resize(800, 600);
    window.show();
    return app.exec();
}