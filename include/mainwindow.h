#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>
#include <QVBoxLayout>
#include <QList>
#include "configmanager.h"
#include <QTimer>
#include <temperaturegause.h>
class QPushButton;
class QScrollArea;
class QWidget;
class QLabel;
class QTextEdit;

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
    void onCellClicked(int col, int cell, const QList<int>& subCellPath);
        void updateTemperatureGauges();
    void refreshData();           // слот для обновления данных каждую секунду
void updateCellWidget(QWidget* cellWidget, const CellInfo& cellInfo); // рекурсивное обновление ячеек
void updateCellWidgets();     // обновление всех ячеек из конфига


private:
    void setupUI();
    void clearLayout(QLayout* layout);
    void setupMenu();
    QWidget* createCellWidget(const CellInfo& cellInfo, int colIndex, int cellIndex, const QList<int>& parentPath = QList<int>());
    QWidget* createSubCellWidget(const CellInfo& cellInfo, int colIndex, int subCellIndex, const QList<int>& parentPath);
    void showCellInfo(const QString& pathDescription, const QString& cellName, const CellInfo& cellInfo); // ОБНОВЛЕННАЯ СИГНАТУРА    // УДАЛИТЕ centralWidget из списка полей
    QPushButton *configButton;
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QHBoxLayout *mainLayout;
    ConfigManager *configManager;
        QVector<TemperatureGauge*> temperatureGauges;
    QTimer *updateTimer;
    // Окно для отображения информации о ячейке
    QTextEdit *cellInfoDisplay;
};

#endif // MAINWINDOW_H
