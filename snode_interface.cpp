#include <QObject>
#include <QtCore>
#include <QPointF>
#include "snode_interface.h"

snode_interface::snode_interface(QObject *parent) : QObject(parent)
{
    m_commType = COMM_SERIAL;
    m_hostName = "COM4";
    m_portNum = 115200;

    //    m_commType = COMM_SOCKET;
    //    m_hostName = "127.0.0.1";
    //    m_portNum = 5001;

    m_port = nullptr;
    m_socket = nullptr;
    m_file = nullptr;
    m_codec = nullptr;
    m_cmdHandler = new cmd_handler(nullptr, false);
    m_devName = "UNKNOW";
    m_isActive = false;
    setCodec(0);
    setCurrSetting(0);
    connect(m_cmdHandler,&cmd_handler::send_command,this,&snode_interface::writeCommand);

    QThread *thread = new QThread;
    m_chart = new chartView;
    m_chart->hide();
    m_chart->setChartType("TIME");

    connect(this,&snode_interface::close,m_chart,&chartView::deleteLater);

    //m_chart->moveToThread(thread);
    m_currConfigName = "MODULE";

//    m_currentSetting = 0;
    //m_port = new QSerialPort(this);

    //connect(&m_workThread,&QThread::finished,&m_cmdHandler,&cmd_handler::deleteLater);

    //m_cmdHandler.moveToThread(&m_workThread);

    m_stream = false;
    m_data.clear();
    m_configOnConnect = false;

    m_logPath = QDir::currentPath()+ "/log";
    if(!QDir(m_logPath).exists()){
        QDir().mkdir(m_logPath);
    }
}

snode_interface::~snode_interface()
{
    qDebug()<<Q_FUNC_INFO;
    //m_workThread.quit();
    //m_workThread.wait();

    if(m_socket){
        qDebug()<<"Delete socket";
        m_socket->close();
        m_socket->deleteLater();
    }
    if(m_port){
        qDebug()<<"Delete serial";
        m_port->close();
        m_port->deleteLater();
    }

    if(m_codec){
        qDebug()<<"Delete codec";
        delete m_codec;
    }

    if(m_chart){
        qDebug()<<"Delete chart";
        m_chart->close();
        m_chart->deleteLater();
//        delete m_chart;
        //        m_chart->deleteLater();
    }

}

void snode_interface::connectToHost()
{
    QPushButton *btn = (QPushButton*)sender();
    if(isOpen()){
        close();
        btn->setText("Connect");
    }else{
        if(open()){
            btn->setText("Disconnect");
        }
    }
}

