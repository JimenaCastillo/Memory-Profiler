#include <QApplication>
#include "../include/MainWindow.hpp"
#include "../Library/include/CallbacksRegistration.hpp"

int main(int argc, char *argv[]) {
    mp::install_callbacks_with_memorytracker();

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}