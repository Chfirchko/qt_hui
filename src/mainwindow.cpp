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
#include <QMap>
#include <QStringList>

#include "mainwindow.h"

// -------------------------------------------------------------
// Вспомогательные данные в анонимном пространстве (не трогаем header)
// -------------------------------------------------------------
namespace {
    // История значений: ключ -> список значений (последовательность)
    QMap<QString, QStringList> g_history;

    // Последний выбранный путь (для отображения в правой панели)
    int g_lastSelectedCol = -1;
    int g_lastSelectedCell = -1;
    QList<int> g_lastSelectedSubPath;

    // Помощник: формирует ключ для истории по индексам
    QString makeHistoryKey(int col, int cell, int sub = -1) {
        if (col < 0 || cell < 0) return QString();
        if (sub >= 0) {
            return QString("col%1/cell%2/sub%3").arg(col).arg(cell).arg(sub);
        } else {
            return QString("col%1/cell%2").arg(col).arg(cell);
        }
    }

    // Добавляет value в историю для key, но только если value не пуст и отличается от последней записи
    void appendToHistory(const QString& key, const QString& value) {
        if (key.isEmpty() || value.isEmpty()) return;
        QStringList &list = g_history[key];
        if (list.isEmpty() || list.last() != value) {
            list.append(value);
        }
    }
}

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
    , configButton(nullptr)
    , scrollArea(nullptr)
    , contentWidget(nullptr)
    , mainLayout(nullptr)
    , configManager(new ConfigManager(this))
    , cellInfoDisplay(nullptr)
{
    setupUI();
    setupMenu();

    // Лог загрузки конфигурации при старте (ConfigManager уже ищет config в ctor)
    if (configManager->configExists()) {
        qDebug() << "Автоматически загружен конфиг:" << configManager->getConfigPath();
    } else {
        qDebug() << "Используется конфиг по умолчанию";
    }

    // Таймер обновления
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::refreshData);
    updateTimer->start(1000); // обновление каждую секунду
}

MainWindow::~MainWindow()
{
    if (configManager) {
        configManager->deleteLater();
    }
}

// --------------------- UI setup ---------------------
void MainWindow::setupUI()
{
    QWidget *centralWgt = new QWidget(this);
    setCentralWidget(centralWgt);

    auto *mainVLayout = new QVBoxLayout(centralWgt);

    auto *topPanel = new QHBoxLayout;

    configButton = new QPushButton("Настроить разделение", this);
    topPanel->addWidget(configButton);

    auto *loadButton = new QPushButton("Загрузить конфиг", this);
    topPanel->addWidget(loadButton);

    auto *saveButton = new QPushButton("Сохранить конфиг", this);
    topPanel->addWidget(saveButton);

    topPanel->addStretch();

    mainVLayout->addLayout(topPanel);

    auto *contentSplitter = new QHBoxLayout;

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    contentWidget = new QWidget;
    mainLayout = new QHBoxLayout(contentWidget);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    scrollArea->setWidget(contentWidget);

    contentSplitter->addWidget(scrollArea, 3);

    cellInfoDisplay = new QTextEdit(this);
    cellInfoDisplay->setReadOnly(true);
    cellInfoDisplay->setPlaceholderText("Выберите ячейку для просмотра информации...");
    cellInfoDisplay->setMaximumWidth(300);
    contentSplitter->addWidget(cellInfoDisplay, 1);

    mainVLayout->addLayout(contentSplitter);

    connect(configButton, &QPushButton::clicked, this, &MainWindow::showConfigDialog);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadConfig);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveConfig);

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

        if (configManager->saveConfig("config.json")) {
            qDebug() << "Конфиг автоматически сохранен";
        }
    }
}

// Температуры пока отдельно не трогаем
void MainWindow::updateTemperatureGauges()
{
    // оставлено пустым (температуры обновляются в updateCellWidget)
}