bool snode_interface::open()
{
    qDebug()<<Q_FUNC_INFO;
//    m_cmdHandler->setHost(0,m_hostName,m_portNum);
//    if(m_cmdHandler->open()){
//        connect(m_cmdHandler,&cmd_handler::dataReady,this,&snode_interface::handleIncomingData,Qt::DirectConnection);
//        return true;
//    }
//    return false;
    bool ret = false;
    if(m_commType == COMM_SERIAL){
        log(LOG_INFO,QString("Connecto to port %1, baudrate=%2").arg(m_hostName).arg(m_portNum));
        if(m_port){
            m_port->close();
            m_port->deleteLater();
            m_port = nullptr;
        }
        m_port = new QSerialPort();
        m_port->setPortName(m_hostName);
        m_port->setBaudRate(m_portNum);
        m_port->setParity(QSerialPort::NoParity);
        m_port->setDataBits(QSerialPort::Data8);
        m_port->setStopBits(QSerialPort::OneStop);
        m_port->setFlowControl(QSerialPort::NoFlowControl);
        m_port->setReadBufferSize(1000);
        if(m_port->open(QIODevice::ReadWrite)){
            ret = true;
            connect(m_port,&QSerialPort::readyRead,this,&snode_interface::handleIncomingData);
            log(LOG_INFO,"Connection OK, start handler");
            //m_port->write("Hello\n");
            QString msg = Q_FUNC_INFO;
            switch(m_codecID){
            case 0:
                //m_codec = new snode_codec(this);
                msg += m_codec->metaObject()->className();
                emit sendLog(msg);
                break;
            case 1:
                //m_codec = new snode_vnode_codec;
                msg += m_codec->metaObject()->className();
                emit sendLog(msg);
                break;
            }
            m_cmdHandler->start();
            m_codec->clear();
            m_codec->issue_param_read("MODULE");
            if(m_configOnConnect){
                if(m_codecID == 1){
                    for(int i=0;i<4;i++){
                        ((snode_vnode_codec*)m_codec)->issue_param_write(i);
                    }
                }
            }

        }
        else{
            log(LOG_ERROR,QString("Connection failed,err = %1").arg(m_port->errorString()));
        }
    }
    else if(m_commType == COMM_SOCKET){
        log(LOG_INFO,QString("Connecto to ip %1, port=%2").arg(m_hostName).arg(m_portNum));
        if(m_socket){
            m_socket->close();
            m_socket->deleteLater();
            m_socket = nullptr;
        }
        m_socket = new QTcpSocket();
        m_socket->connectToHost(m_hostName,(unsigned short)m_portNum);
        if(m_socket->waitForConnected()){
            ret = true;
            connect(m_socket,&QTcpSocket::readyRead,this,&snode_interface::handleIncomingData);
            log(LOG_INFO,"Connection OK, start handler");
            //m_port->write("Hello\n");
            QString msg = Q_FUNC_INFO;
            switch(m_codecID){
            case 0:
                //m_codec = new snode_codec(this);
                msg += m_codec->metaObject()->className();
                emit sendLog(msg);
                break;
            case 1:
                //m_codec = new snode_vnode_codec;
                msg += m_codec->metaObject()->className();
                emit sendLog(msg);
                break;
            }
            m_cmdHandler->start();
            m_codec->clear();
            m_codec->issue_param_read("MODULE");
            if(m_configOnConnect){
                if(m_codecID == 1){
                    for(int i=0;i<4;i++){
                        ((snode_vnode_codec*)m_codec)->issue_param_write(i);
                    }
                }
            }
        }
        else{
            log(LOG_ERROR,QString("Connection failed,err = %1").arg(m_socket->errorString()));
        }

    }
    return ret;
}

bool snode_interface::close()
{
//    return m_cmdHandler->close();

    if(m_commType == COMM_SERIAL){
        log(LOG_INFO,QString("Close port %1").arg(m_hostName));
        if(m_port){
            m_port->close();
            m_port->deleteLater();
            m_port = nullptr;
            m_cmdHandler->stop();
//            m_cmdHandler->deleteLater();
//            m_cmdHandler = nullptr;
        }
    }
    else if(m_commType == COMM_SOCKET){
        if(m_socket){
            m_socket->close();
            m_socket->deleteLater();
            m_socket = nullptr;
            m_cmdHandler->stop();
        }
    }
    return true;
}

void snode_interface::setCodec(int index)
{
    m_codecID = index;
    qDebug()<<Q_FUNC_INFO;
    //snode_codec *codec = nullptr;
    QString msg = Q_FUNC_INFO;
    snode_codec *c = nullptr;
    if(m_codec != nullptr){
        c = m_codec;
        m_codec = nullptr;
    }
    switch(index){
    case 0:

        m_codec = new snode_codec;
        connect(m_codec,&snode_codec::write_data,this,&snode_interface::setCommand);
        connect(m_codec,&snode_codec::send_log,this,&snode_interface::on_codec_message);
        connect(m_codec,&snode_codec::setupReceived,this,&snode_interface::on_codec_setup_received);
        connect(m_codec,&snode_codec::modelNameUpdate,this,&snode_interface::on_codec_model_changed);
        connect(this,&snode_interface::newData,m_codec,&snode_codec::on_decoder_received);

//                connect(m_codec,&snode_codec::write_data,m_cmdHandler,&cmd_handler::add_cmd,Qt::DirectConnection);
        break;
    case 1:
        m_codec = new snode_vnode_codec;
        connect(m_codec,&snode_vnode_codec::write_data,this,&snode_interface::setCommand);
        connect(m_codec,&snode_vnode_codec::send_log,this,&snode_interface::on_codec_message);
        connect(m_codec,&snode_vnode_codec::setupReceived,this,&snode_interface::on_codec_setup_received);
        connect(this,&snode_interface::newData,(snode_vnode_codec*)m_codec,&snode_vnode_codec::on_decoder_received);
        connect(m_codec,&snode_vnode_codec::modelNameUpdate,this,&snode_interface::on_codec_model_changed);
        connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::newRecord,this,&snode_interface::receiveVNodeData);
        //connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::newRaw,this,&snode_interface::receiveStreamData);
//                connect(m_codec,&snode_vnode_codec::write_data,m_cmdHandler,&cmd_handler::add_cmd,Qt::DirectConnection);
        connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::newRecord,m_chart,&chartView::addRecord);
