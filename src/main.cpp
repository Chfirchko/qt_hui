#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.setWindowTitle("Динамическая таблица Qt6");
    window.resize(1000, 600);
    window.show();
    
    return app.exec();
}