// --------------------- Виджеты ячеек ---------------------
// Важное изменение: ставим свойства на ClickableFrame: "col","cell" и для sub - "sub"
QWidget* MainWindow::createCellWidget(const CellInfo& cellInfo, int colIndex, int cellIndex, const QList<int>& parentPath)
{
    qDebug() << "Создание ячейки:" << colIndex << cellIndex << "content:" << cellInfo.content << "value:" << cellInfo.value << "unit:" << cellInfo.unit;
    QList<int> currentPath = parentPath;
    currentPath << cellIndex;

    ClickableFrame* cellFrame = new ClickableFrame(colIndex, cellIndex, currentPath);
    // Сохраняем индексы как свойства, чтобы потом при обновлении определить ключ
    cellFrame->setProperty("col", colIndex);
    cellFrame->setProperty("cell", cellIndex);
    // sub не ставим для основной ячейки

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

    QHBoxLayout* mainContentLayout = new QHBoxLayout;

    QLabel* cellLabel = new QLabel(cellInfo.content);
    cellLabel->setWordWrap(true);
    mainContentLayout->addWidget(cellLabel, 1);

    // Подготавливаем значение для отображения (если нет value, попытаемся вычислить из subCells)
    QString displayValue = cellInfo.value;
    if (displayValue.isEmpty() && !cellInfo.subCells.isEmpty()) {
        double sum = 0;
        int count = 0;
        for (const CellInfo& sc : cellInfo.subCells) {
            bool ok;
            double v = sc.value.toDouble(&ok);
            if (ok) { sum += v; ++count; }
        }
        if (count > 0) displayValue = QString::number(sum / count, 'f', 2);
    }
    if (!displayValue.isEmpty() && !cellInfo.unit.isEmpty()) {
        displayValue += " " + cellInfo.unit;
    }

    if (cellInfo.content.contains("Температура", Qt::CaseInsensitive)) {
        TemperatureGauge *tempGauge = new TemperatureGauge;
        bool ok;
        double temp = cellInfo.value.toDouble(&ok);
        if (!ok && !cellInfo.subCells.isEmpty()) {
            temp = cellInfo.subCells[0].value.toDouble(&ok);
        }
        if (ok) tempGauge->setTemperature(temp);
        mainContentLayout->addWidget(tempGauge, 0, Qt::AlignRight);
        temperatureGauges.append(tempGauge);
    } else {
        // Создаем value QLabel и даём ему понятное имя (objectName), чтобы найти при обновлении
        QLabel* valueLabel = new QLabel(displayValue);
        valueLabel->setObjectName("valueLabel");
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        mainContentLayout->addWidget(valueLabel);
    }

    cellLayout->addLayout(mainContentLayout);

    // Подъячеки
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

    connect(cellFrame, &ClickableFrame::clicked, this, &MainWindow::onCellClicked);

    return cellFrame;
}

QWidget* MainWindow::createSubCellWidget(const CellInfo& cellInfo, int colIndex, int subCellIndex, const QList<int>& parentPath)
{
    QList<int> currentPath = parentPath;
    currentPath << subCellIndex;

    ClickableFrame* subCellFrame = new ClickableFrame(colIndex, subCellIndex, currentPath);
    // Сохраняем свойства: col/cell/sub
    subCellFrame->setProperty("col", colIndex);
    subCellFrame->setProperty("cell", parentPath.isEmpty() ? -1 : parentPath.last()); // parentPath.last() — индекс основной ячейки
    subCellFrame->setProperty("sub", subCellIndex);

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

    QLabel* subCellLabel = new QLabel(cellInfo.content);
    subCellLabel->setWordWrap(true);
    subCellLayout->addWidget(subCellLabel, 1);

    // Подготавливаем displayValue для подъячейки
    QString displayValue;
    bool ok;
    double val = cellInfo.value.toDouble(&ok);
    if (ok) {
        displayValue = QString::number(val, 'f', 2);
    } else if (!cellInfo.value.isEmpty()) {
        displayValue = cellInfo.value; // строковые значения (время, SN)
    }

    if (!displayValue.isEmpty() && !cellInfo.unit.isEmpty()) {
        displayValue += " " + cellInfo.unit;
    }

    // Создаем QLabel для значения подъячейки и даём objectName чтобы обновлять
    QLabel* valueLabel = new QLabel(displayValue);
    valueLabel->setObjectName("subValueLabel");
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    subCellLayout->addWidget(valueLabel);

    connect(subCellFrame, &ClickableFrame::clicked, this, &MainWindow::onCellClicked);

    return subCellFrame;
}

// --------------------- Обновление данных (таймер) ---------------------
void MainWindow::refreshData()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/../data/config.json";
    if (configManager->loadConfig(configPath)) {
        // обновляем только значения в существующих виджетах
        updateCellWidgets();
    } else {
        qWarning() << "Не удалось загрузить конфиг для обновления:" << configPath;
    }
}

void MainWindow::updateCellWidgets()
{
    const QList<ColumnConfig>& columns = configManager->getColumns();

    for (int col = 0; col < mainLayout->count(); ++col) {
        QWidget* columnWidget = mainLayout->itemAt(col)->widget();
        if (!columnWidget) continue;

        QVBoxLayout* columnLayout = qobject_cast<QVBoxLayout*>(columnWidget->layout());
        if (!columnLayout) continue;

        // Если в конфиге меньше колонок, пропускаем
        if (col >= columns.size()) continue;
        const ColumnConfig& columnConfig = columns[col];

        // Пропускаем первый виджет - это заголовок колонки
        for (int cellIndex = 0; cellIndex < columnConfig.cells.size(); ++cellIndex) {
            int widgetIndex = cellIndex + 1;
            if (widgetIndex >= columnLayout->count()) break;

            QWidget* cellWidget = columnLayout->itemAt(widgetIndex)->widget();
            if (cellWidget) {
                updateCellWidget(cellWidget, columnConfig.cells[cellIndex]);
            }
        }
    }

    // После обновления левой части — обновляем правую панель (историю)
    updateRightPanel();
}

