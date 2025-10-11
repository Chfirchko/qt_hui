#pragma once
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QWidget>
#include <QtMath>

class TemperatureGauge : public QWidget {
  Q_OBJECT
public:
  explicit TemperatureGauge(QWidget *parent = nullptr)
      : QWidget(parent), temperature(0) {
    setMinimumSize(120, 120);
  }

  void setTemperature(int temp) {
    temperature = qBound(0, temp, 120); // ограничиваем до 0-120
    update();
  }

protected:
  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    p.translate(width() / 2, height() / 2);
    p.scale(side / 200.0, side / 200.0);

    // фон
    p.setBrush(QColor(200, 200, 200)); // серый фон
    p.setPen(Qt::NoPen);
    p.drawEllipse(-90, -90, 180, 180);

    double startAngle = 225.0; // где начинается 0
    double angleSpan = 270.0;  // полная ширина шкалы


    double needleAngle = startAngle - (temperature / 120.0) * angleSpan;
    // сегменты шкалы
    struct Segment {
      double from;
      double to;
      QColor color;
    };
    Segment segments[] = {
        {0, 20, QColor(0, 0, 255)},    // синий
        {20, 40, QColor(0, 255, 0)},   // зеленый
        {40, 60, QColor(255, 255, 0)}, // желтый
        {60, 90, QColor(255, 0, 0)},   // красный
        {90, 120, QColor(0, 0, 0)}     // черный
    };

    for (const Segment &seg : segments) {
      double segStartAngle = startAngle - (seg.from / 120.0) * angleSpan;
      double segSpan = (seg.to - seg.from) / 120.0 * angleSpan;

      p.setPen(QPen(seg.color, 8, Qt::SolidLine, Qt::FlatCap));
      p.drawArc(-70, -70, 140, 140, segStartAngle * 16, -segSpan * 16);
    }

    // стрелка
    p.save();
    p.rotate(-needleAngle + 90 ); // поворот относительно вертикали вверх
    QPoint needle[3] = {QPoint(-3, 0), QPoint(0, -60), QPoint(3, 0)};
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    p.drawPolygon(needle, 3);
    p.restore();

    // текст температуры
    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 16, QFont::Bold));
    p.drawText(-25, 50, QString::number(temperature) + "°C");
  }

private:
  int temperature;
};
