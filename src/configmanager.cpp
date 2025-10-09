#include "configmanager.h"
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{
    createDefaultConfig();
}

bool ConfigManager::loadConfig(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл конфигурации:" << filename;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Неверный JSON формат в файле:" << filename;
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("columns") || !root["columns"].isArray()) {
        qWarning() << "Отсутствует или неверный массив columns в конфиге";
        return false;
    }

    columns.clear();
    QJsonArray columnsArray = root["columns"].toArray();
    for (const QJsonValue& value : columnsArray) {
        if (value.isObject()) {
            ColumnConfig column = columnFromJson(value.toObject());
            columns.append(column);
        }
    }

    qDebug() << "Конфигурация загружена. Колонок:" << columns.size();
    return true;
}

bool ConfigManager::saveConfig(const QString& filename) const
{
    QJsonObject root;
    QJsonArray columnsArray;

    for (const ColumnConfig& column : columns) {
        columnsArray.append(columnToJson(column));
    }

    root["columns"] = columnsArray;

    QJsonDocument doc(root);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Не удалось создать файл конфигурации:" << filename;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Конфигурация сохранена в:" << filename;
    return true;
}

QStringList ConfigManager::getColumnNames() const
{
    QStringList names;
    for (const ColumnConfig& column : columns) {
        names.append(column.name);
    }
    return names;
}

QList<int> ConfigManager::getCellCounts() const
{
    QList<int> counts;
    for (const ColumnConfig& column : columns) {
        counts.append(column.cellCount);
    }
    return counts;
}

void ConfigManager::setColumns(const QList<ColumnConfig>& newColumns)
{
    columns = newColumns;
}

void ConfigManager::updateColumn(int index, const QString& name, int cellCount, const QList<CellInfo>& cellInfos)
{
    if (index >= 0 && index < columns.size()) {
        columns[index].name = name;
        columns[index].cellCount = cellCount;
        columns[index].cells = cellInfos;
    }
}

void ConfigManager::createDefaultConfig()
{
    columns.clear();

    ColumnConfig col1;
    col1.name = "Колонка 1";
    col1.cellCount = 3;
    for (int i = 0; i < col1.cellCount; ++i) {
        col1.cells.append(CellInfo{QString("Содержимое %1-1").arg(i + 1)});
    }

    ColumnConfig col2;
    col2.name = "Колонка 2";
    col2.cellCount = 4;
    for (int i = 0; i < col2.cellCount; ++i) {
        col2.cells.append(CellInfo{QString("Содержимое %1-2").arg(i + 1)});
    }

    ColumnConfig col3;
    col3.name = "Колонка 3";
    col3.cellCount = 2;
    for (int i = 0; i < col3.cellCount; ++i) {
        col3.cells.append(CellInfo{QString("Содержимое %1-3").arg(i + 1)});
    }

    columns << col1 << col2 << col3;
}

ColumnConfig ConfigManager::columnFromJson(const QJsonObject& json)
{
    ColumnConfig column;

    column.name = json["name"].toString();
    column.cellCount = json["cellCount"].toInt();

    if (json.contains("cells") && json["cells"].isArray()) {
        QJsonArray cellsArray = json["cells"].toArray();
        for (const QJsonValue& cellValue : cellsArray) {
            if (cellValue.isObject()) {
                QJsonObject cellObj = cellValue.toObject();
                CellInfo cell;
                cell.content = cellObj["content"].toString();
                column.cells.append(cell);
            }
        }
    }

    // Если cells не указаны, создаем пустые
    while (column.cells.size() < column.cellCount) {
        column.cells.append(CellInfo{QString("Ячейка %1").arg(column.cells.size() + 1)});
    }

    return column;
}

QJsonObject ConfigManager::columnToJson(const ColumnConfig& column) const
{
    QJsonObject json;
    json["name"] = column.name;
    json["cellCount"] = column.cellCount;

    QJsonArray cellsArray;
    for (const CellInfo& cell : column.cells) {
        QJsonObject cellObj;
        cellObj["content"] = cell.content;
        cellsArray.append(cellObj);
    }
    json["cells"] = cellsArray;

    return json;
}
