#include "snchartview.h"

snChartView::snChartView(QWidget *parent):
    QChartView(parent){}

snChartView::snChartView(QChart *chart,QWidget *parent):
    QChartView (chart,parent),
    m_isTouching(false)
{
    setRubberBand(QChartView::RectangleRubberBand);
}

bool snChartView::viewportEvent(QEvent *event)
{
    if(event->type() == QEvent::TouchBegin){
        m_isTouching = true;
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void snChartView::mousePressEvent(QMouseEvent *event)
{
    if(m_isTouching)
        return;
    if(event->button() == Qt::MiddleButton){
        m_lastMousePos = event->pos();
        event->accept();
    }
    QChartView::mousePressEvent(event);
}

void snChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isTouching)
        return;
    if(event->buttons() & Qt::MiddleButton){
        auto dPos = event->pos() - m_lastMousePos;
        chart()->scroll(-dPos.x(),dPos.y());
        m_lastMousePos = event->pos();
        event->accept();
    }
    QChartView::mouseMoveEvent(event);
}

void snChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_isTouching)
        m_isTouching = false;

    chart()->setAnimationOptions(QChart::NoAnimation);

    QChartView::mouseReleaseEvent(event);
}
void snChartView::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()){
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_multiply:
        chart()->zoomReset();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10,0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10,0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0,10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0,-10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}