//        connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::newRaw,m_chart,&chartView::addStream);
//        connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::newSeries,m_chart,&chartView::updateSeries);
        connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::newWave,m_chart,&chartView::updateWave);
        connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::newFFT,m_chart,&chartView::updateFFT);
        connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::pidUpdate,this,&snode_interface::receivePID);
        if(c != nullptr){
            m_codec->setParam("MODULE",c->getParam("MODULE"));
            m_codec->setParam("SERIAL",c->getParam("MODULE"));
            m_codec->setParam("LAN",c->getParam("MODULE"));
            m_codec->setParam("WLAN",c->getParam("MODULE"));
        }
        break;
    }
    if(c != nullptr)
        c->deleteLater();
}


bool snode_interface::isOpen() const
{
    if(m_commType == COMM_SERIAL && m_port != nullptr){
        return m_port->isOpen();
    }
    if(m_commType == COMM_SOCKET && m_socket != nullptr){
        return m_socket->isOpen();
    }

    return false;
}

void snode_interface::writeCommand(QByteArray b)
{
    QString msg = Q_FUNC_INFO;
    qDebug()<<Q_FUNC_INFO;
    msg += "Send data via "+hostName();
    if(m_commType == COMM_SERIAL){
        if(isOpen()){
            m_port->write(b);
        }else{
            msg += "Device not open";
        }
    }
    else if(m_commType == COMM_SOCKET){
        if(isOpen()){
            m_socket->write(b);
        }
        else{
            msg += "Socket not available";
        }
    }
    emit sendLog(msg);
}

void snode_interface::on_codec_message(QString msg)
{
    log(LOG_CODEC,msg);
}

void snode_interface::on_codec_resp(QByteArray b)
{
    qDebug()<<Q_FUNC_INFO<<b;
    //m_cmdHandler->add_cmd(b);
}

void snode_interface::on_codec_setup_received(int id)
{
    qDebug()<<Q_FUNC_INFO<<id;
    QByteArray b = getCodecParam(id);
    emit codec_setup_updated(b);
    if(id == 0x0 && m_codecID == 0x1){
        m_chart->setChartType(((snode_vnode_codec*)m_codec)->nodeMode());
    }
}

void snode_interface::on_codec_model_changed(QString name)
{
    qDebug()<<Q_FUNC_INFO<<name;
    setCodec(model_map.value(name));
    m_devName = name;
    emit codec_model_change(m_hostName,model_map.value(name));
}

void snode_interface::handleIncomingData()
{
    QByteArray recv;
    if(m_commType == COMM_SERIAL){
        emit newData(m_port->readAll());
        m_cmdHandler->set_responsed(true);
    }
    else if(m_commType == COMM_SOCKET){
        emit newData(m_socket->readAll());
        m_cmdHandler->set_responsed(true);
    }
}

void snode_interface::parseData(QByteArray b)
{
    m_data.append(b);
    if(m_data.size() < HEADER_SIZE) return;
    cmd_header_t header;
    memcpy((char*)&header,m_data.constData(),8);
    if(header.magic1 == MAGIC1 && header.magic2 == MAGIC2){
        if(m_data.size() < header.len) return;
        // parse data
        quint8 mask = header.type & 0xf0;
//        m_parser->parseData(m_data,header.len);
//        switch (mask) {
//        case MASK_DATA:

//            break;
//        }
    }

}

//void snode_interface::setParser(snode_data_parser *p)
//{
//    m_parser = p;
//}

void snode_interface::setCommand(QByteArray b)
{
    m_cmdHandler->add_cmd(b);
}

void snode_interface::log(logType type, QString message)
{
    QString msg = this->metaObject()->className();
    switch(type){
        case LOG_INFO:
        msg +=" INFO:";
        break;
    case LOG_WARNING:
        msg += " WARNING:";
        break;
    case LOG_ERROR:
        msg += " ERROR:";
        break;
    case LOG_CODEC:
        msg += " CODEC:";
    }
    msg += message;
    emit sendLog(msg);
}

