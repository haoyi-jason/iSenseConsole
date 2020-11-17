#include "chartview2.h"
#include "ui_chartview2.h"
#include <QDebug>
chartView2::chartView2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chartView2)
{
    ui->setupUi(this);
    mModel = new seriesModel;
    ui->tableView->setModel(mModel);

    m_recordSize = 1000;

    m_axisModel = new axisModel;
    ui->tableView_2->setModel(m_axisModel);
    ui->tableView_2->setItemDelegateForColumn(4,new SliderDelegate(this));
    ui->tableView_2->setSelectionBehavior(QAbstractItemView::SelectRows);

    QChart *c = new QChart;
//    QValueAxis *xa = new QValueAxis;
//    xa->setMax(2000);
//    xa->setMin(0);
//    QValueAxis *ya = new QValueAxis;
//    ya->setMax(32768);
//    ya->setMin(-32768);
//    ya->setMax(100);
//    ya->setMin(-100);
    c->addAxis(m_axisModel->getItem(0),Qt::AlignBottom);
    c->addAxis(m_axisModel->getItem(1),Qt::AlignBottom);
    c->addAxis(m_axisModel->getItem(2),Qt::AlignLeft);
    c->addAxis(m_axisModel->getItem(3),Qt::AlignRight);
    connect(m_axisModel,&axisModel::dataChanged,this,&chartView2::handleAxisModelChanged);
    ui->graphicsView->setRubberBand(QChartView::HorizontalRubberBand);

    ui->graphicsView->setChart(c);

    ui->graphicsView->setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(ui->graphicsView,&QChartView::customContextMenuRequested,this,&chartView2::showChartContextMenu);
}

chartView2::~chartView2()
{
    delete ui;
}


void chartView2::showChartContextMenu(const QPoint &p)
{
    QMenu contextMenu(tr("Context Menu"),this);
    QAction *act = new QAction("Zoom Horizontal",this);
    act->setProperty("ID",0);
    connect(act,&QAction::triggered,this,&chartView2::handleContextClick);
    act = new QAction("Zoom Vertical",this);
    act->setProperty("ID",1);
    connect(act,&QAction::triggered,this,&chartView2::handleContextClick);
    act = new QAction("Zoom Rectangle",this);
    act->setProperty("ID",2);
    connect(act,&QAction::triggered,this,&chartView2::handleContextClick);

}

void chartView2::handleContextClick()
{
    QAction *act = (QAction*)sender();
    int id = act->property("ID").toInt();
    switch(id){
    case 0:
        ui->graphicsView->setRubberBand(QChartView::HorizontalRubberBand);
        break;
    case 1:
        ui->graphicsView->setRubberBand(QChartView::VerticalRubberBand);
        break;
    case 2:
        ui->graphicsView->setRubberBand(QChartView::RectangleRubberBand);
        break;
    }
}

void chartView2::clearSeries()
{
    QChart *c = ui->graphicsView->chart();
    if(c == nullptr) return;
    c->removeAllSeries();
    mModel->removeRows(0,mModel->rowCount());
}

void chartView2::clearData()
{
//    QChart *c = ui->graphicsView->chart();
//    foreach(QAbstractSeries *s,c->series()){
//        ((QLineSeries*)s)->clear();
//    }
    mModel->clearSeriesData();
}

void chartView2::addValueAxis(Qt::Alignment align, QString title, float min, float max)
{
    QChart *c = ui->graphicsView->chart();
    if(c == nullptr) return;
    QValueAxis *a = new QValueAxis;
    a->setTitleText(title);
    a->setMax(max);
    a->setMin(min);
    c->addAxis(a,align);
}


void chartView2::addDateTimeAxis(Qt::Alignment align, QString title, QDateTime min, QDateTime max)
{
    QChart *c = ui->graphicsView->chart();
    if(c == nullptr) return;
    QDateTimeAxis *a = new QDateTimeAxis;
    a->setTitleText(title);
    c->addAxis(a,align);
}

QLineSeries *chartView2::addSeries(QString title, int axis1, int axis2)
{
    qDebug()<<Q_FUNC_INFO<<title;
    if(mModel->getItem(title)!=nullptr) return nullptr;
    QChart *c = ui->graphicsView->chart();
//    qDebug()<<0;
//    QAbstractAxis *xa = (QAbstractAxis*)(c->axes(Qt::Horizontal).back());
//    QAbstractAxis *ya = (QAbstractAxis*)(c->axes(Qt::Vertical).back());
    if(m_axisModel == nullptr){
//        qDebug()<<"Axis model valid fail";
        return nullptr;
    }
    QAbstractAxis *xa = m_axisModel->getItem(axis1);
    QAbstractAxis *ya = m_axisModel->getItem(axis2);
//    if(xa == nullptr){
//        qDebug()<<"XA fail";
//        return nullptr;
//    }
//    if(ya == nullptr) return nullptr;
    mModel->insertRows(mModel->rowCount(),1);

    QLineSeries *s = mModel->getItem(mModel->rowCount()-1);
    if(s == nullptr) return nullptr;
    c->addSeries(s);
    s->setName(title);
//    qDebug()<<1;
    s->attachAxis(xa);
 //   qDebug()<<2;
    s->attachAxis(ya);
   // qDebug()<<3;
    m_series.append(s);
    return s;
}

