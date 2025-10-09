#include "tableconfigdialog.h"

#include <QSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

TableConfigDialog::TableConfigDialog(QWidget *parent)
    : QDialog(parent)
    , columnCountSpinBox(nullptr)
    , rowCountSpinBox(nullptr)
    , columnNamesLayout(nullptr)
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
    return columnNames;
}

int TableConfigDialog::getRowCount() const
{
    return rowCountSpinBox->value();
}

void TableConfigDialog::updateColumnInputs(int count)
{
    // Очищаем предыдущие поля
    QLayoutItem* child;
    while ((child = columnNamesLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    
    // Создаем новые поля для названий столбцов
    columnNameInputs.clear();
    for (int i = 0; i < count; ++i) {
        auto *label = new QLabel(QString("Столбец %1:").arg(i + 1));
        auto *lineEdit = new QLineEdit(QString("Столбец %1").arg(i + 1));
        columnNamesLayout->addRow(label, lineEdit);
        columnNameInputs.append(lineEdit);
    }
}

void TableConfigDialog::accept()
{
    columnNames.clear();
    for (auto *input : columnNameInputs) {
        QString text = input->text().trimmed();
        if (text.isEmpty()) {
            text = QString("Столбец %1").arg(columnNames.size() + 1);
        }
        columnNames.append(text);
    }
    QDialog::accept();
}

void TableConfigDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    
    // Форма для ввода параметров
    auto *formLayout = new QFormLayout;
    
    columnCountSpinBox = new QSpinBox(this);
    columnCountSpinBox->setRange(1, 20);
    columnCountSpinBox->setValue(3);
    formLayout->addRow("Количество столбцов:", columnCountSpinBox);
    
    rowCountSpinBox = new QSpinBox(this);
    rowCountSpinBox->setRange(1, 100);
    rowCountSpinBox->setValue(5);
    formLayout->addRow("Количество строк:", rowCountSpinBox);
    
    layout->addLayout(formLayout);
    
    // Область для названий столбцов
    auto *columnsLabel = new QLabel("Названия столбцов:", this);
    layout->addWidget(columnsLabel);
    
    columnNamesLayout = new QFormLayout;
    layout->addLayout(columnNamesLayout);
    
    // Инициализируем поля для названий
    updateColumnInputs(columnCountSpinBox->value());
    
    // Связываем сигнал изменения количества столбцов
    connect(columnCountSpinBox, &QSpinBox::valueChanged, 
            this, &TableConfigDialog::updateColumnInputs);
    
    // Кнопки OK/Cancel
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | 
                                    QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    setWindowTitle("Настройка таблицы");
    resize(400, 300);
}