void snode_interface::query_device(param_id id)
{
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_SETUP;
    header.pid = static_cast<quint8>(id);
    header.len = 8;
    header.crc = checksum((quint8*)&header,8);
    QByteArray b;
    b.append((char*)&header,header.len);
    setCommand(b);
//    switch(id){
//        case PARAM_SENSOR_BASE:
//        break;
//    case PARAM_SENSOR_BASE_P1:
//        break;
//    case PARAM_SENSOR_BASE_P2:
//        break;
//    case PARAM_SENSOR_BASE_P3:
//        break;
//    case PARAM_MODULE_CONFIG:
//        break;
//    case PARAM_MODULE_SERIAL:
//        break;
//    case PARAM_MODULE_LAN:
//        break;
//    case PARAM_MODULE_WLAN:
//        break;
//    }
}

void snode_interface::config_device(param_id id, QByteArray b)
{
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_SETUP;
    header.pid = static_cast<quint8>(id);
    header.len = 8+b.size();
    header.crc = checksum((quint8*)&header,8);
    QByteArray b2;
    b2.append((char*)&header,header.len);
    b2.append(b);
    setCommand(b2);
}

quint16 snode_interface::checksum(const quint8 *data, quint16 length)
{
    quint16 chksum = 0;
    for(int i=0;i<length;i++){
        if(i ==6)continue;
        if(i==7)continue;
        chksum+=data[i];
    }

    return chksum;
}

void snode_interface::set_param_json_by_id(int id, QByteArray json)
{
    switch(m_codecID){
    case 0:
        m_codec->setParam(id,json);
        break;
    case 1:
        ((snode_vnode_codec*)m_codec)->setParam(id,json);
        break;
    }
}

void snode_interface::setCodecParam(QString name, QByteArray json)
{
    switch(m_codecID){
    case 0:
        m_codec->setParam(name,json);
        break;
    case 1:
        ((snode_vnode_codec*)m_codec)->setParam(name,json);
        break;
    }
}

void snode_interface::setCodecParam(QByteArray json)
{
    switch(m_codecID){
    case 0:
        m_codec->setParam(m_currConfigName,json);
        break;
    case 1:
        ((snode_vnode_codec*)m_codec)->setParam(m_currConfigName,json);
        break;
    }
}

QByteArray snode_interface::getCodecParam(QString name)
{
    qDebug()<<Q_FUNC_INFO<<m_codecID<<name;
    switch(m_codecID){
    case 0:
        return m_codec->getParam(name);
        break;
    case 1:
        return ((snode_vnode_codec*)m_codec)->getParam(name);
        break;
    }
    return QByteArray();
}

QByteArray snode_interface::getCodecParam(int id)
{
    qDebug()<<Q_FUNC_INFO<<m_codecID<<id;
    switch(m_codecID){
    case 0:
        return m_codec->getParam(id);
        break;
    case 1:
        return ((snode_vnode_codec*)m_codec)->getParam(id);
        break;
    }
    return QByteArray();
}

QByteArray snode_interface::get_param_json_by_id(int id)
{
    qDebug()<<Q_FUNC_INFO<<m_codecID<<id;
    switch(m_codecID){
    case 0:
        return m_codec->getParam(id);
        break;
    case 1:
        return ((snode_vnode_codec*)m_codec)->getParam(id);
        break;
    }
    return QByteArray();
}

void snode_interface::read_config(uint8_t cfg_id)
{
//    switch(m_currentSetting){
//    case 0:
//        m_codec->issue_param_read(m_currentSetting);
//        break;
//    case 1:
//        ((snode_vnode_codec*)m_codec)->issue_param_read(m_currentSetting);
//        break;
//    }
}

void snode_interface::write_config(uint8_t cfg_id)
{
//    switch(m_currentSetting){
//    case 0:
//        m_codec->issue_param_write(m_currentSetting);
//        break;
//    case 1:
//        ((snode_vnode_codec*)m_codec)->issue_param_write(m_currentSetting);
//        break;
//    }
}

void snode_interface::send_command(uint8_t command)
{
    switch(m_currentSetting){
    case 0:
        m_codec->issue_command(command);
        break;
    case 1:
        ((snode_vnode_codec*)m_codec)->issue_command(command);
        break;
    }
}