void MainWindow::updateCellWidget(QWidget* cellWidget, const CellInfo& cellInfo)
{
    // 1) Обновляем основной valueLabel (если есть)
    QLabel* valueLabel = cellWidget->findChild<QLabel*>("valueLabel");
    QString mainDisplay;

    // Если основной value пуст, но есть subCells — усредняем (как отображали при создании)
    if (!cellInfo.value.isEmpty()) {
        mainDisplay = cellInfo.value;
    } else if (!cellInfo.subCells.isEmpty()) {
        double sum = 0;
        int count = 0;
        for (const CellInfo& sc : cellInfo.subCells) {
            bool ok;
            double v = sc.value.toDouble(&ok);
            if (ok) { sum += v; ++count; }
        }
        if (count > 0) mainDisplay = QString::number(sum / count, 'f', 2);
    }

    if (!mainDisplay.isEmpty() && !cellInfo.unit.isEmpty()) {
        mainDisplay += " " + cellInfo.unit;
    }

    if (valueLabel) {
        valueLabel->setText(mainDisplay);
    }

    // 2) Обновляем TemperatureGauge, если есть (работает на основном виджете)
    QList<TemperatureGauge*> gauges = cellWidget->findChildren<TemperatureGauge*>();
    for (TemperatureGauge* gauge : gauges) {
        bool ok;
        double temp = cellInfo.value.toDouble(&ok);
        if (!ok && !cellInfo.subCells.isEmpty()) {
            temp = cellInfo.subCells[0].value.toDouble(&ok);
        }
        if (ok) gauge->setTemperature(temp);
    }

    // 3) Сохраняем основное значение в историю (если есть)
    // Для этого берем свойства "col" и "cell" у корневого фрейма
    QVariant vcol = cellWidget->property("col");
    QVariant vcell = cellWidget->property("cell");
    int colIdx = vcol.isValid() ? vcol.toInt() : -1;
    int cellIdx = vcell.isValid() ? vcell.toInt() : -1;
    QString mainKey = makeHistoryKey(colIdx, cellIdx, -1);
    if (!mainDisplay.isEmpty()) appendToHistory(mainKey, mainDisplay);

    // 4) Рекурсивно обновляем подъячейки: ищем фреймы с property "sub"
    // Мы знаем, что createCellWidget обернул подъячеки в QFrame->QVBoxLayout
    QList<QFrame*> frames = cellWidget->findChildren<QFrame*>();
    for (QFrame* f : frames) {
        // Только те фреймы, которые являются нашим subCellFrame — имеют property "sub"
        QVariant vsub = f->property("sub");
        if (!vsub.isValid()) continue; // пропускаем контейнеры без sub
        int subIdx = vsub.toInt();
        // Найдём соответствующий CellInfo в массиве
        if (subIdx >= 0 && subIdx < cellInfo.subCells.size()) {
            // Найдём QLabel внутри f с objectName "subValueLabel"
            QLabel* subLabel = f->findChild<QLabel*>("subValueLabel");
            const CellInfo& subInfo = cellInfo.subCells[subIdx];

            QString subDisplay;
            bool ok;
            double subVal = subInfo.value.toDouble(&ok);
            if (ok) {
                subDisplay = QString::number(subVal, 'f', 2);
            } else if (!subInfo.value.isEmpty()) {
                subDisplay = subInfo.value;
            }
            if (!subDisplay.isEmpty() && !subInfo.unit.isEmpty()) {
                subDisplay += " " + subInfo.unit;
            }
            if (subLabel) subLabel->setText(subDisplay);

            // Сохраняем в историю по ключу col/cell/sub
            QString subKey = makeHistoryKey(colIdx, cellIdx, subIdx);
            if (!subDisplay.isEmpty()) appendToHistory(subKey, subDisplay);
        }
    }
}

