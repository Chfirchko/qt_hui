#include "mainwindow.h"
#include "tableconfigdialog.h"
#include "temperaturegause.h"
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
#include <QTextEdit>
#include <QMouseEvent>
#include <QDebug>
#include <QRegularExpression>




// Кастомный виджет ячейки с поддержкой кликов
class ClickableFrame : public QFrame
{
    Q_OBJECT
public:
    ClickableFrame(int col, int cell, const QList<int>& subCellPath, QWidget* parent = nullptr)
        : QFrame(parent), m_col(col), m_cell(cell), m_subCellPath(subCellPath) 
    {
        setCursor(Qt::PointingHandCursor);
    }

signals:
    void clicked(int col, int cell, const QList<int>& subCellPath);

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked(m_col, m_cell, m_subCellPath);
        }
        QFrame::mousePressEvent(event);
    }

private:
    int m_col;
    int m_cell;
    QList<int> m_subCellPath;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    // УДАЛИТЕ centralWidget из списка инициализации
    , configButton(nullptr)
    , scrollArea(nullptr)
    , contentWidget(nullptr)
    , mainLayout(nullptr)
    , configManager(new ConfigManager(this))
    , cellInfoDisplay(nullptr)
{
    setupUI();
    setupMenu();
    
    // Автоматически загружаем конфиг при старте, если он существует
    if (configManager->configExists()) {
        qDebug() << "Автоматически загружен конфиг:" << configManager->getConfigPath();
    } else {
        qDebug() << "Используется конфиг по умолчанию";
    }
}

MainWindow::~MainWindow()
{
    // Явная очистка, если нужна
    if (configManager) {
        configManager->deleteLater();
    }
}


void MainWindow::setupUI()
{
    // Создаем центральный виджет
    QWidget *centralWgt = new QWidget(this);
    setCentralWidget(centralWgt);

    auto *mainVLayout = new QVBoxLayout(centralWgt);

    // Верхняя панель с кнопками
    auto *topPanel = new QHBoxLayout;
    
    configButton = new QPushButton("Настроить разделение", this);
    topPanel->addWidget(configButton);
    
    auto *loadButton = new QPushButton("Загрузить конфиг", this);
    topPanel->addWidget(loadButton);
    
    auto *saveButton = new QPushButton("Сохранить конфиг", this);
    topPanel->addWidget(saveButton);
    
    topPanel->addStretch();
    
    mainVLayout->addLayout(topPanel);

    // Основная область с разделением
    auto *contentSplitter = new QHBoxLayout;
    
    // Левая часть - скроллируемая область с колонками
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    
    contentWidget = new QWidget;
    mainLayout = new QHBoxLayout(contentWidget);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    scrollArea->setWidget(contentWidget);
    
    contentSplitter->addWidget(scrollArea, 3); // 3/4 ширины
    
    // Правая часть - информация о выбранной ячейке
    cellInfoDisplay = new QTextEdit(this);
    cellInfoDisplay->setReadOnly(true);
    cellInfoDisplay->setPlaceholderText("Выберите ячейку для просмотра информации...");
    cellInfoDisplay->setMaximumWidth(300);
    contentSplitter->addWidget(cellInfoDisplay, 1); // 1/4 ширины
    
    mainVLayout->addLayout(contentSplitter);

    // Связываем кнопки
    connect(configButton, &QPushButton::clicked, this, &MainWindow::showConfigDialog);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadConfig);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveConfig);

    // Создаем layout из конфига
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
        
        // Автоматически сохраняем конфиг после редактирования
        if (configManager->saveConfig("config.json")) {
            qDebug() << "Конфиг автоматически сохранен";
        }
    }
}
void MainWindow::updateTemperatureGauges()
{
    // Пока ничего не делаем, или потом сюда добавим обновление спидометров
}
QWidget* MainWindow::createCellWidget(const CellInfo& cellInfo, int colIndex, int cellIndex, const QList<int>& parentPath)
{

    qDebug() << "Создание ячейки:" << colIndex << cellIndex << "content:" << cellInfo.content << "value:" << cellInfo.value << "unit:" << cellInfo.unit;
    QList<int> currentPath = parentPath;
    currentPath << cellIndex;
    
    ClickableFrame* cellFrame = new ClickableFrame(colIndex, cellIndex, currentPath);
    cellFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    cellFrame->setLineWidth(2);
    cellFrame->setStyleSheet("QFrame { "
                           "background-color: #f8f8f8; "
                           "border: 2px solid #a0a0a0; "
                           "border-top: 2px solid #606060; "
                           "border-left: 2px solid #606060; "
                           "} "
                           "QFrame:hover { background-color: #e8e8e8; }");
    
    QVBoxLayout* cellLayout = new QVBoxLayout(cellFrame);
    cellLayout->setSpacing(4);
    cellLayout->setContentsMargins(8, 8, 8, 8);
    
    // Основное содержимое ячейки с значением
    QHBoxLayout* mainContentLayout = new QHBoxLayout;
    
    QLabel* cellLabel = new QLabel(cellInfo.content);
    cellLabel->setWordWrap(true);
    mainContentLayout->addWidget(cellLabel, 1);
    
    // Отображаем значение, если оно есть
    QString displayValue = cellInfo.value;
    if (!displayValue.isEmpty() && !cellInfo.unit.isEmpty()) {
        displayValue += " " + cellInfo.unit;
    }
if (cellInfo.content.contains("Температура", Qt::CaseInsensitive)) {
    TemperatureGauge *tempGauge = new TemperatureGauge;
    QString valStr = cellInfo.value;
valStr.remove(QRegularExpression("[^\\d.-]")); // оставляем цифры и точку
bool ok;
double temp = valStr.toDouble(&ok);
if (ok) tempGauge->setTemperature(temp);

mainContentLayout->addWidget(tempGauge, 0, Qt::AlignRight);
} else if (!displayValue.isEmpty()) {
    QLabel* valueLabel = new QLabel(displayValue);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mainContentLayout->addWidget(valueLabel);
}
    cellLayout->addLayout(mainContentLayout);
    
    // Если есть подъячейки, отображаем их ВЕРТИКАЛЬНО
    if (!cellInfo.subCells.isEmpty()) {
        QFrame* subCellsFrame = new QFrame;
        subCellsFrame->setFrameStyle(QFrame::Box);
        subCellsFrame->setLineWidth(2);
        subCellsFrame->setStyleSheet("QFrame { "
                                   "background-color: #f0f0f0; "
                                   "border: 2px solid #909090; "
                                   "border-top: 2px solid #505050; "
                                   "border-left: 2px solid #505050; "
                                   "}");
        
        QVBoxLayout* subCellsLayout = new QVBoxLayout(subCellsFrame);
        subCellsLayout->setSpacing(2);
        subCellsLayout->setContentsMargins(4, 4, 4, 4);
        
        for (int i = 0; i < cellInfo.subCells.size(); ++i) {
            QWidget* subCellWidget = createSubCellWidget(cellInfo.subCells[i], colIndex, i, currentPath);
            subCellsLayout->addWidget(subCellWidget);
        }
        
        cellLayout->addWidget(subCellsFrame);
    }
    
    // Связываем клик
    connect(cellFrame, &ClickableFrame::clicked, this, &MainWindow::onCellClicked);
    
    return cellFrame;
}

