#include "MainWindow.hpp"
#include "Tabs.hpp"
#include "Charts.hpp"
#include <QTabWidget>

using namespace mp::gui;

// Constructor de MainWindow
MainWindow::MainWindow(mp::gui::SocketServer* server, QWidget *parent)
    : QMainWindow(parent), server_(server) // Llama al constructor de QMainWindow con el padre
{
    // Crea el widget de pestañas y lo asocia a esta ventana como padre
    tabs = new QTabWidget(this);

    chartsTab = new Charts;
    tabs->addTab(chartsTab, "Vista General");
    QObject::connect(server_, &SocketServer::metricsUpdated,
                     chartsTab, &Charts::updateMemoryUsage);

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