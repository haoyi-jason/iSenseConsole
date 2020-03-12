#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "isense_chart.h"
#include <QtDebug>
#include <QAbstractListModel>
#include <QPushButton>
#

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //isense_chart *c = new isense_chart();
    //c->setVisible(true);
    //connect(this,&MainWindow::destroyed,c,&isense_chart::close);
//    QString name = "COM1";
//    m_snode.setHostName(name);
//    int p = 115200;
//    m_snode.setPortNumber(p);

//    m_snode.open();

    m_ifmodel = new snode_interface_model;
    ui->tableView->setModel(m_ifmodel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->resizeColumnsToContents();
    //connect(this,&MainWindow::,m_ifmodel,&snode_interface_model::deleteLater);

    m_logModel = new QStringListModel();
    ui->lvLog->setModel(m_logModel);
    ui->lvLog->setEditTriggers(QAbstractItemView::NoEditTriggers);
    handle_log_message("System initial");

    m_simulator = new snode_simulator_model;
    ui->tableView2->setModel(m_simulator);

    minimizeAction = new QAction(tr("Minize"),this);
    connect(minimizeAction,&QAction::triggered,this,&MainWindow::hide);

    maximizeAction = new QAction(tr("Maximize"),this);
    connect(maximizeAction,&QAction::triggered,this,&MainWindow::showMaximized);

    restoreAction = new QAction(tr("Restore"),this);
    connect(restoreAction,&QAction::triggered,this,&MainWindow::showNormal);

    quitAction = new QAction(tr("Quit"),this);
    connect(quitAction,&QAction::triggered,this,&MainWindow::close);


    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    QIcon icon = QIcon(":/images/vnode");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->setToolTip("Tooltip");

    //trayIcon->show();
}

MainWindow::~MainWindow()
{
    qDebug()<<Q_FUNC_INFO;
    delete ui;
//    delete m_ifmodel;
}

void MainWindow::on_pushButton_clicked()
{
    qDebug()<<Q_FUNC_INFO;
//    m_snode.setCommand(QString("First").toUtf8());
//    m_snode.setCommand(QString("Second").toUtf8());
//    m_snode.setCommand(QString("Third").toUtf8());
//    QJsonObject obj;

//    quint8 msg[] = {0x11,0x22,0x33,0x44,0x55};
//    //quint8 msg[] = "Hello";
//    QByteArray ba;
//    ba.append((char*)msg,5);

//    obj.insert("KEY1", 1);
//    obj.insert("Key2",2);

//    QJsonArray ar;
//    QString ss = QString(ba.toHex());
//    obj.insert("Key3",ss);
//    QJsonDocument d(obj);
////    qDebug()<<QString(d.toJson());
//    ui->txtSetting->setText(d.toJson());



//    qDebug()<<ss.toLatin1();

    QChartView a;
    a.setVisible(true);
}

void MainWindow::on_pb_add_interface_clicked()
{
}

void MainWindow::on_pb_del_interface_clicked()
{
}

