#include "MainWindow.hpp"
#include "Tabs.hpp"

// Constructor de MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) // Llama al constructor de QMainWindow con el padre
{
    // Crea el widget de pestañas y lo asocia a esta ventana como padre
    tabs = new QTabWidget(this);

    tabs->addTab(Tabs::createOverviewTab(), "Vista General");
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