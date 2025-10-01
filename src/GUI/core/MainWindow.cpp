#include "MainWindow.hpp"
#include "Tabs.hpp"
#include "Charts.hpp"
#include <QTabWidget>

using namespace mp::gui;

// Constructor de MainWindow
extern mp::gui::SocketServer g_server;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) // Llama al constructor de QMainWindow con el padre
{
    // Crea el widget de pestañas y lo asocia a esta ventana como padre
    tabs = new QTabWidget(this);

    chartsTab = new mp::gui::Charts;
    tabs->addTab(chartsTab, "Vista General");
    QObject::connect(&g_server, &mp::gui::SocketServer::metricsUpdated,
                 chartsTab, &mp::gui::Charts::updateMemoryUsage);

    tabs->addTab(Tabs::createMemoryMapTab(), "Mapa de Memoria");
    tabs->addTab(Tabs::createByFileTab(), "Por Archivo");
    tabs->addTab(Tabs::createLeaksTab(), "Leaks");

    // Establece el widget de pestañas como contenido central de la ventana
    setCentralWidget(tabs);

    // Título de la ventana
    setWindowTitle("Memory Profiler");

    // Tamaño inicial de la ventana
    resize(800, 600);
}

//unificacion de codigo