void snode_interface::start_stop()
{
    if(!isOpen()) return;
    //qDebug()<<Q_FUNC_INFO;
    QPushButton *btn = (QPushButton*)sender();
    if(isActive()){
        m_codec->issue_active(false);
        m_isActive = false;
        btn->setText("START");
        m_stream = true;
        m_data.clear();
    }
    else{
        m_codec->issue_active(true);
        m_isActive = true;
        btn->setText("STOP");
        m_stream = false;
    }
}

QStringList snode_interface::codec_configs()
{
    qDebug()<<Q_FUNC_INFO<<m_codecID;
    if(m_codec == nullptr){
        qDebug()<<"codec is null";
        return QStringList();
    }
    switch(m_codecID){
    case 0:
        return m_codec->supportConfig();
        break;
    case 1:
        return ((snode_vnode_codec*)m_codec)->supportConfig();
        break;
    }
    return  QStringList();
}

void snode_interface::setConfigItem()
{
    if(!isOpen()) return;
    switch(m_codecID){
    case 0:
        m_codec->setParam(m_currConfigName,m_tempContent.toLatin1());
        m_codec->issue_param_write(m_currConfigName);
        break;
    case 1:
        ((snode_vnode_codec*)m_codec)->setParam(m_currConfigName,m_tempContent.toLatin1());
        ((snode_vnode_codec*)m_codec)->issue_param_write(m_currConfigName);
        break;
    }
}

void snode_interface::getConfigItem()
{
    if(isOpen()){
        switch(m_codecID){
        case 0:
            m_codec->issue_param_read(m_currConfigName);
            break;
        case 1:
            ((snode_vnode_codec*)m_codec)->issue_param_read(m_currConfigName);
            break;
        }
    }else{
        switch(m_codecID){
        case 0:
            emit codec_setup_updated(m_codec->getParam(m_currConfigName));
            break;
        case 1:
            emit codec_setup_updated(((snode_vnode_codec*)m_codec)->getParam(m_currConfigName));
            break;
        }
    }
}
void snode_interface::enableLog(bool v){
    qDebug()<<Q_FUNC_INFO;
    if(m_logPath.isNull()) return;
    m_log = v;
    if(m_log){
        if(m_file == nullptr){
            m_file = new QFile;
        }
        QString fname = QString("%1/%2_%3.csv").arg(m_logPath).arg(m_hostName).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss"));
        qDebug()<<"Log file:"<<fname;
        m_file->setFileName(fname);
//        if(!m_file->exists()){

//        }
        if(!m_file->open(QIODevice::WriteOnly | QIODevice::Text)){
            qDebug()<<"File open error, name = "<<fname;
            m_file->close();
        }
        QTextStream ts(m_file);
        if(m_file->size() == 0){
            ts<<"Date,Time,X-P,Y-P,Z-P,X-R,Y-R,Z-R,X-C,Y-C,Z-C,X-V,Y-V,Z-V\n";
        }
    }
    else{
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

void snode_interface::enableFFT(bool v)
{
    switch(m_codecID){
    case 1:
        ((snode_vnode_codec*)m_codec)->enableFFT(v);
        break;
    }

}

//void snode_interface::writeRecord(QList<double> v)
//{

//}

void snode_interface::receiveVNodeData(QList<float> v)
{
    qDebug()<<Q_FUNC_INFO<<m_log<<(m_file == nullptr);
    if(m_log && (m_file !=nullptr)){
        QTextStream ts(m_file);
        QDateTime dt = QDateTime::currentDateTime();
        ts << dt.toString("yyyy-MM-dd,hh:mm:ss.zzz");
        for(int i=0;i<12;i++){
            ts << ","<<QString::number(v[i]);
        }
        ts << "\n";
    }

}

void snode_interface::receiveStreamData(QByteArray b)
{

}

/* snode_interface_model */

snode_interface_model::snode_interface_model(QObject *parent)
    :QAbstractTableModel (parent)
{


}

snode_interface_model::~snode_interface_model()
{
    qDebug()<<Q_FUNC_INFO;
    foreach(snode_interface *d,m_data){
        delete d;
        d = nullptr;
    }
}
int snode_interface_model::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int snode_interface_model::columnCount(const QModelIndex &parent) const
{
    return 13;
}

QVariant snode_interface_model::data(const QModelIndex &index, int role) const
{
    switch(role){
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()){
        case 2:
            return m_data.at(index.row())->hostName();
            break;
        case 3:
            return m_data.at(index.row())->portNumber();
            break;
        case 4:
            return m_data.at(index.row())->devName();
        case 12:
            return m_data.at(index.row())->pidInfo();
        default:
            return QVariant();
            break;
        }
        break;
    case Qt::FontRole:
        break;
    case Qt::BackgroundRole:
        break;
    case Qt::TextAlignmentRole:
        break;
    case Qt::CheckStateRole:
        break;
    }

    return QVariant();
}

QVariant snode_interface_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
        switch(section){
        case 0:
            return QString("Action");
        case 1:
            return QString("Type");
        case 2:
            return QString("Host");
        case 3:
            return QString("Port");
        case 4:
            return QString("Model");
        case 5:
            return QString("Config");
        case 6:
            return QString("Get");
        case 7:
            return QString("Set");
        case 8:
            return QString("Operation");
        case 9:
            return QString("Chart");
        case 10:
            return QString("Log");
        case 11:
            return QString("FFT");
        case 12:
            return QString("PID");
        }
    }
    return QVariant();
}

