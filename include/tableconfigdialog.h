#ifndef TABLECONFIGDIALOG_H
#define TABLECONFIGDIALOG_H

#include <QDialog>
#include <QList>
#include <QVBoxLayout>  // ДОБАВИТЬ ЭТОТ INCLUDE
#include <QHBoxLayout>  // ДОБАВИТЬ ЭТОТ INCLUDE
#include "configmanager.h"

class QSpinBox;
class QFormLayout;
class QLineEdit;
class QDialogButtonBox;
class QLabel;
class QTextEdit;
class QScrollArea;
class QFrame;

class TableConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableConfigDialog(QWidget *parent = nullptr);
    
    int getColumnCount() const;
    QStringList getColumnNames() const;
    QList<int> getCellCounts() const;
    QList<ColumnConfig> getColumnsConfig() const;
    
    void setConfigData(const QList<ColumnConfig>& columns);

private slots:
    void updateColumnInputs(int count);
    void accept() override;

private:
    void setupUI();
    void setupColumnUI(int index, const ColumnConfig* config = nullptr);

    QSpinBox *columnCountSpinBox;
    QVBoxLayout *columnsContainerLayout;
    QDialogButtonBox *buttonBox;
    
    struct ColumnWidgets {
        QLineEdit* nameEdit;
        QSpinBox* cellCountSpin;
        QList<QTextEdit*> cellEdits;
        QWidget* columnWidget;
        QVBoxLayout* cellsLayout;
    };
    
    QVector<ColumnWidgets> columnWidgets;
    QList<ColumnConfig> columnsConfig;
};

#endif // TABLECONFIGDIALOG_H
