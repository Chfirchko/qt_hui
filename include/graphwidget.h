#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QChart>
#include <QVBoxLayout>
#include <QString>

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = nullptr);

    // Обновление данных графика
    void setData(const QVector<double> &data, const QString &cellName);

private:
    QChart *chart;
    QLineSeries *series;
    QChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QVBoxLayout *layout;
};

#endif // GRAPHWIDGET_H