bool snode_interface_model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::EditRole){
        if(!checkIndex(index))
            return false;

        int r = index.row();
        int c = index.column();
        if(r >= m_data.size()) return false;
        switch(c){
        case 1:{
            snode_interface::commType t = static_cast<snode_interface::commType>(value.toInt());
            m_data.at(r)->setType(t);
        }break;
        case 2:{
            QString s = value.toString();
            m_data.at(r)->setHostName(s);
        }break;
        case 3:{
            int p = value.toInt();
            m_data.at(r)->setPortNumber(p);
        }break;
        }
    }
    return true;
}

Qt::ItemFlags snode_interface_model::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool snode_interface_model::insertRows(int row, int count, const QModelIndex &parent)
{
    snode_interface *n = new snode_interface;

    beginInsertRows(parent,row,row+count-1);

    m_data.append(n);
    endInsertRows();
    return true;
}

bool snode_interface_model::removeRows(int row, int count, const QModelIndex &parent)
{
    if(row >= m_data.size()) return false;
    if(row < 0) return false;
    beginRemoveRows(parent,row,row+count-1);
    m_data.removeAt(row);
    endRemoveRows();
    return true;
}

snode_interface *snode_interface_model::getItem(QString name)
{
    snode_interface *ret = nullptr;
    for(int i=0;i<m_data.size();i++)
    {
        if(m_data.at(i)->hostName() == name){
            ret = m_data.at(i);
            break;
        }
    }
    return ret;
}

snode_interface *snode_interface_model::getItem(int id)
{
    snode_interface *ret = nullptr;
    if(id < m_data.size()){
        ret = m_data.at(id);
    }
    return ret;
}

int snode_interface_model::getItemId(QString name)
{
    for(int i=0;i<m_data.size();i++)
    {
        if(m_data.at(i)->hostName() == name){
            return i;
            break;
        }
    }
    return -1;

}

/* snode simulator




*/

snode_simulator::snode_simulator(QObject *parent):snode_interface (parent)
{
    m_cmdHandler = new cmd_handler(nullptr,true);
    m_hostName = "COM2";
    setCodec(0);
    setCurrSetting(0);
}


