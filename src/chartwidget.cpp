#include "chartwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) { setMinimumHeight(260); }

void ChartWidget::setSeries(const QVector<double> &current, const QVector<double> &previous,
                            const QString &title, const QString &unit) {
    currentValues = current; previousValues = previous; chartTitle = title; chartUnit = unit; update();
}

void ChartWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#17202b"));
    p.setPen(QColor("#f2f5f7")); p.setFont(QFont("Sans", 12, QFont::DemiBold));
    p.drawText(20, 28, chartTitle);
    const QRectF area(48, 50, width()-70, height()-90);
    p.setPen(QPen(QColor("#344150"), 1));
    for (int i=0;i<5;i++) p.drawLine(area.left(), area.top()+area.height()*i/4, area.right(), area.top()+area.height()*i/4);
    double maximum = 1;
    for (double v : currentValues) maximum = std::max(maximum, v);
    for (double v : previousValues) maximum = std::max(maximum, v);
    auto draw = [&](const QVector<double> &values, QColor color) {
        if (values.isEmpty()) return;
        QPainterPath path;
        for (int i=0;i<values.size();++i) {
            const double x = area.left() + area.width() * i / std::max<qsizetype>(1, values.size()-1);
            const double y = area.bottom() - area.height() * values[i] / maximum;
            i ? path.lineTo(x,y) : path.moveTo(x,y);
        }
        p.setPen(QPen(color, 3)); p.drawPath(path);
    };
    draw(previousValues, QColor("#667788")); draw(currentValues, QColor("#45d6a7"));
    p.setFont(QFont("Sans", 9)); p.setPen(QColor("#9dabb8"));
    p.drawText(area.left(), height()-14, "Previous period");
    p.setPen(QColor("#45d6a7")); p.drawText(area.left()+120, height()-14, "Current period");
    p.setPen(QColor("#9dabb8")); p.drawText(area.right()-80, 28, QString::number(maximum, 'f', 0)+" "+chartUnit);
}
