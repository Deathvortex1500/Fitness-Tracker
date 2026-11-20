#include "chartwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMap>
#include <algorithm>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) { setMinimumHeight(260); }

void ChartWidget::setSeries(const QVector<double> &current, const QVector<double> &previous,
                            const QString &title, const QString &unit) {
    currentValues = current; previousValues = previous; chartTitle = title; chartUnit = unit; update();
}

void ChartWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    static const QMap<QString,QColor> colors={{"Calories",QColor("#ffd166")},{"Protein",QColor("#52ffa8")},{"Carbs",QColor("#62c6ff")},{"Fat",QColor("#d99cff")},{"Water",QColor("#54efff")},{"Weight",QColor("#ff7eb6")}};
    QColor accent("#45d9ff");
    for (auto it=colors.cbegin();it!=colors.cend();++it) if (chartTitle.startsWith(it.key())) { accent=it.value(); break; }
    p.fillRect(rect(), QColor("#000"));
    p.setPen(accent); p.setFont(QFont("Sans", 12, QFont::DemiBold));
    p.drawText(20, 28, chartTitle);
    const QRectF area(48, 50, width()-70, height()-90);
    p.setPen(QPen(QColor("#242424"), 1));
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
    draw(previousValues, QColor("#777")); draw(currentValues, accent);
    p.setFont(QFont("Sans", 9)); p.setPen(QColor("#b8b8b8"));
    p.drawText(area.left(), height()-14, "Previous period");
    p.setPen(accent); p.drawText(area.left()+120, height()-14, "Current period");
    p.setPen(accent); p.drawText(area.right()-80, 28, QString::number(maximum, 'f', 0)+" "+chartUnit);
}