void MainWindow::handle_sonde_connect()
{
    QPushButton *btn = (QPushButton*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<btn->property("ID");

    int id = btn->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
    if(d!=nullptr){
        qDebug()<<"Device found";
        if(d->isOpen()){
            d->close();
            btn->setText("Connect");
        }else{
            if(d->open()){
                btn->setText("Disconnect");
            }
        }
    }else{
        qDebug()<<"Device NOT found";
    }
}

void MainWindow::handle_simulator_open()
{
    QPushButton *btn = (QPushButton*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<btn->property("ID");

    int id = btn->property("ID").toInt();
    snode_simulator *d = m_simulator->getItem(id);
    if(d!=nullptr){
        qDebug()<<"Device found"<<d->hostName()<<d->portNumber();
        //d->setCodec(d)
        d->open();
    }else{
        qDebug()<<"Device NOT found";
    }
}

void MainWindow::handle_simulator_close()
{
    QPushButton *btn = (QPushButton*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<btn->property("ID");

    int id = btn->property("ID").toInt();
    snode_simulator *d = m_simulator->getItem(id);
    if(d!=nullptr){
        qDebug()<<"Device found";
        d->close();
    }else{
        qDebug()<<"Device NOT found";
    }
}

void MainWindow::handle_snode_type_changed(int index)
{
    QComboBox *cbo = (QComboBox*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<cbo->property("ID");

    int id = cbo->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
    if(d){
        d->setType(static_cast<snode_interface::commType>(cbo->currentIndex()));
    }

}

void MainWindow::handle_snode_model_changed(int index)
{
    QComboBox *cbo = (QComboBox*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<cbo->property("ID");

    int id = cbo->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
    if(d){

    }else{
        qDebug()<<"No item found";
    }
}

void MainWindow::handle_snode_model_select(int index)
{
    qDebug()<<Q_FUNC_INFO<<index;
    QComboBox *cbo = (QComboBox*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<cbo->property("ID");

    int id = cbo->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
    if(d){
        d->setCodec(index);
        QModelIndex idx = m_ifmodel->index(id,4);
        qDebug()<<"Index:"<<idx;
        QComboBox *cbo = qobject_cast<QComboBox*>(ui->tableView->indexWidget(idx));
        if(cbo != nullptr){
//            qDebug()<<"ITEM:"<<cbo->itemText(0);
            cbo->blockSignals(true);
            cbo->clear();
//            qDebug()<<d->codec_configs();
            cbo->addItems(d->codec_configs());
            cbo->blockSignals(false);
        }
    }else{
        qDebug()<<"No item found";
    }
}

void MainWindow::handle_simulator_model_select(int index)
{
    qDebug()<<Q_FUNC_INFO<<index;
    QComboBox *cbo = (QComboBox*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<cbo->property("ID");

    int id = cbo->property("ID").toInt();
    snode_simulator *d = m_simulator->getItem(id);
    if(d){
        d->setCodec(index);
        QModelIndex idx = m_simulator->index(id,4);
        qDebug()<<"Index:"<<idx;
        QComboBox *cbo = qobject_cast<QComboBox*>(ui->tableView2->indexWidget(idx));
        if(cbo != nullptr){
//            qDebug()<<"ITEM:"<<cbo->itemText(0);
            cbo->blockSignals(true);
            cbo->clear();
//            qDebug()<<d->codec_configs();
            cbo->addItems(d->codec_configs());
            cbo->blockSignals(false);
        }
    }else{
        qDebug()<<"No item found";
    }
}

void MainWindow::handle_snode_setting_select(int index)
{
    qDebug()<<Q_FUNC_INFO<<index;
    QComboBox *cbo = (QComboBox*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<cbo->property("ID");

    int id = cbo->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);

    if(d){
        //d->setCurrSetting(index+0x40);
        //d->getConfigItem(cbo->currentText());
        d->setCurrConfigName(cbo->currentText());
        //ui->txtSetting->setText(d->getCodecParam(cbo->currentText().trimmed()));
    }else{
        qDebug()<<"No item found";
    }
}

void MainWindow::handle_snode_get_param()
{
    QPushButton *btn = (QPushButton*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<btn->property("ID");

    int id = btn->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
    if(d == nullptr) return;

    QModelIndex idx = m_ifmodel->index(id,4);
    qDebug()<<"Index:"<<idx;
    QComboBox *cbo = qobject_cast<QComboBox*>(ui->tableView->indexWidget(idx));

    if(cbo != nullptr){
       // d->getConfigItem(cbo->currentText());
    }
}

void MainWindow::handle_snode_set_param()
{
    QPushButton *btn = (QPushButton*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<btn->property("ID");

    int id = btn->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
    if(d == nullptr) return;

    QModelIndex idx = m_ifmodel->index(id,4);
    qDebug()<<"Index:"<<idx;
    QComboBox *cbo = qobject_cast<QComboBox*>(ui->tableView->indexWidget(idx));

    if(cbo != nullptr){
//        d->setCodecParam(cbo->currentText(),ui->txtSetting->toPlainText().toLatin1());
//        d->setConfigItem(cbo->currentText());
    }
}

void MainWindow::handle_snode_start_stop()
{
    QPushButton *btn = (QPushButton*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<btn->property("ID");

    int id = btn->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
    if(d == nullptr) return;

    d->start_stop();
    if(d->isActive()){
        btn->setText("STOP");
    }
    else{
        btn->setText("START");
    }

}

void MainWindow::handle_snode_view_chart()
{
    qDebug()<<Q_FUNC_INFO;
    QPushButton *btn = (QPushButton*)sender();
    qDebug()<<Q_FUNC_INFO<<"Sender ID:"<<btn->property("ID");

    int id = btn->property("ID").toInt();
    snode_interface *d = m_ifmodel->getItem(id);
//    d->showChart(true);
//    isense_chart *c = new isense_chart;
//    c->setProperty("ID",id);

//    m_charts.append(c);
//    c->setVisible(true);

//    connect(c,&isense_chart::destroyed,this, &MainWindow::handle_chart_destroy);


//    chartView *v = new chartView;
//    v->setProperty("ID",id);
//    v->show();
//    m_charts.append(v);
//    connect(v,&chartView::destroyed,this,&MainWindow::handle_chart_destroy);
    //v->setTitle("TREND");
//    v->setChartType("FFT");
}

void MainWindow::handle_device_setup_update(QByteArray b)
{
//    snode_interface *d = m_ifmodel->getItem(name);
//    if(d == nullptr) return;

//    int r = m_ifmodel->getItemId(name);
//    if(r == -1) return;

//    QModelIndex idx = m_ifmodel->index(r,4);
//    //qDebug()<<"Index:"<<idx;
//    QComboBox *cbo = qobject_cast<QComboBox*>(ui->tableView->indexWidget(idx));

    ui->txtSetting->setText(b);



}

void MainWindow::handle_device_model_update(QString name, int id)
{
    qDebug()<<Q_FUNC_INFO<<name<<id;
    snode_interface *d = m_ifmodel->getItem(name);
    if(d == nullptr) return;

    int r = m_ifmodel->getItemId(name);
    if(r == -1) return;

    QModelIndex idx = m_ifmodel->index(r,5);
    //qDebug()<<"Index:"<<idx;
    QComboBox *cbo = qobject_cast<QComboBox*>(ui->tableView->indexWidget(idx));

    cbo->blockSignals(true);
    cbo->clear();
    cbo->addItems(d->codec_configs());
    cbo->setCurrentIndex(cbo->findText("MODULE"));
    cbo->blockSignals(false);
    //ui->txtSetting->setText(d->getCodecParam(cbo->currentText().trimmed()));

    ui->tableView->viewport()->update();


}

void MainWindow::on_pbListen_clicked()
{


}

void MainWindow::on_pbClose_clicked()
{
    m_cmdServer->deleteLater();
    m_cmdServer = nullptr;
}

void MainWindow::add_interface(int type, QString hostName, int port)
{
    qDebug()<<Q_FUNC_INFO;
    // check if the hostname present
    if(m_ifmodel->getItem(hostName)!= nullptr) return;

    m_ifmodel->insertRows(m_ifmodel->rowCount(),1);
    int row = m_ifmodel->rowCount()-1;
    QPushButton *btn = new QPushButton("connect");
    btn->setProperty("ID",row);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,7),btn);
    QComboBox *cbo = new QComboBox;
    cbo->addItem("LAN");
    cbo->addItem("SERIAL");
    if(type == snode_interface::COMM_SERIAL)
        cbo->setCurrentIndex(1);
    else
        cbo->setCurrentIndex(0);
    cbo->setProperty("ID",row);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,0),cbo);

    connect(btn,&QPushButton::clicked,this,&MainWindow::handle_sonde_connect);
    connect(cbo,SIGNAL(currentIndexChanged(int)),this,SLOT(handle_snode_type_changed(int)));

//    m_ifmodel->setData(m_ifmodel->index(row,1),hostName);
//    m_ifmodel->setData(m_ifmodel->index(row,2),port);
    snode_interface *d = m_ifmodel->getItem(row);
    if(d != nullptr){
        connect(d,&snode_interface::sendLog,this,&MainWindow::handle_log_message);
        //connect(d,&snode_interface::newData,this,&MainWindow::handle_data_received);
        d->setType(snode_interface::COMM_SERIAL);
        d->setHostName("COM1");
        d->setPortNumber(115200);
        d->setCodec(0);
    }
}

void MainWindow::handle_log_message(QString msg)
{
    QStringList sl = m_logModel->stringList();
    sl.append(msg);
    m_logModel->setStringList(sl);
    ui->lvLog->scrollToBottom();

    if(m_logModel->rowCount() > 120){
        m_logModel->removeRows(0,20);
    }
//    sl << "TEST"<<msg;
//    m_logModel->insertRow(r);
//    m_logModel->setData(m_logModel->index(r+1,0),msg);
}

void MainWindow::handle_data_received(QString host, QByteArray b)
{
    QStringList sl = m_logModel->stringList();
    sl.append(QString("Receive data from %1, size = %2").arg(host).arg(b.size()));
    m_logModel->setStringList(sl);
}

void MainWindow::on_pbAddSimulator_clicked()
{
}

void MainWindow::handle_chart_destroy(QObject *obj)
{
    qDebug()<<Q_FUNC_INFO;
    int id = obj->property("ID").toInt();
    if(id >= 0){
        foreach(chartView *d,m_charts){
            if(d->property("ID")==id){
                m_charts.removeOne((chartView*)obj);
            }
        }
    }
}

void MainWindow::on_actionE_xit_triggered()
{
    close();
}

snode_interface *MainWindow::addInterface(QString type, QString name, int port)
{
    qDebug()<<Q_FUNC_INFO;
    // check if interface exist
    snode_interface *d;
    d = m_ifmodel->getItem(name);
    if(d != nullptr) return d;
    m_ifmodel->insertRows(m_ifmodel->rowCount(),1);
    int row = m_ifmodel->rowCount()-1;
    d = m_ifmodel->getItem(row);

    if(d == nullptr) return nullptr ;

    connect(this,&MainWindow::close,d,&snode_interface::deleteLater);
    d->setHostName(name);
    d->setPortNumber(port);

    QPushButton *btn = new QPushButton("Connect");
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,d,&snode_interface::connectToHost);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,0),btn);

    QComboBox *cbo = new QComboBox;
    cbo->addItems(d->supportInterface());
    cbo->setCurrentIndex(cbo->findText(type));
    cbo->setProperty("ID",row);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,1),cbo);
    connect(cbo,&QComboBox::currentTextChanged,d,&snode_interface::setInterfaceType);

    cbo = new QComboBox;
    cbo->addItems(d->codec_configs());
    cbo->setCurrentIndex(cbo->findText("MODULE"));
    cbo->setProperty("ID",row);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,5),cbo);
    connect(cbo,&QComboBox::currentTextChanged,d,&snode_interface::setCurrConfigName);
    ui->txtSetting->setText(d->getCodecParam(cbo->currentText().trimmed()));

    btn = new QPushButton("Refresh");
    btn->setProperty("ID",row);
    //connect(btn,&QPushButton::clicked,this,&MainWindow::handle_snode_get_param);
    connect(btn,&QPushButton::clicked,d,&snode_interface::getConfigItem);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,6),btn);

    btn = new QPushButton("SET");
    //btn->setEnabled(false);
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,d,&snode_interface::setConfigItem);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,7),btn);

    btn = new QPushButton("START");
    btn->setProperty("ID",row);
    //btn->setEnabled(false);
    connect(btn,&QPushButton::clicked,d,&snode_interface::start_stop);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,8),btn);

    btn = new QPushButton("ChartView");
    btn->setProperty("ID",row);
    //btn->setEnabled(false);
    connect(btn,&QPushButton::clicked,d,&snode_interface::showChart);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,9),btn);

    btn = new QPushButton("LOG");
    btn->setCheckable(true);
    //btn->setEnabled(false);
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,d,&snode_interface::enableLog);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,10),btn);

    connect(d,&snode_interface::sendLog,this,&MainWindow::handle_log_message);
    connect(d,&snode_interface::codec_setup_updated,this,&MainWindow::handle_device_setup_update);
    connect(d,&snode_interface::codec_model_change,this,&MainWindow::handle_device_model_update);
    connect(ui->txtSetting,&QTextEdit::textChanged,d,&snode_interface::setContent);


    ui->tableView->resizeColumnsToContents();

    return d;
}

