#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>

struct CellInfo {
    QString content;
    QString value;      // Текущее значение для отображения
    QString unit;       // Единица измерения
    QList<CellInfo> subCells; // Рекурсивная структура для вложенных ячеек
};

struct ColumnConfig {
    QString name;
    int cellCount;
    QList<CellInfo> cells;
};

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);

    bool loadConfig(const QString& filename);
    bool saveConfig(const QString& filename) const;
    bool configExists() const;

    // Геттеры
    int getColumnCount() const { return columns.size(); }
    QList<ColumnConfig> getColumns() const { return columns; }
    QStringList getColumnNames() const;
    QList<int> getCellCounts() const;

    // Методы для работы со значениями
    bool updateCellValue(int columnIndex, int cellIndex, const QString& value);
    bool updateSubCellValue(int columnIndex, int cellIndex, int subCellIndex, const QString& value);
    QString getCellValue(int columnIndex, int cellIndex) const;

    // Сеттеры
    void setColumns(const QList<ColumnConfig>& newColumns);
    void updateColumn(int index, const QString& name, int cellCount, const QList<CellInfo>& cellInfos);

    // Создание конфига по умолчанию
    void createDefaultConfig();

    // Путь к текущему конфигу
    QString getConfigPath() const { return configPath; }
    void setConfigPath(const QString& path) { configPath = path; }

private:
    QList<ColumnConfig> columns;
    QString configPath;

    ColumnConfig columnFromJson(const QJsonObject& json);
    QJsonObject columnToJson(const ColumnConfig& column) const;
    CellInfo cellFromJson(const QJsonObject& json);
    QJsonObject cellToJson(const CellInfo& cell) const;
};

#endif // CONFIGMANAGER_H
