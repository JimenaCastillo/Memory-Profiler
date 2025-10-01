#include "Tabs.hpp"
#include "Views.hpp"
#include "Charts.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>

// Variables globales internas a este archivo
// Se usan para compartir widgets con Views.cpp
// No se liberan intencionalmente para que el profiler detecte fugas
namespace {
    QLabel* g_metricsLabel = nullptr;
    QTableWidget* g_allocTable = nullptr;
}

// Pestaña "Vista General"
QWidget* Tabs::createOverviewTab() {
    auto *tab = new QWidget;         // Contenedor principal de la pestaña
    auto *layout = new QVBoxLayout;  // Layout vertical

    // Leak intencional
    g_metricsLabel = new QLabel("Esperando métricas...");
    layout->addWidget(g_metricsLabel);

    // Leak intencional
    g_allocTable = new QTableWidget(1, 2);
    g_allocTable->setHorizontalHeaderLabels({"Métrica", "Valor"});
    layout->addWidget(g_allocTable);

    // Inicializa Views con estos widgets para que puedan actualizarse
    mp::gui::initViews(g_metricsLabel, g_allocTable);

    // Leak intencional
    auto* chartsWidget = new mp::gui::Charts;

    // Añade el gráfico al layout
    layout->addWidget(chartsWidget);

    // Asigna el layout al widget y lo devuelve
    tab->setLayout(layout);
    return tab;
}

// Pestaña "Mapa de Memoria"
QWidget* Tabs::createMemoryMapTab() {
    auto *tab = new QWidget;
    auto *layout = new QVBoxLayout;

    layout->addWidget(new QLabel("Mapa de memoria (bloques asignados)"));

    // Leak intencional
    auto *table = new QTableWidget(5, 3);
    table->setHorizontalHeaderLabels({"Dirección", "Tipo", "Tamaño"});
    layout->addWidget(table);

    tab->setLayout(layout);
    return tab;
}

// Pestaña "Por Archivo"
QWidget* Tabs::createByFileTab() {
    auto *tab = new QWidget;
    auto *layout = new QVBoxLayout;

    layout->addWidget(new QLabel("Asignación por archivo fuente"));

    // Leak intencional
    auto *table = new QTableWidget(5, 2);
    table->setHorizontalHeaderLabels({"Archivo", "Asignaciones"});
    layout->addWidget(table);

    tab->setLayout(layout);
    return tab;
}

// Pestaña "Leaks"
QWidget* Tabs::createLeaksTab() {
    auto *tab = new QWidget;
    auto *layout = new QVBoxLayout;

    layout->addWidget(new QLabel("Resumen de leaks"));

    // Leak intencional
    auto *table = new QTableWidget(3, 2);
    table->setHorizontalHeaderLabels({"Archivo", "Leaks (MB)"});
    layout->addWidget(table);

    tab->setLayout(layout);
    return tab;
}

//unificacion de codigo