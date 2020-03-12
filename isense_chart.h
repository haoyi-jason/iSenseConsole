#ifndef ISENSE_CHART_H
#define ISENSE_CHART_H

#include <QObject>
#include <QtCharts>
#include <QChartView>
//#include <QWidget>
using namespace QtCharts;

class isense_chart : public QWidget
{
public:
    const QMap<QString,quint8> chartType_map =
            QMap<QString,quint8>({{"TREND",0x0},{"WAVEFORM",0x1},{"FREQ",0x2}});

    isense_chart(QWidget *parent = nullptr);
    ~isense_chart();
    void set_series(QString name, QVector<QPointF> d);
    void setTitle(QString title);
    QSplineSeries *getSeries(QString name);

    int chartType() const{return m_chartType;}
    void setChartType(int v){m_chartType = v;}

public slots:
    void addData(QString name, QPointF value);
    void setData(QString name, QList<QPointF> values);


private:
    QChartView m_cv;
    QList<QSplineSeries*> m_series;
    int m_chartType;
};

#endif // ISENSE_CHART_H