QLineSeries *chartView2::addTimeSeries(QString title)
{
    qDebug()<<Q_FUNC_INFO<<title;
    if(mModel->getItem(title)!=nullptr) return nullptr;
    QChart *c = ui->graphicsView->chart();
    QAbstractAxis *xa = (QAbstractAxis*)(c->axes(Qt::Horizontal).back());
    QAbstractAxis *ya = (QAbstractAxis*)(c->axes(Qt::Vertical).back());

    mModel->insertRows(mModel->rowCount(),1);

    QLineSeries *s = mModel->getItem(mModel->rowCount()-1);
    if(s == nullptr) return nullptr;
    c->addSeries(s);
    s->setName(title);
    s->attachAxis(m_axisModel->getItem(0));
    s->attachAxis(m_axisModel->getItem(1));
    m_series.append(s);
    return s;
}
QLineSeries *chartView2::findSeries(QString title)
{
    return mModel->getItem(title);
}

void chartView2::setSeriesData(QString name, QVector<QPointF> data)
{
    //qDebug()<<Q_FUNC_INFO<<name;
    QLineSeries *s = findSeries(name);
    if(s == nullptr){
        s = addSeries(name);
        if(s == nullptr) return;
    }

    //qDebug()<<"Replace data";
    s->replace(data);
}

void chartView2::appendSeriesData(QStringList name, QVector<QPointF> data)
{
    qDebug()<<Q_FUNC_INFO<<name;
    QLineSeries *ss;
    qint64 last = data[0].x();
    for(int i=0;i<name.size();i++){
        if((ss=findSeries(name.at(i))) == nullptr){
            qDebug()<<"Add series";
            ss = addTimeSeries(name.at(i));
            if(ss != nullptr){
                ss->append(data.at(i));
            }
        }else{
            ss->append(data.at(i));
        }
    }

    qDebug()<<"Axis Scale check";
    // todo: move x-axis
    QDateTimeAxis *x = (QDateTimeAxis*)m_axisModel->getItem(1);

    if(last >= x->max().toMSecsSinceEpoch()){
        QChart *c = ui->graphicsView->chart();
        qreal dw = c->plotArea().width()*0.2;
        c->scroll(dw,0);
    }

    ui->tableView->viewport()->update();
}

void chartView2::on_pbHBand_clicked()
{
    ui->graphicsView->setRubberBand(QChartView::HorizontalRubberBand);

}

void chartView2::on_pbVBand_clicked()
{
    ui->graphicsView->setRubberBand(QChartView::VerticalRubberBand);
}

void chartView2::on_pbRBand_clicked()
{
    ui->graphicsView->setRubberBand(QChartView::RectangleRubberBand);
}

void chartView2::on_tableView_clicked(const QModelIndex &index)
{
    if(index.column() == 1){
        seriesModel *m = (seriesModel*)index.model();
        QColor color = QColorDialog::getColor();
        qDebug()<<"Selected color:"<<color;
        m->setData(index,QVariant(color),Qt::BackgroundRole);
    }
}

void chartView2::setCaption(QString cap)
{
    this->setWindowTitle(cap);
}

void chartView2::on_pbRstView_clicked()
{
    ui->graphicsView->chart()->zoomReset();
}

void chartView2::setAxisScale(int id, float min, float max)
{
    if((id == 0) || (id == 2) ||(id==3)){
        QValueAxis *a = (QValueAxis*)m_axisModel->getItem(id);
        a->setRange(min,max);
    }
}

void chartView2::setAxisScale(int id, QDateTime min, QDateTime max)
{
    if(id == 1){
        QDateTimeAxis *a = (QDateTimeAxis*)m_axisModel->getItem(id);
        a->setRange(min,max);
    }
}

void chartView2::on_pbCapture_clicked()
{
    QString m_logPath = QDir::currentPath()+ "/capture";
    if(!QDir(m_logPath).exists()){
        QDir().mkdir(m_logPath);
    }
    QPixmap p = ui->graphicsView->grab();
    QImage image = p.toImage();
    image.save(QString("%1/%2-%3_WAVE.png").arg(m_logPath).arg(windowTitle()).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss")));
}

void chartView2::setAxisBuffer(int id,int sz)
{
    m_axisModel->setSize(id,sz);
}

void chartView2::handleAxisModelChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)//, QVector<int> &roles)
{
    qDebug()<<Q_FUNC_INFO<<topLeft.row();
    QValueAxis *a = (QValueAxis*)m_axisModel->getItem(topLeft.row());
    if(a){
        emit requestData(a->min(),a->max());
    }
}