QWidget* MainWindow::createSubCellWidget(const CellInfo& cellInfo, int colIndex, int subCellIndex, const QList<int>& parentPath)
{
    QList<int> currentPath = parentPath;
    currentPath << subCellIndex;
    
    ClickableFrame* subCellFrame = new ClickableFrame(colIndex, subCellIndex, currentPath);
    subCellFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    subCellFrame->setLineWidth(1);
    subCellFrame->setStyleSheet("QFrame { "
                              "background-color: #f0f0f0; "
                              "border: 1px solid #808080; "
                              "border-top: 1px solid #404040; "
                              "border-left: 1px solid #404040; "
                              "} "
                              "QFrame:hover { background-color: #e0e0e0; }");
    
    QHBoxLayout* subCellLayout = new QHBoxLayout(subCellFrame);
    subCellLayout->setContentsMargins(6, 4, 6, 4);
    
    // Название подъячейки
    QLabel* subCellLabel = new QLabel(cellInfo.content);
    subCellLabel->setWordWrap(true);
    subCellLayout->addWidget(subCellLabel, 1);
    
    // Значение подъячейки
    QString displayValue = cellInfo.value;
    if (!displayValue.isEmpty() && !cellInfo.unit.isEmpty()) {
        displayValue += " " + cellInfo.unit;
    }
    
    if (cellInfo.content.contains("Температура", Qt::CaseInsensitive)) {
        TemperatureGauge *tempGauge = new TemperatureGauge;
        bool ok;
        int temp = cellInfo.value.toInt(&ok);
        if (ok) tempGauge->setTemperature(temp);
        subCellLayout->addWidget(tempGauge, 0, Qt::AlignRight);
    }
    else if (!displayValue.isEmpty()) {
        QLabel* valueLabel = new QLabel(displayValue);
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        subCellLayout->addWidget(valueLabel);
    }
    
    // Связываем клик
    connect(subCellFrame, &ClickableFrame::clicked, this, &MainWindow::onCellClicked);
    
    return subCellFrame;
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
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить конфигурацию", "config.json", "JSON Files (*.json)");
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
        columnFrame->setLineWidth(3);
        columnFrame->setStyleSheet("QFrame { "
                                 "background-color: #e8e8e8; "
                                 "border: 3px solid #707070; "
                                 "border-top: 3px solid #303030; "
                                 "border-left: 3px solid #303030; "
                                 "}");
        
        // Вертикальный layout для колонки
        QVBoxLayout *columnLayout = new QVBoxLayout(columnFrame);
        columnLayout->setSpacing(4);
        columnLayout->setContentsMargins(4, 4, 4, 4);
        
        // Заголовок колонки - ОСТАВЛЯЕМ ЖИРНЫЙ ШРИФТ
        QLabel *titleLabel = new QLabel(columnConfig.name);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("QLabel { "
                                "background-color: #d0d0d0; "
                                "padding: 10px; "
                                "font-weight: bold; " // ОСТАВЛЯЕМ ЖИРНЫЙ ТОЛЬКО ЗДЕСЬ
                                "font-size: 14px; "
                                "border: 2px solid #606060; "
                                "border-top: 2px solid #202020; "
                                "border-left: 2px solid #202020; "
                                "}");
        titleLabel->setMinimumHeight(40);
        columnLayout->addWidget(titleLabel);
        
        // Создаем ячейки в колонке
        for (int cell = 0; cell < columnConfig.cellCount && cell < columnConfig.cells.size(); ++cell) {
            QWidget* cellWidget = createCellWidget(columnConfig.cells[cell], col, cell);
            columnLayout->addWidget(cellWidget);
        }
        
        // Добавляем растягивающее пространство в конец колонки
        columnLayout->addStretch();
        
        // Добавляем колонку в основной layout с растягиванием
        mainLayout->addWidget(columnFrame, 1);



    }



        updateTemperatureGauges();
}

