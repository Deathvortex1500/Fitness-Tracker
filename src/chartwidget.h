#pragma once
#include <QWidget>
#include <QVector>

// A dependency-free line chart used by the analytics page.
class ChartWidget : public QWidget {
public:
    explicit ChartWidget(QWidget *parent = nullptr);
    void setSeries(const QVector<double> &current, const QVector<double> &previous,
                   const QString &title, const QString &unit);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QVector<double> currentValues, previousValues;
    QString chartTitle, chartUnit;
};
