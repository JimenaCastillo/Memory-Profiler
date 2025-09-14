#pragma once
#include <QMainWindow>
#include <QTabWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT // Necesario para señales y slots de Qt
public:
    explicit MainWindow(QWidget *parent = nullptr); // Constructor
private:
    QTabWidget *tabs; // Contenedor de pestañas
};

