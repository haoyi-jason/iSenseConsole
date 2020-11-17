#ifndef SNCHARTVIEW_H
#define SNCHARTVIEW_H

#include <QObject>
#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

class snChartView : public QChartView
{
public:
    snChartView(QWidget *parent=nullptr);
    snChartView(QChart *chart,QWidget *parent=nullptr);

protected:
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    bool m_isTouching;
    QPointF m_lastMousePos;
};

#endif // SNCHARTVIEW_H
