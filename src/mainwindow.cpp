#include "mainwindow.h"
#include "tableconfigdialog.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , configButton(nullptr)
    , scrollArea(nullptr)
    , contentWidget(nullptr)
    , mainLayout(nullptr)
    , configManager(new ConfigManager(this))
{
    setupUI();
    setupMenu();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *layout = new QVBoxLayout(centralWidget);

    // Кнопка для настройки
    configButton = new QPushButton("Настроить разделение", this);
    layout->addWidget(configButton);
    connect(configButton, &QPushButton::clicked,
            this, &MainWindow::showConfigDialog);

    // Область с прокруткой для контента
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    
    contentWidget = new QWidget;
    mainLayout = new QHBoxLayout(contentWidget);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    scrollArea->setWidget(contentWidget);
    
    layout->addWidget(scrollArea);

    // Создаем layout из конфига по умолчанию
    createLayoutFromConfig();
}

void MainWindow::setupMenu()
{
    QMenu *fileMenu = menuBar()->addMenu("Файл");

    QAction *loadAction = new QAction("Загрузить конфиг", this);
    QAction *saveAction = new QAction("Сохранить конфиг", this);

    fileMenu->addAction(loadAction);
    fileMenu->addAction(saveAction);

    connect(loadAction, &QAction::triggered, this, &MainWindow::loadConfig);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveConfig);
}

void MainWindow::showConfigDialog()
{
    TableConfigDialog dialog(this);
    dialog.setConfigData(configManager->getColumns());
    
    if (dialog.exec() == QDialog::Accepted) {
        configManager->setColumns(dialog.getColumnsConfig());
        createLayoutFromConfig();
    }
}

void MainWindow::loadConfig()
{
    QString filename = QFileDialog::getOpenFileName(this, "Загрузить конфигурацию", "", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        if (configManager->loadConfig(filename)) {
            createLayoutFromConfig();
            QMessageBox::information(this, "Успех", "Конфигурация загружена успешно!");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить конфигурацию!");
        }
    }
}

void MainWindow::saveConfig()
{
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить конфигурацию", "", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        if (configManager->saveConfig(filename)) {
            QMessageBox::information(this, "Успех", "Конфигурация сохранена успешно!");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить конфигурацию!");
        }
    }
}

void MainWindow::clearLayout(QLayout* layout)
{
    if (!layout) return;
    
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
        }
        delete item;
    }
}

void MainWindow::createLayoutFromConfig()
{
    clearLayout(mainLayout);

    QList<ColumnConfig> columns = configManager->getColumns();

    for (int col = 0; col < columns.size(); ++col) {
        const ColumnConfig& columnConfig = columns[col];
        
        // Создаем фрейм для колонки
        QFrame *columnFrame = new QFrame;
        columnFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
        columnFrame->setLineWidth(2);
        
        // Вертикальный layout для колонки
        QVBoxLayout *columnLayout = new QVBoxLayout(columnFrame);
        columnLayout->setSpacing(2);
        columnLayout->setContentsMargins(2, 2, 2, 2);
        
        // Заголовок колонки
        QLabel *titleLabel = new QLabel(columnConfig.name);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("QLabel { background-color: #e0e0e0; padding: 8px; font-weight: bold; border: 1px solid #a0a0a0; }");
        titleLabel->setMinimumHeight(30);
        columnLayout->addWidget(titleLabel);
        
        // Создаем ячейки в колонке
        for (int cell = 0; cell < columnConfig.cellCount && cell < columnConfig.cells.size(); ++cell) {
            QFrame *cellFrame = new QFrame;
            cellFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            cellFrame->setLineWidth(1);
            cellFrame->setMinimumHeight(80);
            cellFrame->setStyleSheet("QFrame { background-color: #f8f8f8; }");
            
            QVBoxLayout *cellLayout = new QVBoxLayout(cellFrame);
            cellLayout->setContentsMargins(5, 5, 5, 5);
            
            QLabel *cellLabel = new QLabel(columnConfig.cells[cell].content);
            cellLabel->setAlignment(Qt::AlignCenter);
            cellLabel->setWordWrap(true);
            
            cellLayout->addWidget(cellLabel);
            columnLayout->addWidget(cellFrame);
        }
        
        // Добавляем растягивающее пространство в конец колонки
        columnLayout->addStretch();
        
        // Добавляем колонку в основной layout с растягиванием
        mainLayout->addWidget(columnFrame, 1);
    }
}
