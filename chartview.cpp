#include "chartview.h"
#include "ui_chartview.h"
#include <QSizePolicy>
chartView::chartView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chartView)
{
    ui->setupUi(this);

    setWindowTitle("Chart View");

    ui->graphicsView->setChart(new QChart);
    ui->graphicsView_2->setChart(new QChart);


    m_chartType = -1;

    m_dataModel = new vnodeDataModel;
    m_dataModel->insertRows(0,1,QModelIndex());
    ui->tableView->setModel(m_dataModel);
    for(int i=0;i<6;i++){
        QPushButton *btn = new QPushButton;
        btn->setProperty("ID",i);
        btn->setCheckable(true);
        btn->setChecked(true);
        btn->setText("SHOW");
        connect(btn,&QPushButton::clicked,m_dataModel,&vnodeDataModel::setVisible);
        ui->tableView->setIndexWidget(m_dataModel->index(i,2),btn);

    }
    connect(m_dataModel,&vnodeDataModel::showCurve,this,&chartView::showTrend);
//    QVector<float> a;
//    a << 0;
    //m_seriesData << a<<a<<a<<a<<a<<a;

    nofXPoints = 1000;

    m_tmr = new QTimer;
    connect(m_tmr,&QTimer::timeout,this,&chartView::handle_chart_update);

    ui->graphicsView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
   // m_workThread = new QThread;
    //moveToThread(m_workThread);

    paramModel *m = new paramModel;
    //m->setChart(m_charts[1]);
    ui->tableView_2->setModel(m);

    m = new paramModel;
    m->setValue(0,0);
    m->setValue(1,256);
    m->setValue(2,0);
    m->setValue(3,100);

    //m->setChart(m_charts[2]);
    ui->tableView_3->setModel(m);

    qDebug()<<ui->graphicsView->renderHints();
    ui->graphicsView->setRenderHint(QPainter::Antialiasing,false);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform,false);
    ui->graphicsView_2->setRenderHint(QPainter::Antialiasing);

    //ui->page->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

chartView::~chartView()
{
    delete ui;
    foreach(QChart *c, m_charts){
        delete c;
    }

    delete m_dataModel;
}

void chartView::setTitle(QString title)
{
    setWindowTitle(title);
}

void chartView::addData(QString name, QPointF value)
{
    QLineSeries *s = getSeries(name);

    if(s != nullptr){
        s->append(value);
    }

//    if(s == nullptr){
//        s = new QLineSeries;
//        s->setName(name);
//        s->append(value);
//        m_cv->chart()->addSeries(s);
//    }
//    else{
//        s->append(value);
//    }
}

void chartView::setData(QString name, QList<QPointF> values)
{
    QLineSeries *s = getSeries(name);
    if(s == nullptr){
        s = new QLineSeries;
        s->setName(name);
        s->append(values);
        m_cv->chart()->addSeries(s);
    }
    else{
        s->replace(values);
    }

}

