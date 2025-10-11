#include <QApplication>
#include "mainwindow.h"
#include <temperaturegause.h>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.setWindowTitle("Horoshiy User Interface(HUI)");
    window.resize(1000, 600);
    window.show();
    
    return app.exec();
}
