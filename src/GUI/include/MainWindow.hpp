#pragma once
#include "Charts.hpp"
#include "SocketServer.hpp"
#include <QMainWindow>
#include <QTabWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT // Necesario para señales y slots de Qt
public:
    explicit MainWindow(mp::gui::SocketServer* server, QWidget *parent = nullptr); // Constructor
private:
    QTabWidget *tabs; // Contenedor de pestañas
    mp::gui::Charts* chartsTab;
    mp::gui::SocketServer* server_;
};

//unificacion de codigo