QLineSeries *chartView::getSeries(QString name)
{
    foreach(QLineSeries *s,m_series){
        if(s->name() == name)
            return s;
    }
    return nullptr;
}
void chartView::setChartType(int id)
{
    setChartType(chartType_map.key(id));
    qDebug()<<Q_FUNC_INFO<<id<<chartType_map.key(id);
}
void chartView::setChartType(QString name)
{
    int type = chartType_map.value(name);
    qDebug()<<Q_FUNC_INFO<<name<<type;
    if(type != m_chartType ){
        m_chartType = type;
        ui->stackedWidget_2->setCurrentIndex(m_chartType);
        QDateTime n = QDateTime::currentDateTime();
        QValueAxis *ya;
        QValueAxis *x2;
        QDateTimeAxis *xa;
        switch(m_chartType){
        case 0:{
            //ui->graphicsView->setVisible(false);
           //ui->graphicsView_2->setVisible(false);
            QChart *c = ui->graphicsView->chart();
            c->removeAllSeries();
            if(c->axes(Qt::Horizontal).size() > 0)
                c->removeAxis(c->axes(Qt::Horizontal).back());
            if(c->axes(Qt::Vertical).size() > 0)
                c->removeAxis(c->axes(Qt::Vertical).back());
            c->setTitle("Waveform");
            QValueAxis *xa = new QValueAxis;
            QValueAxis *ya = new QValueAxis;
            c->addAxis(xa,Qt::AlignBottom);
            c->addAxis(ya,Qt::AlignLeft);
            paramModel *m = (paramModel*)ui->tableView_2->model();
            m->setChart(c);
            for(int i=0;i<3;i++){
                QLineSeries *s = new QLineSeries;
                c->addSeries(s);
                s->setName(seriesNameXYZ_map.key(i));
                s->setUseOpenGL(true);
                s->attachAxis(xa);
                s->attachAxis(ya);
            }
            ///ui->graphicsView->setChart(c);

            c = ui->graphicsView_2->chart();
            c->removeAllSeries();
            if(c->axes(Qt::Horizontal).size() > 0)
                c->removeAxis(c->axes(Qt::Horizontal).back());
            if(c->axes(Qt::Vertical).size() > 0)
                c->removeAxis(c->axes(Qt::Vertical).back());
            c->setTitle("FFT");
            xa = new QValueAxis;
            ya = new QValueAxis;
            c->addAxis(xa, Qt::AlignBottom);
            c->addAxis(ya,Qt::AlignLeft);
            m = (paramModel*)ui->tableView_3->model();
            m->setChart(c);
            for(int i=0;i<3;i++){
                QLineSeries *s = new QLineSeries;
                c->addSeries(s);
                s->setName(seriesNameXYZ_map.key(i));
                s->setUseOpenGL(true);
                s->attachAxis(xa);
                s->attachAxis(ya);
            }

            m_tmr->start(100);
            ///ui->graphicsView_2->setChart(c);
            ui->graphicsView_2->setVisible(true);
            //ui->graphicsView->setVisible(true);
//            ui->page->layout()->setSizeConstraint(QLayout::SetFixedSize);
//            qDebug()<<ui->page->layout()->count();
            //ui->graphicsView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
           // ui->graphicsView_2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
            //ui->graphicsView_2->adjustSize();
        }break;
        case 1:{
           // ui->graphicsView->setVisible(false);
            ui->graphicsView_2->setVisible(false);
            QChart *c = ui->graphicsView->chart();
            c->removeAllSeries();
            if(c->axes(Qt::Horizontal).size() > 0)
                c->removeAxis(c->axes(Qt::Horizontal).back());
            if(c->axes(Qt::Vertical).size() > 0)
                c->removeAxis(c->axes(Qt::Vertical).back());
            c->setTitle("TREND");
            QDateTimeAxis *xa = new QDateTimeAxis;
            xa->setRange(QDateTime::currentDateTime(),QDateTime::currentDateTime().addSecs(100));
            QValueAxis *ya = new QValueAxis;
            c->addAxis(xa,Qt::AlignBottom);
            c->addAxis(ya,Qt::AlignLeft);
            for(int i=0; i< 6;i++){
                QLineSeries *s = new QLineSeries;
                c->addSeries(s);
                s->setName(seriesNameRMS_map.key(i));
                s->setUseOpenGL(true);
                s->attachAxis(xa);
                s->attachAxis(ya);
            }
            //ui->graphicsView->setChart(c);
            //ui->graphicsView->setVisible(true);
            ui->graphicsView->viewport()->repaint();
            m_tmr->stop();
            qDebug()<<ui->page->layout()->count();
            vnodeDataModel *m = (vnodeDataModel*)ui->tableView->model();
            m->setChart(c);
        }break;
        case 2:{
            QChart *c = ui->graphicsView->chart();
            c->removeAllSeries();
            if(c->axes(Qt::Horizontal).size() > 0)
                c->removeAxis(c->axes(Qt::Horizontal).back());
            if(c->axes(Qt::Vertical).size() > 0)
                c->removeAxis(c->axes(Qt::Vertical).back());
            c->setTitle("Waveform");
            QValueAxis *xa = new QValueAxis;
            QValueAxis *ya = new QValueAxis;
            c->addAxis(xa,Qt::AlignBottom);
            c->addAxis(ya,Qt::AlignLeft);
            paramModel *m = (paramModel*)ui->tableView_2->model();
            m->setChart(c);
            for(int i=0;i<6;i++){
                QLineSeries *s = new QLineSeries;
                c->addSeries(s);
                s->setName(seriesNameXYZ_map.key(i));
                s->setUseOpenGL(true);
                s->attachAxis(xa);
                s->attachAxis(ya);
            }

            c = ui->graphicsView_2->chart();
            c->removeAllSeries();
            if(c->axes(Qt::Horizontal).size() > 0)
                c->removeAxis(c->axes(Qt::Horizontal).back());
            if(c->axes(Qt::Vertical).size() > 0)
                c->removeAxis(c->axes(Qt::Vertical).back());
            c->setTitle("FFT");
            xa = new QValueAxis;
            ya = new QValueAxis;
            c->addAxis(xa, Qt::AlignBottom);
            c->addAxis(ya,Qt::AlignLeft);
            m = (paramModel*)ui->tableView_3->model();
            m->setChart(c);
            for(int i=0;i<3;i++){
                QLineSeries *s = new QLineSeries;
                c->addSeries(s);
                s->setName(seriesNameXYZ_map.key(i));
                s->setUseOpenGL(true);
                s->attachAxis(xa);
                s->attachAxis(ya);
            }

            m_tmr->start(100);
            ///ui->graphicsView_2->setChart(c);
            ui->graphicsView_2->setVisible(true);
        }break;
        }
    }
}