bool snode_simulator::open()
{
    qDebug()<<Q_FUNC_INFO;
    bool ret = false;
    if(m_commType == COMM_SERIAL){
        log(LOG_INFO,QString("Connecto to port %1, baudrate=%2").arg(m_hostName).arg(m_portNum));

        if(m_port!=nullptr && m_port->isOpen()){
            m_port->close();
            m_port->deleteLater();
            m_port = nullptr;
        }else{
            qDebug()<<"Open serial port"<<m_hostName<<m_portNum;
            log(LOG_INFO,QString("Connecto to port %1, baudrate=%2").arg(m_hostName).arg(m_portNum));
            if(m_port){
                m_port->close();
                m_port->deleteLater();
                m_port = nullptr;
            }
            m_port = new QSerialPort();
            m_port->setPortName(m_hostName);
            m_port->setBaudRate(m_portNum);
            m_port->setParity(QSerialPort::NoParity);
            m_port->setDataBits(QSerialPort::Data8);
            m_port->setStopBits(QSerialPort::OneStop);
            m_port->setFlowControl(QSerialPort::NoFlowControl);
            if(m_port->open(QIODevice::ReadWrite)){
                ret = true;
                connect(m_port,&QSerialPort::readyRead,this,&snode_simulator::handleIncomingData);
                connect(m_cmdHandler,&cmd_handler::send_command,this,&snode_simulator::writeCommand);
                //connect(m_cmdHandler,&cmd_handler::dataReady,this,&snode_simulator::on_data_out);
                QString msg = Q_FUNC_INFO;
                switch(m_codecID){
                case 0:
                    //m_codec = new snode_codec(this);
                    msg += m_codec->metaObject()->className();
                    emit sendLog(msg);
                    connect(m_codec,&snode_codec::send_log,this,&snode_interface::on_codec_message);
                    connect(this,&snode_simulator::newData,m_codec,&snode_codec::on_encoder_received);
                    connect(m_codec,&snode_codec::write_data,this,&snode_interface::setCommand);
//                    connect(m_codec,&snode_codec::send_resp,m_cmdHandler,&cmd_handler::add_cmd,Qt::DirectConnection);
                    break;
                case 1:
                    //m_codec = new snode_vnode_codec;
                    msg += m_codec->metaObject()->className();
                    emit sendLog(msg);
                    connect(m_codec,&snode_vnode_codec::send_log,this,&snode_interface::on_codec_message);
                    connect(this,&snode_simulator::newData,(snode_vnode_codec*)m_codec,&snode_vnode_codec::on_encoder_received);
                    connect(m_codec,&snode_vnode_codec::write_data,this,&snode_interface::setCommand);
//                    connect(m_codec,&snode_vnode_codec::send_resp,m_cmdHandler,&cmd_handler::add_cmd,Qt::DirectConnection);
                    connect(m_cmdHandler,&cmd_handler::generate_packet,(snode_vnode_codec*)m_codec,&snode_vnode_codec::generateRecord,Qt::DirectConnection);
                    connect((snode_vnode_codec*)m_codec,&snode_vnode_codec::send_command,this,&snode_simulator::on_encoder_command);
                    break;
                }
                m_cmdHandler->start();
                //log(LOG_INFO,"Connection OK, start handler");
                //m_port->write("Hello\n");
            }else{
                log(LOG_ERROR,QString("Connection failed,err = %1").arg(m_port->errorString()));
            }
        }
    }
    else if(m_commType == COMM_SOCKET){
        log(LOG_INFO,QString("Connecto to ip %1, port=%2").arg(m_hostName).arg(m_portNum));
    }
    return ret;
}

bool snode_simulator::close()
{
    if(m_commType == COMM_SERIAL){
        log(LOG_INFO,QString("Close port %1").arg(m_hostName));
        if(m_port){
            m_port->close();
            m_port->deleteLater();
            m_port = nullptr;
        }
        if(m_codec != nullptr){
            m_codec->deleteLater();
            m_codec = nullptr;
        }
//        if(m_cmdHandler){
//            m_cmdHandler->stop();
//            m_cmdHandler->deleteLater();
//            m_cmdHandler = nullptr;
//        }
    }
    return true;
}

void snode_simulator::setCodec(int index)
{
    m_codecID = index;
    qDebug()<<Q_FUNC_INFO;
    //snode_codec *codec = nullptr;
    QString msg = Q_FUNC_INFO;
    if(m_codec != nullptr){
        m_codec->deleteLater();
        m_codec = nullptr;
    }
    switch(index){
    case 0:

        m_codec = new snode_codec;
        connect(m_codec,&snode_codec::write_data,this,&snode_simulator::setCommand);
        connect(m_codec,&snode_codec::send_log,this,&snode_simulator::on_codec_message);
        connect(m_codec,&snode_codec::setupReceived,this,&snode_simulator::on_codec_setup_received);
        connect(this,&snode_simulator::newData,m_codec,&snode_codec::on_decoder_received);

//                connect(m_codec,&snode_codec::write_data,m_cmdHandler,&cmd_handler::add_cmd,Qt::DirectConnection);
        break;
    case 1:
        m_codec = new snode_vnode_codec;
        connect(m_codec,&snode_vnode_codec::write_data,this,&snode_simulator::setCommand);
        connect(m_codec,&snode_vnode_codec::send_log,this,&snode_simulator::on_codec_message);
        connect(m_codec,&snode_vnode_codec::setupReceived,this,&snode_simulator::on_codec_setup_received);
        connect(this,&snode_simulator::newData,(snode_vnode_codec*)m_codec,&snode_vnode_codec::on_decoder_received);
//                connect(m_codec,&snode_vnode_codec::write_data,m_cmdHandler,&cmd_handler::add_cmd,Qt::DirectConnection);
        break;
    }

}


