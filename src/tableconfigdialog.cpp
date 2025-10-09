#include "tableconfigdialog.h"

#include <QSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QScrollArea>
#include <QFrame>

TableConfigDialog::TableConfigDialog(QWidget *parent)
    : QDialog(parent)
    , columnCountSpinBox(nullptr)
    , columnsContainerLayout(nullptr)
    , buttonBox(nullptr)
{
    setupUI();
}

int TableConfigDialog::getColumnCount() const
{
    return columnCountSpinBox->value();
}

QStringList TableConfigDialog::getColumnNames() const
{
    QStringList names;
    for (const auto& colWidgets : columnWidgets) {
        names.append(colWidgets.nameEdit->text());
    }
    return names;
}

QList<int> TableConfigDialog::getCellCounts() const
{
    QList<int> counts;
    for (const auto& colWidgets : columnWidgets) {
        counts.append(colWidgets.cellCountSpin->value());
    }
    return counts;
}

QList<ColumnConfig> TableConfigDialog::getColumnsConfig() const
{
    return columnsConfig;
}

void TableConfigDialog::setConfigData(const QList<ColumnConfig>& columns)
{
    columnCountSpinBox->setValue(columns.size());
    
    // Очищаем текущие виджеты
    while (!columnWidgets.isEmpty()) {
        ColumnWidgets widgets = columnWidgets.takeLast();
        if (widgets.columnWidget) {
            widgets.columnWidget->deleteLater();
        }
    }
    
    // Создаем новые виджеты с данными из конфига
    for (int i = 0; i < columns.size(); ++i) {
        setupColumnUI(i, &columns[i]);
    }
}

void TableConfigDialog::updateColumnInputs(int count)
{
    // Очищаем предыдущие поля
    while (!columnWidgets.isEmpty()) {
        ColumnWidgets widgets = columnWidgets.takeLast();
        if (widgets.columnWidget) {
            widgets.columnWidget->deleteLater();
        }
    }
    
    // Создаем новые поля
    for (int i = 0; i < count; ++i) {
        setupColumnUI(i);
    }
}

void TableConfigDialog::setupColumnUI(int index, const ColumnConfig* config)
{
    ColumnWidgets widgets;
    
    // Создаем виджет для колонки
    QFrame* columnFrame = new QFrame;
    columnFrame->setFrameStyle(QFrame::Box);
    columnFrame->setLineWidth(1);
    
    QVBoxLayout* columnLayout = new QVBoxLayout(columnFrame);
    
    // Поля для названия и количества ячеек
    QFormLayout* headerLayout = new QFormLayout;
    
    QLabel* nameLabel = new QLabel(QString("Название колонки %1:").arg(index + 1));
    widgets.nameEdit = new QLineEdit;
    if (config) {
        widgets.nameEdit->setText(config->name);
    } else {
        widgets.nameEdit->setText(QString("Колонка %1").arg(index + 1));
    }
    headerLayout->addRow(nameLabel, widgets.nameEdit);
    
    QLabel* cellCountLabel = new QLabel(QString("Кол-во ячеек в колонке %1:").arg(index + 1));
    widgets.cellCountSpin = new QSpinBox;
    widgets.cellCountSpin->setRange(1, 20);
    if (config) {
        widgets.cellCountSpin->setValue(config->cellCount);
    } else {
        widgets.cellCountSpin->setValue(index + 2);
    }
    headerLayout->addRow(cellCountLabel, widgets.cellCountSpin);
    
    columnLayout->addLayout(headerLayout);
    
    // Область для содержимого ячеек
    QLabel* cellsLabel = new QLabel("Содержимое ячеек:");
    columnLayout->addWidget(cellsLabel);
    
    widgets.cellsLayout = new QVBoxLayout;
    columnLayout->addLayout(widgets.cellsLayout);
    
    // Создаем поля для ячеек
    int cellCount = config ? config->cellCount : (index + 2);
    for (int cellIdx = 0; cellIdx < cellCount; ++cellIdx) {
        QLabel* cellLabel = new QLabel(QString("Ячейка %1:").arg(cellIdx + 1));
        QTextEdit* cellEdit = new QTextEdit;
        cellEdit->setMaximumHeight(60);
        cellEdit->setPlaceholderText(QString("Введите содержимое ячейки %1-%2").arg(index + 1).arg(cellIdx + 1));
        
        if (config && cellIdx < config->cells.size()) {
            cellEdit->setPlainText(config->cells[cellIdx].content);
        } else {
            cellEdit->setPlainText(QString("Содержимое %1-%2").arg(index + 1).arg(cellIdx + 1));
        }
        
        widgets.cellsLayout->addWidget(cellLabel);
        widgets.cellsLayout->addWidget(cellEdit);
        widgets.cellEdits.append(cellEdit);
    }
    
    widgets.columnWidget = columnFrame;
    columnWidgets.append(widgets);
    
    // Добавляем в контейнер
    if (columnsContainerLayout) {
        columnsContainerLayout->addWidget(columnFrame);
    }
}

void TableConfigDialog::accept()
{
    columnsConfig.clear();
    
    for (int i = 0; i < columnWidgets.size(); ++i) {
        ColumnConfig config;
        config.name = columnWidgets[i].nameEdit->text().trimmed();
        if (config.name.isEmpty()) {
            config.name = QString("Колонка %1").arg(i + 1);
        }
        
        config.cellCount = columnWidgets[i].cellCountSpin->value();
        
        // Сохраняем содержимое ячеек
        for (int cellIdx = 0; cellIdx < columnWidgets[i].cellEdits.size() && cellIdx < config.cellCount; ++cellIdx) {
            CellInfo cell;
            cell.content = columnWidgets[i].cellEdits[cellIdx]->toPlainText().trimmed();
            if (cell.content.isEmpty()) {
                cell.content = QString("Ячейка %1-%2").arg(i + 1).arg(cellIdx + 1);
            }
            config.cells.append(cell);
        }
        
        columnsConfig.append(config);
    }
    
    QDialog::accept();
}

void TableConfigDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    
    // Группа для основных параметров
    auto *mainGroup = new QGroupBox("Основные параметры", this);
    auto *mainLayout = new QVBoxLayout(mainGroup);
    
    auto *formLayout = new QFormLayout;
    
    columnCountSpinBox = new QSpinBox(this);
    columnCountSpinBox->setRange(1, 6);
    columnCountSpinBox->setValue(3);
    formLayout->addRow("Количество колонок:", columnCountSpinBox);
    
    mainLayout->addLayout(formLayout);
    layout->addWidget(mainGroup);
    
    // Область с прокруткой для колонок
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    
    QWidget* scrollWidget = new QWidget;
    columnsContainerLayout = new QVBoxLayout(scrollWidget);
    scrollArea->setWidget(scrollWidget);
    
    layout->addWidget(scrollArea);
    
    // Инициализируем поля
    updateColumnInputs(columnCountSpinBox->value());
    
    // Связываем сигнал изменения количества колонок
    connect(columnCountSpinBox, &QSpinBox::valueChanged, 
            this, &TableConfigDialog::updateColumnInputs);
    
    // Кнопки OK/Cancel
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | 
                                    QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    setWindowTitle("Настройка вертикального разделения");
    resize(600, 500);
}