void chartView::addRecord(QList<float> v)
{
    qDebug()<<Q_FUNC_INFO<<v;
//    QDateTime dt = QDateTime::currentDateTime();
    quint64 epoch = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QModelIndex index;
    //QList<QLineSeries*> series =qobject_cast<QList<QLineSeries*>>(ui->graphicsView->chart()->series());
    QChart *c = ui->graphicsView->chart();
    QLineSeries *s;
    QDateTimeAxis *dx = (QDateTimeAxis*)c->axes(Qt::Horizontal).back();
    for(int i=0;i<6;i++){
        //qDebug()<<m_series[i]->count();
        float fv = v.at(i)*9.81;
        QPointF pt = QPointF(epoch,fv);
        if(i < c->series().count()){
            s = (QLineSeries*)c->series().at(i);
            s->append(epoch,fv);
        }
        dataModel *d = m_dataModel->getItem(i);
        if(d != nullptr){
            d->value = fv;
        }
        quint64 cepoch = dx->max().toMSecsSinceEpoch();
        if(cepoch > epoch){
            if((cepoch - epoch) < 10000)
                dx->setRange(dx->min().addSecs(20),dx->max().addSecs(20));
        }else{
            dx->setRange(QDateTime::currentDateTime().addSecs(-80),QDateTime::currentDateTime().addSecs(20));
        }
    }
//    QDateTimeAxis *a = (QDateTimeAxis*)m_charts[0]->axisX();
//    a->setRange(m_series[0]->points().last().x()
    ui->tableView->viewport()->update();
    ui->graphicsView->viewport()->update();
    qDebug()<<Q_FUNC_INFO<<"out";
}

void chartView::updateSeries(QVector<float> res)
{
    qDebug()<<Q_FUNC_INFO;
    //QElapsedTimer timer;
   // timer.start();
    m_mutex.lock();
    for(int i=0;i<res.size();i++){
        m_seriesData[i%3].append(res[i]);
    }
    for(int i=0;i<6;i++){
        //qDebug()<<"A Series size:"<<m_seriesData[i].size();
        if(m_seriesData[i].size() > nofXPoints){
            //qDebug()<<"Remove";
            m_seriesData[i].remove(0,m_seriesData[i].size()-nofXPoints);
        }
         //qDebug()<<"B Series size:"<<m_seriesData[i].size();
    }
//    foreach(QVector<float> v,m_seriesData){
//        if(v.size() > nofXPoints)
//            v.remove(v.size()-(nofXPoints-200));
//    }
    m_mutex.unlock();
    //qDebug()<<QString("Elapsed %1 ms").arg(timer.elapsed());
}

void chartView::updateWave(QVector<float>x, QVector<float> y, QVector<float> z)
{
    //qDebug()<<Q_FUNC_INFO;
    m_seriesData[0]=x;
    m_seriesData[1]=y;
    m_seriesData[2]=z;

}

void chartView::updateGYRO(QVector<float>x, QVector<float> y, QVector<float> z)
{
    //qDebug()<<Q_FUNC_INFO;
    m_seriesData[3]=x;
    m_seriesData[4]=y;
    m_seriesData[5]=z;

}

