#include "graphwidget.h"
#include <algorithm>

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
{
    series = new QLineSeries(this);

    chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();

    axisX = new QValueAxis;
    axisY = new QValueAxis;
    axisX->setTitleText("Время");
    axisY->setTitleText("Значение");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    setLayout(layout);
}

void GraphWidget::setData(const QVector<double> &data, const QString &cellName)
{
    series->clear();

    for (int i = 0; i < data.size(); ++i) {
        series->append(i, data[i]);
    }

    if (data.isEmpty()) {
        series->append(0, 0);
    }

    axisX->setRange(0, data.size() > 0 ? data.size() - 1 : 10);

    double minY = 0, maxY = 10;
    if (!data.isEmpty()) {
        minY = *std::min_element(data.begin(), data.end());
        maxY = *std::max_element(data.begin(), data.end());
        if (minY == maxY) maxY += 1;
    }
    axisY->setRange(minY, maxY);

    chart->setTitle(QStringLiteral("График: %1").arg(cellName));
    chartView->repaint();
}

