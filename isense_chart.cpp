#include <QHBoxLayout>
#include "isense_chart.h"

isense_chart::isense_chart(QWidget *parent):QWidget(parent)
{
    setWindowTitle("chart");
    resize(800,600);

    QHBoxLayout *l = new QHBoxLayout;
    l->addWidget(&m_cv);
    setLayout(l);

    QChart *c = new QChart;
    c->setTitle("TEST CHART");
    m_cv.setChart(c);

    QValueAxis *ya = new QValueAxis;
    ya->setMax(1000);ya->setMin(-500);
    c->setAxisY(ya);
    QValueAxis *xa = new QValueAxis;
    xa->setMin(0);xa->setMax(1000);
    c->setAxisX(xa);

    QSplineSeries *s = new QSplineSeries;
    for(int i=0;i<1000;i++){
        s->append(i,i);
    }
    c->addSeries(s);
    s->setName(QString("LINE1"));

    setAttribute(Qt::WA_DeleteOnClose);
}

isense_chart::~isense_chart()
{

}

void isense_chart::set_series(QString name, QVector<QPointF> d)
{

}

void isense_chart::setTitle(QString title)
{
    setWindowTitle(title);
}

void isense_chart::addData(QString name, QPointF value)
{
    QSplineSeries *s = getSeries(name);
    if(s == nullptr){
        s = new QSplineSeries;
        s->setName(name);
        s->append(value);
        m_cv.chart()->addSeries(s);
    }
    else{
        s->append(value);
    }
}

void isense_chart::setData(QString name, QList<QPointF> values)
{
    QSplineSeries *s = getSeries(name);
    if(s == nullptr){
        s = new QSplineSeries;
        s->setName(name);
        s->append(values);
        m_cv.chart()->addSeries(s);
    }
    else{
        s->replace(values);
    }

}

QSplineSeries *isense_chart::getSeries(QString name)
{
    foreach(QSplineSeries *s,m_series){
        if(s->name() == name)
            return s;
    }
    return nullptr;
}
