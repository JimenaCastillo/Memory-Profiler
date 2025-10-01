#pragma once
#include "Charts.hpp"
#include <QMainWindow>
#include <QTabWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT // Necesario para señales y slots de Qt
public:
    explicit MainWindow(QWidget *parent = nullptr); // Constructor
private:
    QTabWidget *tabs; // Contenedor de pestañas
    mp::gui::Charts* chartsTab;
};

//unificacion de codigo