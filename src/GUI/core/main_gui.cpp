#include <QApplication>
#include <QTimer>
#include "MainWindow.hpp"
#include "Callbacks.hpp"
#include "SocketServer.hpp"

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

    auto* server = new mp::gui::SocketServer;
    if (!server->start(7777)) {
        return 1;
    }

    MainWindow* w = new MainWindow(server);
    w->show();

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [server]() {
        // Si necesitas hacer algo periódico con el servidor, hazlo aquí
    });
    timer.start(500); // cada 500 ms

    int ret = app.exec();

    server->stop();
    delete server;
    return ret;
}

//unificacion de codigo