void MainWindow::on_action_Add_triggered()
{
    qDebug()<<Q_FUNC_INFO;
    m_ifmodel->insertRows(m_ifmodel->rowCount(),1);
    int row = m_ifmodel->rowCount()-1;
    snode_interface *d = m_ifmodel->getItem(row);

    if(d == nullptr) return ;

    //connect(this,&MainWindow::close,d,&snode_interface::deleteLater);

    QPushButton *btn = new QPushButton("Connect");
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,d,&snode_interface::connectToHost);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,0),btn);

    QComboBox *cbo = new QComboBox;
    cbo->addItems(d->supportInterface());
    cbo->setCurrentIndex(cbo->findText(d->currentInterface()));
    cbo->setProperty("ID",row);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,1),cbo);
    connect(cbo,&QComboBox::currentTextChanged,d,&snode_interface::setInterfaceType);

    cbo = new QComboBox;
    cbo->addItems(d->codec_configs());
    cbo->setCurrentIndex(cbo->findText("MODULE"));
    cbo->setProperty("ID",row);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,5),cbo);
    connect(cbo,&QComboBox::currentTextChanged,d,&snode_interface::setCurrConfigName);
    ui->txtSetting->setText(d->getCodecParam(cbo->currentText().trimmed()));

    btn = new QPushButton("Refresh");
    btn->setProperty("ID",row);
    //connect(btn,&QPushButton::clicked,this,&MainWindow::handle_snode_get_param);
    connect(btn,&QPushButton::clicked,d,&snode_interface::getConfigItem);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,6),btn);

    btn = new QPushButton("SET");
    //btn->setEnabled(false);
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,d,&snode_interface::setConfigItem);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,7),btn);

    btn = new QPushButton("START");
    btn->setProperty("ID",row);
    //btn->setEnabled(false);
    connect(btn,&QPushButton::clicked,d,&snode_interface::start_stop);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,8),btn);

    btn = new QPushButton("ChartView");
    btn->setProperty("ID",row);
    //btn->setEnabled(false);
    connect(btn,&QPushButton::clicked,d,&snode_interface::showChart);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,9),btn);

    btn = new QPushButton("LOG");
    btn->setCheckable(true);
    //btn->setEnabled(false);
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,d,&snode_interface::enableLog);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,10),btn);

    btn = new QPushButton("FFT");
    btn->setCheckable(true);
    //btn->setEnabled(false);
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,d,&snode_interface::enableFFT);
    ui->tableView->setIndexWidget(m_ifmodel->index(row,11),btn);

    connect(d,&snode_interface::sendLog,this,&MainWindow::handle_log_message);
    connect(d,&snode_interface::codec_setup_updated,this,&MainWindow::handle_device_setup_update);
    connect(d,&snode_interface::codec_model_change,this,&MainWindow::handle_device_model_update);
    connect(ui->txtSetting,&QTextEdit::textChanged,d,&snode_interface::setContent);


    ui->tableView->resizeColumnsToContents();
}