// --------------------- Загрузка/сохранение конфигов и layout ---------------------
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

        QFrame *columnFrame = new QFrame;
        columnFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
        columnFrame->setLineWidth(3);
        columnFrame->setStyleSheet("QFrame { "
                                 "background-color: #e8e8e8; "
                                 "border: 3px solid #707070; "
                                 "border-top: 3px solid #303030; "
                                 "border-left: 3px solid #303030; "
                                 "}");

        QVBoxLayout *columnLayout = new QVBoxLayout(columnFrame);
        columnLayout->setSpacing(4);
        columnLayout->setContentsMargins(4, 4, 4, 4);

        QLabel *titleLabel = new QLabel(columnConfig.name);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("QLabel { "
                                "background-color: #d0d0d0; "
                                "padding: 10px; "
                                "font-weight: bold; "
                                "font-size: 14px; "
                                "border: 2px solid #606060; "
                                "border-top: 2px solid #202020; "
                                "border-left: 2px solid #202020; "
                                "}");
        titleLabel->setMinimumHeight(40);
        columnLayout->addWidget(titleLabel);

        for (int cell = 0; cell < columnConfig.cellCount && cell < columnConfig.cells.size(); ++cell) {
            QWidget* cellWidget = createCellWidget(columnConfig.cells[cell], col, cell);
            columnLayout->addWidget(cellWidget);
        }

        columnLayout->addStretch();

        mainLayout->addWidget(columnFrame, 1);
    }

    // Первичная инициализация истории (можно не обязательно)
    updateCellWidgets();
    updateTemperatureGauges();
}

// --------------------- Клики и правая панель истории ---------------------
void MainWindow::onCellClicked(int col, int cell, const QList<int>& subCellPath)
{
    qDebug() << "Клик по ячейке:" << col << cell << "путь:" << subCellPath;
    // сохраняем последний выбранный путь в глобальные переменные
    g_lastSelectedCol = col;
    g_lastSelectedCell = cell;
    g_lastSelectedSubPath = subCellPath;

    // Отображаем информацию о выбранной ячейке (в том числе текущие значения)
    if (col < configManager->getColumns().size()) {
        const ColumnConfig column = configManager->getColumns()[col];

        const CellInfo* currentCell = nullptr;
        QString pathDescription = QString("Колонка: %1").arg(col + 1);
        QString cellName = "";

        if (subCellPath.size() == 1) {
            if (cell < column.cells.size()) {
                currentCell = &column.cells[cell];
                cellName = currentCell->content;
                pathDescription += QString(" → Ячейка: %1").arg(cell + 1);
            }
        } else {
            if (subCellPath[0] < column.cells.size()) {
                currentCell = &column.cells[subCellPath[0]];
                pathDescription += QString(" → Ячейка: %1").arg(subCellPath[0] + 1);
                cellName = currentCell->content;

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
    // Покажем краткую информацию о выбранной ячейке вверху правой панели,
    // а затем — всю историю (updateRightPanel делает это тоже).
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

    // Поместим краткую информацию в начало панели, а историю добавим ниже в updateRightPanel.
    // Здесь просто временно устанавливаем текст и затем updateRightPanel дополнит/перезапишет.
    cellInfoDisplay->setPlainText(infoText);

    // Обновим правую панель, чтобы включить историю + выбранную ячейку (updateRightPanel делает объединение)
    updateRightPanel();
}

// Формирует и показывает правую панель с историей всех ключей.
// Если есть "последняя выбранная" ячейка — она будет показана сверху.
void MainWindow::updateRightPanel()
{
    QString out;

    // Если есть выбранная ячейка — показываем её кратко сверху
    if (g_lastSelectedCol >= 0 && g_lastSelectedCell >= 0) {
        const QList<ColumnConfig>& cols = configManager->getColumns();
        if (g_lastSelectedCol < cols.size()) {
            const ColumnConfig &col = cols[g_lastSelectedCol];
            if (g_lastSelectedCell < col.cells.size()) {
                const CellInfo &ci = col.cells[g_lastSelectedCell];
                out += QString("Выбрано: %1 / %2\n").arg(col.name, ci.content);

                QString value = ci.value;
                if (value.isEmpty() && !ci.subCells.isEmpty()) {
                    // среднее
                    double sum = 0; int cnt = 0;
                    for (const CellInfo &s : ci.subCells) {
                        bool ok; double v = s.value.toDouble(&ok);
                        if (ok) { sum += v; ++cnt; }
                    }
                    if (cnt > 0) value = QString::number(sum / cnt, 'f', 2);
                }
                if (!value.isEmpty()) {
                    out += QString("Текущее значение: %1\n\n").arg(value);
                } else {
                    out += QString("\n");
                }
            }
        }
    }

    // Затем показываем всю историю: ключ -> v1, v2, ...
    for (auto it = g_history.constBegin(); it != g_history.constEnd(); ++it) {
        const QString &key = it.key();
        const QStringList &vals = it.value();
        if (vals.isEmpty()) continue;
        out += QString("%1: %2\n").arg(key, vals.join(", "));
    }

    cellInfoDisplay->setPlainText(out);
}

#include "mainwindow.moc"

