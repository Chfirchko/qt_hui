#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showConfigDialog();
    void createTable(int columnCount, const QStringList &columnNames, int rowCount);

private:
    void setupUI();

    QTableWidget *tableWidget;
    QPushButton *configButton;
    QWidget *centralWidget;
};

#endif // MAINWINDOW_H
