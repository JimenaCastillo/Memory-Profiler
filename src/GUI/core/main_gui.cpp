#include <QApplication>
#include <QTimer>
#include "MainWindow.hpp"
#include "GUI.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    if (!mp::gui::startGUI(7777)) {
        return 1;
    }

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        mp::gui::tickGUI();
    });
    timer.start(500); // cada 500 ms

    int ret = app.exec();

    mp::gui::stopGUI();
    return ret;
}

//unificacion de codigo