void chartView::updateFFT(QVector<float>x, QVector<float> y, QVector<float> z)
{

    m_seriesFFTData[0]=x;
    m_seriesFFTData[1]=y;
    m_seriesFFTData[2]=z;
}
void chartView::addStream(QByteArray b)
{
    QDataStream ds(&b,QIODevice::ReadOnly);
    //ds.setByteOrder(QDataStream::LittleEndian);
    //ds.setFloatingPointPrecision(QDataStream::SinglePrecision);


    int nofRecord = b.size()/4/3;
    qDebug()<<Q_FUNC_INFO<<" Records:"<<nofRecord;
    //float fv;
    qint32 v;
    for(int i=0;i<nofRecord;i++){
        for(int j=0;j<3;j++){
            ds >> v;
            m_seriesData[j].append((float)v*0.000038259);
        }
    }

//    foreach(QVector<float>s,m_seriesData){
//        if(s.size() >(nofXPoints)){
//            s.remove(0,200);
//            //qDebug()<<"Remove data";
//        }
//    }
    // update series
    for(int i=0;i<3;i++){
        if(m_seriesData[i].size() > nofXPoints){
            m_seriesData[i].remove(0,200);
        }
        //qDebug()<<"Series:"<<i<<" Size="<<m_seriesData[i].size();
        QList<QPointF> pts;
        int ptStart,ptEnd;
        for(int pt=0;pt<m_seriesData[i].size();pt++){
            pts.append(QPointF(pt,m_seriesData[i].at(pt)));
        }
        m_series.at(i)->replace(pts);
    }
    //qDebug()<<m_seriesData[0].at(0)<<m_seriesData[0].at(1)<<m_seriesData[2].at(0);
    //ui->graphicsView->viewport()->update();
}

void chartView::handle_chart_update()
{
    m_mutex.lock();
    QList<QPointF> pts;

    QChart *c = ui->graphicsView->chart();
    QLineSeries *s;
    int seriesCount = m_chartType==0?3:6;
    QVector<float> dat;
    for(int i=0;i<seriesCount;i++){
        dat = m_seriesData[i];
        pts.clear();
        for(int pt=0;pt<dat.size();pt++){
            pts.append(QPointF(pt,dat.at(pt)));
        }
        if(i < c->series().count()){
            s = (QLineSeries*)c->series().at(i);
            s->replace(pts);
        }
    }

    m_mutex.unlock();
    ui->graphicsView->viewport()->update();

    c = ui->graphicsView_2->chart();
    for(int i=0;i<seriesCount;i++){
        pts.clear();
        for(int pt=0;pt<m_seriesFFTData[i].size();pt++){
            pts.append(QPointF(pt,m_seriesFFTData[i].at(pt)));
        }
        if(i < c->series().count()){
            s = (QLineSeries*)c->series().at(i);
            s->replace(pts);
        }
    }
    ui->graphicsView_2->viewport()->update();
}



void chartView::on_pbLog_clicked()
{
    //ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()==0?1:0);
    ui->graphicsView_2->setVisible(ui->graphicsView_2->isVisible()?false:true);
}

void chartView::showTrend(int id, bool v)
{
    QChart *c = ui->graphicsView->chart();
    if(id < c->series().size()){
        c->series().at(id)->setVisible(v);
        ui->graphicsView->viewport()->repaint();
    }
}

void chartView::on_pbCapture_clicked()
{
    QString m_logPath = QDir::currentPath()+ "/capture";
    if(!QDir(m_logPath).exists()){
        QDir().mkdir(m_logPath);
    }
    switch(m_chartType){
    case 0:{ // dual view
        QPixmap p = ui->graphicsView->grab();
        QImage image = p.toImage();
        image.save(QString("%1/%2-%3_WAVE.png").arg(m_logPath).arg(windowTitle()).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss")));
        p = ui->graphicsView_2->grab();
        QImage image2 = ui->graphicsView_2->grab().toImage();
        image2.save(QString("%1/%2-%3_FFT.png").arg(m_logPath).arg(windowTitle()).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss")));
        }
        break;
    case 1:

        QPixmap p = ui->graphicsView->grab();
        QImage image = p.toImage();
        image.save(QString("%1/%2-%3_TREND.png").arg(m_logPath).arg(windowTitle()).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss")));
        break;
    }
}
