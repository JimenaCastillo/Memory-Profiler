#include <QApplication>
#include <QTimer>
#include "MainWindow.hpp"
#include "GUI.hpp"
#include "Callbacks.hpp"  // Asegúrate de incluir esto

int main(int argc, char *argv[]) {
    // Registrar callbacks de profiling antes de cualquier asignación
    mp::register_callbacks({
        .onAlloc = [](void* ptr, size_t size, const char* tag) {
            // Implementar lógica de profiling
        },
        .onFree = [](void* ptr) {
            // Implementar lógica de liberación
        }
    });

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