#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>
#include <QVBoxLayout>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QTimer>
#include <QTextEdit>
#include "configmanager.h"
#include <temperaturegause.h>

class QPushButton;
class QScrollArea;
class QWidget;
class QLabel;

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
    void refreshData();  // обновление данных каждую секунду
    void updateCellWidget(QWidget* cellWidget, const CellInfo& cellInfo); // рекурсивное обновление ячеек
    void updateCellWidgets(); // обновление всех ячеек из конфига

private:
    void setupUI();
    void clearLayout(QLayout* layout);
    void setupMenu();
    QWidget* createCellWidget(const CellInfo& cellInfo, int colIndex, int cellIndex, const QList<int>& parentPath = QList<int>());
    QWidget* createSubCellWidget(const CellInfo& cellInfo, int colIndex, int subCellIndex, const QList<int>& parentPath);
    void showCellInfo(const QString& pathDescription, const QString& cellName, const CellInfo& cellInfo);
    void updateRightPanel();  // ✅ добавляем объявление метода

    // === UI Элементы ===
    QPushButton *configButton;
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QHBoxLayout *mainLayout;
    QVBoxLayout *rightPanel;  // ✅ добавляем панель справа
    QTextEdit *cellInfoDisplay;

    // === Служебные ===
    ConfigManager *configManager;
    QVector<TemperatureGauge*> temperatureGauges;
    QTimer *updateTimer;

    // === Хранилище истории ===
    QMap<QString, QStringList> savedValues;  // ✅ для накопления всех значений
};

#endif // MAINWINDOW_H

