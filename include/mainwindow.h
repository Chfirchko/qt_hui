#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>
#include <QVBoxLayout>
#include <QList>
#include "configmanager.h"

class QPushButton;
class QScrollArea;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showConfigDialog();
    void createLayoutFromConfig();
    void loadConfig();
    void saveConfig();

private:
    void setupUI();
    void clearLayout(QLayout* layout);
    void setupMenu();

    QWidget *centralWidget;
    QPushButton *configButton;
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QHBoxLayout *mainLayout;
    ConfigManager *configManager;
};

#endif // MAINWINDOW_H
