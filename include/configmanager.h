#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct CellInfo {
    QString content;
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

    // Геттеры
    int getColumnCount() const { return columns.size(); }
    QList<ColumnConfig> getColumns() const { return columns; }
    QStringList getColumnNames() const;
    QList<int> getCellCounts() const;

    // Сеттеры
    void setColumns(const QList<ColumnConfig>& newColumns);
    void updateColumn(int index, const QString& name, int cellCount, const QList<CellInfo>& cellInfos);

    // Создание конфига по умолчанию
    void createDefaultConfig();

private:
    QList<ColumnConfig> columns;

    ColumnConfig columnFromJson(const QJsonObject& json);
    QJsonObject columnToJson(const ColumnConfig& column) const;
};

#endif // CONFIGMANAGER_H