void MainWindow::on_action_Delete_triggered()
{
    qDebug()<<Q_FUNC_INFO;
    //m_ifmodel->removeRow(ui->tableView->selectedIndexes().at(0).row());
    ui->tableView->model()->removeRows(ui->tableView->currentIndex().row(),1);
}

void MainWindow::on_action_Start_triggered()
{
    m_cmdServer = new command_server;
    if(m_cmdServer->listen(5001)){
        qDebug()<<Q_FUNC_INFO<< "Listening....";
        connect(m_cmdServer,&command_server::add_interface,this,&MainWindow::add_interface);
    }else{
        qDebug()<<Q_FUNC_INFO<< "Listen fail";
    }
}

void MainWindow::on_action_Add_2_triggered()
{
    qDebug()<<Q_FUNC_INFO;
    m_simulator->insertRows(m_simulator->rowCount(),1);
    int row = m_simulator->rowCount()-1;

    snode_simulator *d = m_simulator->getItem(row);
    if(d == nullptr) return;

    QPushButton *btn = new QPushButton("Listen");
    btn->setProperty("ID",row);
    connect(btn,&QPushButton::clicked,this,&MainWindow::handle_simulator_open);
    ui->tableView2->setIndexWidget(m_simulator->index(row,4),btn);

    QComboBox *cbo = new QComboBox;
    cbo->addItems(d->supportInterface());
//    cbo->addItem("SERIAL");
//    cbo->addItem("LAN");
    cbo->setCurrentIndex(0);
    cbo->setProperty("ID",row);
    ui->tableView2->setIndexWidget(m_simulator->index(row,0),cbo);
    connect(cbo,SIGNAL(currentIndexChanged(int)),this,SLOT(handle_snode_type_changed(int)));

    cbo = new QComboBox;
    cbo->addItems(d->supportModels());
    cbo->setCurrentIndex(0);
    cbo->setProperty("ID",row);
    ui->tableView2->setIndexWidget(m_simulator->index(row,3),cbo);
    connect(cbo,SIGNAL(currentIndexChanged(int)),this,SLOT(handle_simulator_model_select(int)));

    if(d != nullptr){
        connect(d,&snode_interface::sendLog,this,&MainWindow::handle_log_message);
        //connect(d,&snode_interface::newData,this,&MainWindow::handle_data_received);
        //d->setType(snode_interface::COMM_SERIAL);
        //d->setHostName("COM2");
        //d->setPortNumber(115200);
        //d->setCodec(1);

    }
    ui->tableView2->resizeColumnsToContents();
}

