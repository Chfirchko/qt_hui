#ifndef TABLECONFIGDIALOG_H
#define TABLECONFIGDIALOG_H

#include <QDialog>

class QSpinBox;
class QFormLayout;
class QLineEdit;
class QDialogButtonBox;

class TableConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableConfigDialog(QWidget *parent = nullptr);
    
    int getColumnCount() const;
    QStringList getColumnNames() const;
    int getRowCount() const;

private slots:
    void updateColumnInputs(int count);
    void accept() override;

private:
    void setupUI();

    QSpinBox *columnCountSpinBox;
    QSpinBox *rowCountSpinBox;
    QFormLayout *columnNamesLayout;
    QDialogButtonBox *buttonBox;
    QVector<QLineEdit*> columnNameInputs;
    QStringList columnNames;
};

#endif // TABLECONFIGDIALOG_H