void MainWindow::onCellClicked(int col, int cell, const QList<int>& subCellPath)
{
    qDebug() << "Клик по ячейке:" << col << cell << "путь:" << subCellPath;
    
    // Находим содержимое ячейки по пути
    if (col < configManager->getColumns().size()) {
        const ColumnConfig column = configManager->getColumns()[col];
        
        const CellInfo* currentCell = nullptr;
        QString pathDescription = QString("Колонка: %1").arg(col + 1);
        QString cellName = "";
        
        if (subCellPath.size() == 1) {
            // Клик на основную ячейку
            if (cell < column.cells.size()) {
                currentCell = &column.cells[cell];
                cellName = currentCell->content;
                pathDescription += QString(" → Ячейка: %1").arg(cell + 1);
            }
        } else {
            // Клик на подъячейку
            if (subCellPath[0] < column.cells.size()) {
                currentCell = &column.cells[subCellPath[0]];
                pathDescription += QString(" → Ячейка: %1").arg(subCellPath[0] + 1);
                cellName = currentCell->content;
                
                // Проходим по пути подъячеек
                for (int i = 1; i < subCellPath.size(); i++) {
                    int subIndex = subCellPath[i];
                    if (subIndex < currentCell->subCells.size()) {
                        currentCell = &currentCell->subCells[subIndex];
                        pathDescription += QString(" → Подъячейка: %1").arg(subIndex + 1);
                        cellName = currentCell->content;
                    } else {
                        break;
                    }
                }
            }
        }
        
        if (currentCell) {
            showCellInfo(pathDescription, cellName, *currentCell);
        }
    }
}

void MainWindow::showCellInfo(const QString& pathDescription, const QString& cellName, const CellInfo& cellInfo)
{
    QString infoText;
    infoText += pathDescription + "\n\n";
    infoText += QString("Название: %1\n").arg(cellName);
    
    QString displayValue = cellInfo.value;
    if (!displayValue.isEmpty() && !cellInfo.unit.isEmpty()) {
        displayValue += " " + cellInfo.unit;
    }
    
    if (!displayValue.isEmpty()) {
        infoText += QString("Значение: %1\n").arg(displayValue);
    }
    
    if (!cellInfo.subCells.isEmpty()) {
        infoText += QString("\nПодъячеек: %1").arg(cellInfo.subCells.size());
        for (int i = 0; i < cellInfo.subCells.size(); ++i) {
            QString subDisplayValue = cellInfo.subCells[i].value;
            if (!subDisplayValue.isEmpty() && !cellInfo.subCells[i].unit.isEmpty()) {
                subDisplayValue += " " + cellInfo.subCells[i].unit;
            }
            infoText += QString("\n- %1: %2").arg(cellInfo.subCells[i].content).arg(subDisplayValue);
        }
    }
    
    cellInfoDisplay->setPlainText(infoText);
}
#include "mainwindow.moc"