void MainWindow::on_action_Clear_triggered()
{
    m_logModel->removeRows(0,m_logModel->rowCount());
}

void MainWindow::on_actionE_xit_2_triggered()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    qDebug()<<Q_FUNC_INFO;
    if(trayIcon->isVisible()){
        qDebug()<<"Hide Window";
        hide();
        e->ignore();
    }

    delete m_ifmodel;
}

void MainWindow::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::WindowStateChange){
        if(windowState()==Qt::WindowMinimized){
            hide();
        }
    }
    e->accept();
}

void MainWindow::on_action_Load_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open JSON file"), "./", tr("JSON files (*.txt *.json)"));
    if(fileName.isNull()) return;

    QFile f(fileName);
    f.open(QIODevice::ReadOnly);

    QByteArray b = f.readAll();
    f.close();
    parseConfig(b);
}

void MainWindow::on_action_Save_triggered()
{

}

void MainWindow::parseConfig(QByteArray json)
{
    qDebug()<<Q_FUNC_INFO;
    QString msg = Q_FUNC_INFO;
    QJsonParseError e;
    QJsonDocument d = QJsonDocument::fromJson(json,&e);

    if(d.isNull()){
        msg += e.errorString();
        handle_log_message(msg);
        return;
    }

    QJsonObject obj1 = d.object();
    if(obj1.contains("Interfaces")){
        QJsonArray ja = obj1["Interfaces"].toArray();
        foreach(const QJsonValue &value,ja){
            QJsonObject obj = value.toObject();
            QString type;
            QString host;
            int baud = 0;
            if(obj.contains("TYPE")){
                type = obj.value("TYPE").toString();
            }
            if(obj.contains("HOSTNAME")){
                host = obj.value("HOSTNAME").toString();
            }
            if(obj.contains("BAUDRATE")){
                baud = obj.value("BAUDRATE").toInt();
            }
            if(obj.contains("PORT")){
                baud = obj.value("PORT").toInt();
            }

            if((!type.isNull()) && (!host.isNull()) && (baud != 0)){
                // add interface
                snode_interface *d = addInterface(type,host,baud);
                if(obj.contains("MODEL")){
                    d->on_codec_model_changed(obj.value("MODEL").toString());
                }
                if(d != nullptr){
                    if(obj.contains("NODE")){
                        QJsonObject o = obj.value("NODE").toObject();
                        QJsonDocument doc(o);
                        d->setCodecParam("NODE",doc.toJson());
                    }
                    if(obj.contains("ACCEL")){
                        QJsonObject o = obj.value("ACCEL").toObject();
                        QJsonDocument doc(o);
                        d->setCodecParam("ACCEL",doc.toJson());
                    }
                    if(obj.contains("TIME")){
                        QJsonObject o = obj.value("TIME").toObject();
                        QJsonDocument doc(o);
                        d->setCodecParam("TIME",doc.toJson());
                    }
                    if(obj.contains("FREQ")){
                        QJsonObject o = obj.value("FREQ").toObject();
                        QJsonDocument doc(o);
                        d->setCodecParam("FREQ",doc.toJson());
                    }
                    d->autoConfig();
                }
            }

        }
    }
}

void MainWindow::on_pb_a_clicked()
{
    snode_interface *d = m_ifmodel->getItem(0);
    if(d){
        d->chart()->setChartType(0);
    }
}

void MainWindow::on_pb_b_clicked()
{
    snode_interface *d = m_ifmodel->getItem(0);
    if(d){
        d->chart()->setChartType(1);
    }

}
