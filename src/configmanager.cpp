#include "configmanager.h"
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QCoreApplication>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{
    // Пытаемся найти конфиг в разных местах
    QStringList possiblePaths = {
        "/home/asswecan/Download/hui/build/config.json",
        "../config.json", 
        "../../config.json",
        QCoreApplication::applicationDirPath() + "/config.json"
    };
    
    for (const QString& path : possiblePaths) {
        if (QFile::exists(path)) {
            qDebug() << "Найден конфиг по пути:" << path;
            if (loadConfig(path)) {
                return;
            }
        }
    }
    
    qWarning() << "Конфиг не найден, используется конфиг по умолчанию";
    createDefaultConfig();
}

bool ConfigManager::configExists() const
{
    return QFile::exists(configPath);
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

    qDebug() << "Содержимое файла" << filename << ":" << data;

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

    configPath = filename;
    qDebug() << "Конфигурация загружена. Колонок:" << columns.size();
    
    // Отладочный вывод
    for (int i = 0; i < columns.size(); ++i) {
        qDebug() << "Колонка" << i << ":" << columns[i].name;
        for (int j = 0; j < columns[i].cells.size(); ++j) {
            qDebug() << "  Ячейка" << j << ":" << columns[i].cells[j].content 
                     << "Значение:" << columns[i].cells[j].value;
            for (int k = 0; k < columns[i].cells[j].subCells.size(); ++k) {
                qDebug() << "    Подъячейка" << k << ":" << columns[i].cells[j].subCells[k].content
                         << "Значение:" << columns[i].cells[j].subCells[k].value;
            }
        }
    }
    
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

bool ConfigManager::updateCellValue(int columnIndex, int cellIndex, const QString& value)
{
    if (columnIndex >= 0 && columnIndex < columns.size() &&
        cellIndex >= 0 && cellIndex < columns[columnIndex].cells.size()) {
        columns[columnIndex].cells[cellIndex].value = value;
        return true;
    }
    return false;
}

bool ConfigManager::updateSubCellValue(int columnIndex, int cellIndex, int subCellIndex, const QString& value)
{
    if (columnIndex >= 0 && columnIndex < columns.size() &&
        cellIndex >= 0 && cellIndex < columns[columnIndex].cells.size() &&
        subCellIndex >= 0 && subCellIndex < columns[columnIndex].cells[cellIndex].subCells.size()) {
        columns[columnIndex].cells[cellIndex].subCells[subCellIndex].value = value;
        return true;
    }
    return false;
}

QString ConfigManager::getCellValue(int columnIndex, int cellIndex) const
{
    if (columnIndex >= 0 && columnIndex < columns.size() &&
        cellIndex >= 0 && cellIndex < columns[columnIndex].cells.size()) {
        return columns[columnIndex].cells[cellIndex].value;
    }
    return QString();
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
    col1.name = "ЦОС";
    col1.cellCount = 6;
    for (int i = 0; i < col1.cellCount; ++i) {
        CellInfo cell;
        cell.content = QString("Содержимое %1-1").arg(i + 1);
        // Добавляем подъячейки для демонстрации
        if (i == 0) {
            CellInfo subCell1;
            subCell1.content = "Подъячейка 1-1";
            subCell1.value = "0.0 В";
            subCell1.unit = "В";
            CellInfo subCell2;
            subCell2.content = "Подъячейка 1-2";
            subCell2.value = "0.0 В";
            subCell2.unit = "В";
            cell.subCells << subCell1 << subCell2;
        }
        col1.cells.append(cell);
    }

    ColumnConfig col2;
    col2.name = "ВИП";
    col2.cellCount = 5;
    for (int i = 0; i < col2.cellCount; ++i) {
        col2.cells.append(CellInfo{QString("Содержимое %1-2").arg(i + 1)});
    }

    ColumnConfig col3;
    col3.name = "ПП";
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
                CellInfo cell = cellFromJson(cellValue.toObject());
                column.cells.append(cell);
            }
        }
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
        cellsArray.append(cellToJson(cell));
    }
    json["cells"] = cellsArray;

    return json;
}

CellInfo ConfigManager::cellFromJson(const QJsonObject& json)
{
    CellInfo cell;
    cell.content = json["content"].toString();

    // Правильная загрузка значения value (число или строка)
    if (json.contains("value")) {
        QJsonValue val = json["value"];
        if (val.isString()) {
            cell.value = val.toString();
        } else if (val.isDouble()) {
            cell.value = QString::number(val.toDouble(), 'f', 2); // 2 знака после запятой
        } else {
            cell.value = "";
        }
    } else {
        cell.value = "";
    }

    // Загрузка unit
    if (json.contains("unit")) {
        cell.unit = json["unit"].toString();
    } else {
        cell.unit = "";
    }

    qDebug() << "Загружена ячейка:" << cell.content << "value:" << cell.value << "unit:" << cell.unit;

    // Загружаем подъячейки
    if (json.contains("subCells") && json["subCells"].isArray()) {
        QJsonArray subCellsArray = json["subCells"].toArray();
        qDebug() << "  Найдено подъячеек:" << subCellsArray.size();

        for (const QJsonValue& subCellValue : subCellsArray) {
            if (subCellValue.isObject()) {
                QJsonObject subCellObj = subCellValue.toObject();
                CellInfo subCell;
                subCell.content = subCellObj["content"].toString();

                // Правильная загрузка value для подъячейки
                if (subCellObj.contains("value")) {
                    QJsonValue val = subCellObj["value"];
                    if (val.isString()) {
                        subCell.value = val.toString();
                    } else if (val.isDouble()) {
                        subCell.value = QString::number(val.toDouble(), 'f', 2);
                    } else {
                        subCell.value = "";
                    }
                } else {
                    subCell.value = "";
                }

                // Загрузка unit
                if (subCellObj.contains("unit")) {
                    subCell.unit = subCellObj["unit"].toString();
                } else {
                    subCell.unit = "";
                }

                qDebug() << "  Загружена подъячейка:" << subCell.content << "value:" << subCell.value << "unit:" << subCell.unit;
                cell.subCells.append(subCell);
            }
        }
    }

    return cell;
}

QJsonObject ConfigManager::cellToJson(const CellInfo& cell) const
{
    QJsonObject json;
    json["content"] = cell.content;
    
    if (!cell.value.isEmpty()) {
        json["value"] = cell.value;
    }
    
    if (!cell.unit.isEmpty()) {
        json["unit"] = cell.unit;
    }

    QJsonArray subCellsArray;
    for (const CellInfo& subCell : cell.subCells) {
        subCellsArray.append(cellToJson(subCell));
    }
    
    if (!subCellsArray.isEmpty()) {
        json["subCells"] = subCellsArray;
    }

    return json;
}
