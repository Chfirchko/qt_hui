#include "mainwindow.h"
#include "tableconfigdialog.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , tableWidget(nullptr)
    , configButton(nullptr)
    , centralWidget(nullptr)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *layout = new QVBoxLayout(centralWidget);

    // Кнопка для настройки таблицы
    configButton = new QPushButton("Настроить таблицу", this);
    layout->addWidget(configButton);
    connect(configButton, &QPushButton::clicked,
            this, &MainWindow::showConfigDialog);

    // Область для таблицы
    tableWidget = new QTableWidget(this);
    layout->addWidget(tableWidget);

    // Создаем таблицу с параметрами по умолчанию
    createTable(3, {"Столбец 1", "Столбец 2", "Столбец 3"}, 5);
}

void MainWindow::showConfigDialog()
{
    TableConfigDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        createTable(dialog.getColumnCount(),
                   dialog.getColumnNames(),
                   dialog.getRowCount());
    }
}

void MainWindow::createTable(int columnCount, const QStringList &columnNames, int rowCount)
{
    tableWidget->clear();

    // Устанавливаем количество столбцов и строк
    tableWidget->setColumnCount(columnCount);
    tableWidget->setRowCount(rowCount);

    // Устанавливаем названия столбцов
    tableWidget->setHorizontalHeaderLabels(columnNames);

    // Заполняем таблицу тестовыми данными
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < columnCount; ++col) {
            auto *item = new QTableWidgetItem(
                QString("Ячейка %1-%2").arg(row + 1).arg(col + 1)
            );
            tableWidget->setItem(row, col, item);
        }
    }

    // Настраиваем внешний вид таблицы
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
}