void snode_simulator::handleIncomingData()
{
    QByteArray recv;
    if(m_commType == COMM_SERIAL){
        recv = m_port->readAll();
//        if(m_parser != nullptr)
//            m_parser->parse_command(recv);
    }
    if(recv.size()){
//        m_codec->on_decoder_received(recv);
        qDebug()<<Q_FUNC_INFO;
        emit newData(recv);
        //m_cmdHandler.set_responsed(true);
    }
}

void snode_simulator::start()
{
//    m_stop = false;

    m_cmdHandler->start(10,10);
}

void snode_simulator::stop()
{
//    m_stop = true;
    m_cmdHandler->stop();
}

void snode_simulator::on_data_out(QByteArray b)
{
    if(m_commType == COMM_SERIAL){
        log(LOG_INFO,QString("Send data on port %1").arg(m_hostName));
        if(m_port){
            m_port->write(b);
        }
    }
}

void snode_simulator::on_encoder_command(quint8 cmd)
{
    qDebug()<<Q_FUNC_INFO<<cmd;
    QVector<double> p = ((snode_vnode_codec*)m_codec)->sampleParams();
    switch(cmd){
    case COMM_IF_CMD_STOP:
        m_cmdHandler->stop();
        break;
    case COMM_IF_CMD_START:
        m_cmdHandler->start(p.at(0),p.at(1));
        break;
    }
}

/* snode_simulator_model */

snode_simulator_model::snode_simulator_model(QObject *parent):QAbstractTableModel (parent){

}

int snode_simulator_model::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int snode_simulator_model::columnCount(const QModelIndex &parent) const
{
    return 5;
}

QVariant snode_simulator_model::data(const QModelIndex &index, int role) const
{
    switch(role){
    case Qt::DisplayRole:
        switch(index.column()){
        case 1:
            return m_data.at(index.row())->hostName();
            break;
        case 2:
            return m_data.at(index.row())->portNumber();
            break;
        case 3:
            return m_data.at(index.row())->devName();
            break;
        case 4:
            return QVariant();
            break;
        }
    }
    return  QVariant();
}

QVariant snode_simulator_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
        switch(section){
        case 0:
            return QString("Interface Type");
        case 1:
            return QString("Host/Port Name");
        case 2:
            return QString("Port/Baudrate");
        case 3:
            return QString("Model");
        case 4:
            return QString("Open/Close");
        }
    }
    return QVariant();
}

bool snode_simulator_model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug()<<Q_FUNC_INFO;
    if(role == Qt::EditRole){
        if(!checkIndex(index))
            return false;

        int r = index.row();
        int c = index.column();
        if(r >= m_data.size()) return false;
        switch(c){
        case 0:{
            snode_interface::commType t = static_cast<snode_interface::commType>(value.toInt());
            m_data.at(r)->setType(t);
        }break;
        case 1:{
            QString s = value.toString();
            m_data.at(r)->setHostName(s);
        }break;
        case 2:{
            int p = value.toInt();
            m_data.at(r)->setPortNumber(p);
        }break;
        }
    }
    return true;

}

Qt::ItemFlags snode_simulator_model::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool snode_simulator_model::insertRows(int row, int count, const QModelIndex &parent)
{
    snode_simulator *n = new snode_simulator;

    beginInsertRows(parent,row,row+count-1);
    m_data.append(n);
    endInsertRows();
    return true;
}

bool snode_simulator_model::removeRows(int row, int count, const QModelIndex &parent)
{
    if(row >= m_data.size()) return false;
    beginRemoveRows(parent,row,row+count-1);
    m_data.removeAt(row);
    endRemoveRows();
    return true;
}

snode_simulator* snode_simulator_model::getItem(QString name)
{
    snode_simulator *ret = nullptr;
    for(int i=0;i<m_data.size();i++)
    {
        if(m_data.at(i)->hostName() == name){
            ret = m_data.at(i);
            break;
        }
    }
    return ret;
}

snode_simulator* snode_simulator_model::getItem(int id)
{
    snode_simulator *ret = nullptr;
    if(id < m_data.size()){
        ret = m_data.at(id);
    }
    return ret;
}
