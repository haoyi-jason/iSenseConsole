#include "snode_data_parser.h"
#include "kiss_fft.h"
//snode_data_parser::snode_data_parser(QObject *parent) : QObject(parent)
//{

//}



/*  snode_codec */

snode_codec::snode_codec(QObject *parent):QObject(parent)
{
    m_genRecords = 10;
    memcpy(m_moduleSetting.name, "NONAME\0",11);
    memcpy(m_moduleSetting.user, "NODATA\0",10);
    m_moduleSetting.verNum = 0x01000000;
    m_moduleSetting.serialNum = 0x12345678;



    memcpy(m_wlanSetting.prefix1,"AP_PREFIX\0",10);
    memcpy(m_wlanSetting.prefix2,"STA_SSID\0",9);
    memcpy(m_wlanSetting.passwd1,"PASSWORD\0",9);
    memcpy(m_wlanSetting.passwd2,"PASSWORD\0",9);
    m_wlanSetting.secType = 2;
    m_wlanSetting.connectionTimeout = 60;
    m_wlanSetting.wlan_mode = 0;

//    m_supportConfig.insert("MODULE",0x40);
//    m_supportConfig.insert("SERIAL",0x41);
//    m_supportConfig.insert("LAN",0x42);
//    m_supportConfig.insert("WLAN",0x43);

}

snode_codec::~snode_codec()
{

}



int snode_codec::parse_command(QByteArray b)
{

}


void snode_codec::on_decoder_received(QByteArray b)
{
    m_incomingData.append(b);
    decodeData();
}
bool snode_codec::decodeData()
{
    QString msg = Q_FUNC_INFO;
    qDebug()<<Q_FUNC_INFO<<m_incomingData.size();
    if(m_incomingData.size() < HEADER_SIZE) return false;

    cmd_header_t header;
    //QString tmp;
    memcpy((char*)&header,m_incomingData.constData(),HEADER_SIZE);
    if((header.magic1 == MAGIC1) && header.magic2 == MAGIC2){
        qDebug()<<"Parsing data";
            // parse data
        quint8 mask = header.type;// & 0xf0;
        if(mask == (MASK_CMD | CMD_SETUP)){
            QByteArray b = m_incomingData.mid(HEADER_SIZE,header.len-HEADER_SIZE);
            quint8 pid = header.pid & 0x7f;
            switch(pid){
            case PARAM_USER_DATA:
                msg += "USER DATA";
                 m_userData = b;
                m_incomingData.remove(0,header.len);
                emit setupReceived(pid);
                break;
            case PARAM_MODULE_CONFIG:
                msg += "Module Config";
                //b.append((char*)&m_moduleSetting,sizeof(m_moduleSetting));
                memcpy((char*)&m_moduleSetting,b.constData(),b.size());
                m_incomingData.remove(0,header.len);
                emit setupReceived(pid);
                emit modelNameUpdate(QString((char*)m_moduleSetting.name));
                break;
            case PARAM_MODULE_SERIAL:
                msg += "Serial config";
//                b.append((char*)&m_serialSetting,sizeof(m_serialSetting));
                memcpy((char*)&m_serialSetting,b.constData(),b.size());
                m_incomingData.remove(0,header.len);
                emit setupReceived(pid);
                break;
            case PARAM_MODULE_LAN:
                msg += "Lan config";
//                b.append((char*)&m_lanSetting,sizeof(m_lanSetting));
                memcpy((char*)&m_lanSetting,b.constData(),b.size());
                m_incomingData.remove(0,header.len);
                emit setupReceived(pid);
                break;
            case PARAM_MODULE_WLAN:
                msg += "Wlan config";
//                b.append((char*)&m_wlanSetting,sizeof(m_wlanSetting));
                memcpy((char*)&m_wlanSetting,b.constData(),b.size());
                m_incomingData.remove(0,header.len);
                emit setupReceived(pid);
                break;
            }
            emit send_log(msg);
        }
    }
    return false;
}

void snode_codec::on_encoder_received(QByteArray b)
{
    qDebug()<<Q_FUNC_INFO;

    emit send_log(Q_FUNC_INFO);
    m_incomingData.append(b);
    if(m_incomingData.size() < HEADER_SIZE) return;

    cmd_header_t header,h_resp;
    QByteArray resp;

    memcpy((char*)&header,m_incomingData.constData(),HEADER_SIZE);
    h_resp = header;
    if((header.magic1 == MAGIC1) && header.magic2 == MAGIC2){
        if(m_incomingData.size() >= header.len){
            // parse data
            emit send_log("Parsing data...");
            quint8 mask = header.type & 0xf0;
            if(mask == MASK_CMD){
                if(header.len == HEADER_SIZE){
                    QByteArray b;
                    quint8 pid = header.pid & 0x7f;
                    switch(pid){
                    case PARAM_USER_DATA:
                        emit send_log("get user data");
                        b.append(m_userData);
                        h_resp.len = HEADER_SIZE + b.size();
                        h_resp.crc = checksum(b,b.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
                        b.insert(0,(char*)&h_resp,HEADER_SIZE);
                        m_incomingData.remove(0,header.len);
                        emit write_data(b);
                        break;
                    case PARAM_MODULE_CONFIG:
                        emit send_log("get module config");
                        b.append((char*)&m_moduleSetting,sizeof(m_moduleSetting));
                        m_incomingData.remove(0,header.len);
                        h_resp.len = HEADER_SIZE + b.size();
                        h_resp.crc = checksum(b,b.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
                        b.insert(0,(char*)&h_resp,HEADER_SIZE);
                        m_incomingData.remove(0,header.len);
                        emit write_data(b);
                        break;
                    case PARAM_MODULE_SERIAL:
                        emit send_log("get serial config");
                        b.append((char*)&m_serialSetting,sizeof(m_serialSetting));
                        m_incomingData.remove(0,header.len);
                        h_resp.len = HEADER_SIZE + b.size();
                        h_resp.crc = checksum(b,b.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
                        b.insert(0,(char*)&h_resp,HEADER_SIZE);
                        emit write_data(b);
                        break;
                    case PARAM_MODULE_LAN:
                        emit send_log("get lan config");
                        b.append((char*)&m_lanSetting,sizeof(m_lanSetting));
                        m_incomingData.remove(0,header.len);
                        h_resp.len = HEADER_SIZE + b.size();
                        h_resp.crc = checksum(b,b.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
                        b.insert(0,(char*)&h_resp,HEADER_SIZE);
                        emit write_data(b);
                        break;
                    case PARAM_MODULE_WLAN:
                        emit send_log("get wlan config");
                        b.append((char*)&m_wlanSetting,sizeof(m_wlanSetting));
                        m_incomingData.remove(0,header.len);
                        h_resp.len = HEADER_SIZE + b.size();
                        h_resp.crc = checksum(b,b.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
                        b.insert(0,(char*)&h_resp,HEADER_SIZE);
                        emit write_data(b);
                        break;
                    }
                }else{
                    send_log("set parameters");
                    QByteArray b = m_incomingData.mid(HEADER_SIZE,header.len-HEADER_SIZE);
                    switch(header.pid){
                    case PARAM_USER_DATA:
                        m_userData = b;
                        m_incomingData.remove(0,header.len);
                        break;
                    case PARAM_MODULE_CONFIG:
                        memcpy((char*)&m_moduleSetting,b.constData(),b.size());
                        m_incomingData.remove(0,header.len);
                        break;
                    case PARAM_MODULE_SERIAL:
                        memcpy((char*)&m_serialSetting,b.constData(),b.size());
                        m_incomingData.remove(0,header.len);
                        break;
                    case PARAM_MODULE_LAN:
                        memcpy((char*)&m_lanSetting,b.constData(),b.size());
                        m_incomingData.remove(0,header.len);
                        break;
                    case PARAM_MODULE_WLAN:
                        memcpy((char*)&m_wlanSetting,b.constData(),b.size());
                        m_incomingData.remove(0,header.len);
                        break;
                    }
                }
            }
        }
    }
}

QByteArray snode_codec::getParam(int id)
{
    qDebug()<<Q_FUNC_INFO<<id;
    QJsonParseError e;
    QJsonObject obj;
    QByteArray ba;
    switch(id){
    case PARAM_MODULE_CONFIG:
        obj.insert("FLAG",QString("0x%1").arg(m_moduleSetting.flag,8,16,QChar('0')));
        obj.insert("VERSION",QString("%1").arg(m_moduleSetting.verNum,8,16,QChar('0')));
        obj.insert("SERIAL",QString("%1").arg(m_moduleSetting.serialNum,8,16,QChar('0')));
        obj.insert("VENDER",QString::fromUtf8((char*)m_moduleSetting.vender));
        obj.insert("USER",QString::fromUtf8((char*)m_moduleSetting.user));
        obj.insert("NAME",QString::fromUtf8((char*)m_moduleSetting.name));
        break;
    case PARAM_MODULE_SERIAL:
        obj.insert("Address",QString("%1").arg(m_serialSetting.slave_address,3,10,QChar('0')));
        obj.insert("Baudrate",QString("%1").arg(m_serialSetting.baudrate_val,5,10));
        obj.insert("Address",QString("%1").arg(m_serialSetting.slave_address,3,10));
        break;
    case PARAM_MODULE_LAN:
        break;
    case PARAM_MODULE_WLAN:
        obj.insert("WLAN MODE",m_wlanSetting.wlan_mode==0?"AP":"STA");
        obj.insert("AP PREFIX",QString::fromUtf8((char*)m_wlanSetting.prefix1));
        obj.insert("AP PASSWD",QString::fromUtf8((char*)m_wlanSetting.passwd1));
        obj.insert("STA SSID",QString::fromUtf8((char*)m_wlanSetting.prefix2));
        obj.insert("STA PASSWD",QString::fromUtf8((char*)m_wlanSetting.passwd2));
        obj.insert("Connect Timeout",QString("%1").arg(m_wlanSetting.connectionTimeout));
        obj.insert("SEC Type",QString("%1").arg(m_wlanSetting.secType));
        break;
    case PARAM_USER_DATA:
        obj.insert("USER STORAGE",QString::fromUtf8(m_userData));
        break;
    }
    QJsonDocument d(obj);
    return d.toJson();
}

QByteArray snode_codec::getParam(QString name)
{
    return getParam(config_map.value(name));
}

void snode_codec::setParam(QString name, QByteArray json)
{
    setParam(config_map.value(name),json);
}

void snode_codec::setParam(int id, QByteArray json)
{
    qDebug()<<Q_FUNC_INFO<<id;
    QJsonParseError e;
    QJsonDocument d = QJsonDocument::fromJson(json,&e);
    if(d.isNull()){
        qDebug()<<"JSON error:"<<e.errorString();
        return;
    }
    QJsonObject obj = d.object();

    QString r;
    QByteArray ba;
    switch(id){
    case PARAM_MODULE_CONFIG:
        if(obj.contains("FLAG")){
            ba = QByteArray::fromHex(obj.value("FLAG").toString().toUtf8());
            //qDebug()<<"Module setting:"<<ba<<ba.constData();
            memcpy(&m_moduleSetting.flag,ba.constData(),4);

        }
        if(obj.contains("VERSION")){
            ba = QByteArray::fromHex(obj.value("VERSION").toString().toUtf8());
            memcpy(&m_moduleSetting.verNum,ba.constData(),4);
        }
        if(obj.contains("SERIAL")){
            ba = QByteArray::fromHex(obj.value("SERIAL").toString().toUtf8());
            memcpy(&m_moduleSetting.serialNum,ba.constData(),4);
        }
        if(obj.contains("NAME")){
            ba = obj.value("NAME").toString().toLatin1();
            memcpy(m_moduleSetting.name,ba.constData(),ba.size()>32?32:ba.size());
        }
        if(obj.contains("VENDER")){
            ba = obj.value("VENDER").toString().toLatin1();
            memcpy(m_moduleSetting.vender,ba.constData(),ba.size()>32?32:ba.size());
        }
        if(obj.contains("USER")){
            ba = obj.value("USER").toString().toLatin1();
            memcpy(m_moduleSetting.user,ba.data(),ba.count()>32?32:ba.size());
        }
        break;
    case PARAM_MODULE_SERIAL:
        break;
    case PARAM_MODULE_LAN:
        break;
    case PARAM_MODULE_WLAN:
        if(obj.contains("WLAN MODE")){
            QString v = obj.value("WLAN MODE").toString();
            m_wlanSetting.wlan_mode = (v == "AP")?0:1;
        }
        if(obj.contains("AP PREFIX")){
            ba = obj.value("AP PREFIX").toString().toLatin1();
            memcpy(m_wlanSetting.prefix1,ba.constData(),ba.size()>16?16:ba.size());
        }
        if(obj.contains("AP PASSWD")){
            ba = obj.value("AP PASSWD").toString().toLatin1();
            memcpy(m_wlanSetting.passwd1,ba.constData(),ba.size()>16?16:ba.size());
        }

        if(obj.contains("STA SSID")){
            ba = obj.value("STA SSID").toString().toLatin1();
            memcpy(m_wlanSetting.passwd1,ba.constData(),ba.size()>16?16:ba.size());
        }
        if(obj.contains("STA PASSWD")){
            ba = obj.value("STA PASSWD").toString().toLatin1();
            memcpy(m_wlanSetting.passwd2,ba.constData(),ba.size()>16?16:ba.size());
        }
        break;
    case PARAM_USER_DATA:
        if(obj.contains("USER STORAGE")){
            m_userData = obj.value("USER STORAGE").toString().toLatin1();
            //memcpy(m_moduleSetting.user,ba.data(),ba.count()>32?32:ba.size());
        }
        break;
    }
}

//QStringList snode_codec::configList()
//{
//    qDebug()<<Q_FUNC_INFO;
//    return QStringList(m_supportConfig.keys());
//}


quint16 snode_codec::checksum(const quint8 *data, quint16 len)
{
    quint16 chksum = 0;
    for(int i=0;i<len;i++){
        if(i ==6)continue;
        if(i==7)continue;
        chksum+=data[i];
    }

    return chksum;
}

quint16 snode_codec::checksum(QByteArray b, int len)
{
    quint16 chksum = 0;
    for(int i=0;i<len;i++){
        chksum+=static_cast<quint8>(b.at(i));
    }

    return chksum;
}

void snode_codec::issue_param_read(quint8 cmd)
{
    send_log(Q_FUNC_INFO);
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_SETUP;
    header.pid = (quint8)cmd;
    header.len = HEADER_SIZE;

    header.crc = checksum((quint8*)&header,HEADER_SIZE);
    QByteArray b;
    b.append((char*)&header,header.len);

    m_bWaitResponse = true;
    emit write_data(b);
}

void snode_codec::issue_param_read(QString name)
{
    issue_param_read(config_map.value(name));
}

void snode_codec::issue_param_write(quint8 cmd)
{
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_SETUP;
    header.pid = (quint8)cmd;

    QByteArray b;
    bool sendcmd = false;
    int sz = 0;
    //b.append((char*)&header,HEADER_SIZE);
    switch(cmd){
        case PARAM_USER_DATA:
        header.len = HEADER_SIZE+128;
        b.append((char*)m_userData.constData());
        sendcmd = true;
        break;
    case PARAM_MODULE_CONFIG:
        sz = sizeof(module_setting_t);
        header.len = HEADER_SIZE+sz;
        //b = QByteArray((char*)&m_moduleSetting,sz);
        b.append((char*)&m_moduleSetting,sz);
        sendcmd = true;
        break;
    case PARAM_MODULE_SERIAL:
        sz = sizeof(serial_setting_t);
        header.len = HEADER_SIZE+sizeof (serial_setting_t);
        b.append((char*)&m_serialSetting);
        sendcmd = true;
        break;
    case PARAM_MODULE_LAN:
        sz = sizeof(lan_setting_t);
        header.len = HEADER_SIZE+sizeof(lan_setting_t);
        b.append((char*)&m_lanSetting);
        sendcmd = true;
        break;
    case PARAM_MODULE_WLAN:
        sz = sizeof(wireless_param_t);
        header.len = HEADER_SIZE+sizeof (wireless_param_t);
        b.append((char*)&m_wlanSetting);
        sendcmd = true;
        break;
    }

    header.crc = checksum((quint8*)&header,HEADER_SIZE);
    //qDebug()<<"CRC:"<<header.crc;
    header.crc += checksum(b,b.size());
    //qDebug()<<"CRC:"<<header.crc;
    b.insert(0,(char*)&header,HEADER_SIZE);
    emit write_data(b);
//    if(sendcmd){
//        m_bWaitResponse = true;
//        emit write_data(b);
//    }
}

void snode_codec::issue_param_write(QString name)
{
    issue_param_write(config_map.value(name));
}

void snode_codec::issue_command(quint8 cmd)
{
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_SETUP;
    header.pid = (quint8)cmd;

    QByteArray b;
    bool sendcmd = false;
    b.append((char*)&header,HEADER_SIZE);
    header.crc = checksum((quint8*)&header,HEADER_SIZE);
    emit write_data(b);
//    if(sendcmd){
//        m_bWaitResponse = true;
//    }

}

void snode_codec::issue_active(bool act)
{
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_CONTROL;
    header.pid = act?CONTROL_START:CONTROL_STOP;
    header.len = HEADER_SIZE;
    header.crc = checksum((quint8*)&header,HEADER_SIZE);

    QByteArray b;
    bool sendcmd = false;
    b.append((char*)&header,HEADER_SIZE);
    emit write_data(b);

    if(act){
        m_incomingData.clear();
        m_expPID = 0;
        pidIn = pidExp = 0;
    }
}

void snode_codec::issue_param_save()
{
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_SETUP;
    header.pid = CMD_SETUP_SAVE;
    header.len = HEADER_SIZE;
    header.crc = checksum((quint8*)&header,HEADER_SIZE);

    QByteArray b;
    bool sendcmd = false;
    b.append((char*)&header,HEADER_SIZE);
    emit write_data(b);

}

/** VNODE  */

snode_vnode_codec::snode_vnode_codec(QObject *parent):snode_codec (parent)
{
    //qDebug()<<Q_FUNC_INFO;
    memcpy(m_moduleSetting.name, "SNODE_VNODE\0",12);
    m_timeDomainParam.sampleNumber = 100;
    m_timeDomainParam.samplePeriodMs = 1000;

    m_nodeParam.opMode = 0;
    m_nodeParam.commType = 0;

    m_adxl.outputrate = 0x2;
    m_adxl.fullscale = 0x1;
    m_adxl.highpassfilter = 0;

    m_adxlScale = 0.0000039*9.81;

    m_freqDomainParam.bins = 512;
    m_freqDomainParam.window = 0;
    m_freqDomainParam.overlap = 0;

    //qDebug()<<"code of AP"<<opmode_map.value("AP");
    m_genFFT = false;
}

QVector<double> snode_vnode_codec::sampleParams()
{
    QVector<double> ret;
    ret.append(m_timeDomainParam.sampleNumber);
    ret.append(m_timeDomainParam.samplePeriodMs);
    qDebug()<<Q_FUNC_INFO<< "Size="<<ret.size();
    return ret;
}
void snode_vnode_codec::issue_param_read(QString name)
{
    snode_codec::issue_param_read(config_map.value(name));
}

void snode_vnode_codec::issue_param_write(int cmd)
{
    cmd_header_t header;
    header.magic1 = MAGIC1;
    header.magic2 = MAGIC2;
    header.type = MASK_CMD | CMD_SETUP;
    header.pid = (quint8)cmd;

    QByteArray b;
    //b.append((char*)&header,HEADER_SIZE);
    int sz = 0;
    switch(cmd){
    case PARAM_SENSOR_BASE:
        sz = sizeof(node_param_t);
        header.len = HEADER_SIZE+sz;
        b.append((char*)&m_nodeParam,sz);
        header.crc = checksum((quint8*)&header,HEADER_SIZE) + checksum(b,b.size());
        b.insert(0,(char*)&header,HEADER_SIZE);
        emit write_data(b);
        break;
    case PARAM_SENSOR_BASE_P1:
        sz = sizeof(adxl355_config_t);
        header.len = HEADER_SIZE+sz;
        b = QByteArray((char*)&m_adxl,sz);
        header.crc = checksum((quint8*)&header,HEADER_SIZE) + checksum(b,b.size());
        b.insert(0,(char*)&header,HEADER_SIZE);
        emit write_data(b);
        qDebug()<<"ADXL:"<<m_adxl.outputrate;
        break;
    case PARAM_SENSOR_BASE_P2:
        sz = sizeof(time_domain_param_t);
        header.len = HEADER_SIZE+sz;
        b.append((char*)&m_timeDomainParam,sz);
        header.crc = checksum((quint8*)&header,HEADER_SIZE) + checksum(b,b.size());
        b.insert(0,(char*)&header,HEADER_SIZE);
        emit write_data(b);
        break;
    case PARAM_SENSOR_BASE_P3:
        sz = sizeof(freq_domain_param_t);
        header.len = HEADER_SIZE+sz;
        b.append((char*)&m_freqDomainParam,sz);
        header.crc = checksum((quint8*)&header,HEADER_SIZE) + checksum(b,b.size());
        b.insert(0,(char*)&header,HEADER_SIZE);
        emit write_data(b);
        break;
    default:
        snode_codec::issue_param_write(cmd);
        break;
    }

}

void snode_vnode_codec::issue_param_write(QString name)
{
    issue_param_write(config_map.value(name));
}

void snode_vnode_codec::on_decoder_received(QByteArray b)
{
    //qDebug()<<Q_FUNC_INFO<<b.size();
    m_incomingData.append(b);

//    QElapsedTimer t;
//    t.start();
    decodeData();
//    qDebug()<<Q_FUNC_INFO<<"Elapsed:"<<t.elapsed()<<" milliseconds";
//    while(decodeData()){
//        QThread::msleep(50);
//    }
}
bool snode_vnode_codec::decodeData()
{
    QString msg = Q_FUNC_INFO;
    //qDebug()<<Q_FUNC_INFO;
    // call base class for data handling
    if(m_incomingData.size() < HEADER_SIZE) return false;
    cmd_header_t header;
    memcpy((char*)&header,m_incomingData.constData(),HEADER_SIZE);
    if((header.magic1 == MAGIC1) && header.magic2 == MAGIC2){
        //qDebug()<<"Parsing data";
        msg += "Parse Data..";
        quint8 mask = header.type ;//& 0xf0;
        if(mask == (MASK_CMD | CMD_CONTROL)){
            msg += "CONTROL COMMAND=>";
        }
        else if(mask == (MASK_CMD | CMD_SETUP)){
            msg += "Setup=>";
            QByteArray b = m_incomingData.mid(HEADER_SIZE,header.len-HEADER_SIZE);
            quint8 pid = header.pid & 0x7f;
            switch(pid){
            case PARAM_SENSOR_BASE:
                msg += "Node Config";
                memcpy((char*)&m_nodeParam,b.constData(),b.size());
                emit setupReceived(pid);
                m_incomingData.remove(0,header.len);
                break;
            case PARAM_SENSOR_BASE_P1:
                msg += "ADXL Config";
                memcpy((char*)&m_adxl,b.constData(),b.size());
                emit setupReceived(pid);
                m_incomingData.remove(0,header.len);
                break;
            case PARAM_SENSOR_BASE_P2:
                msg += "Time Domain config";
                memcpy((char*)&m_timeDomainParam,b.constData(),b.size());
                emit setupReceived(pid);
                m_incomingData.remove(0,header.len);
                break;
            case PARAM_SENSOR_BASE_P3:
                msg += "Frequency Domain Config";
                memcpy((char*)&m_freqDomainParam,b.constData(),b.size());
                emit setupReceived(pid);
                m_incomingData.remove(0,header.len);
                break;
            default:
                return snode_codec::decodeData();
            }
        }
        else if((mask & MASK_DATA) == MASK_DATA){
            msg += QString("DATA, PID=%1,MODE=%2").arg(header.pid).arg(m_nodeParam.opMode);
            if(m_nodeParam.opMode == 0){
                parseStream();
            }else if(m_nodeParam.opMode == 1){
                parseVNode();
            }
            qDebug()<<QString("IN:%1,EXP:%2").arg(header.pid).arg(m_expPID);
            m_expPID++;
            //emit pidUpdate(header.pid,m_expPID);
        }
        else if(mask == MASK_CMD_RET_OK){
            m_incomingData.remove(0,header.len);
        }
        else if(mask == MASK_CMD_RET_ERR){
            m_incomingData.remove(0,header.len);
        }
        else if(mask == MASK_CMD_RET_BUSY){
            m_incomingData.remove(0,header.len);
        }
    }
    msg += QString("Data remaining=%1 bytes").arg(m_incomingData.size());
    //emit send_log(msg);
    return true;
}

void snode_vnode_codec::parseVNode()
{
    qDebug()<<Q_FUNC_INFO;
    cmd_header_t header;
    memcpy((char*)&header,m_incomingData.constData(),HEADER_SIZE);
    QByteArray b = m_incomingData.mid(HEADER_SIZE,header.len - HEADER_SIZE);
    QDataStream out(&b,QIODevice::ReadOnly);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out.setByteOrder(QDataStream::LittleEndian);
    float v;
    QList<float> rec;
    for(int i=0;i<12;i++){
        out >> v;
        m_result.append(v);
        rec.append(v);
    }
//    emit new_data();
    emit newRecord(rec);
    qDebug()<<"EMIT DATA:"<<rec;
    m_incomingData.remove(0,header.len);
}

void snode_vnode_codec::parseStream()
{
    cmd_header_t header;
    memcpy((char*)&header,m_incomingData.constData(),HEADER_SIZE);
    QByteArray b = m_incomingData.mid(HEADER_SIZE,header.len - HEADER_SIZE);
    m_incomingData.remove(0,header.len);
    //return;
    if(b.size() == 0){
        return;
    }
    QDataStream in(&b,QIODevice::ReadOnly);
    //QByteArray bout;
    //QDataStream out(&bout,QIODevice::WriteOnly);
    //in.setByteOrder(QDataStream::LittleEndian);
    //out.setFloatingPointPrecision(QDataStream::SinglePrecision);

    struct int_3b v3b;
    float v;
    int nofRecord = b.size()/3/3;
   // QByteArray s;
    //qDebug()<<Q_FUNC_INFO<<nofRecord;
    QVector<float> result;
    //result.resize(3);
    for(int i=0;i<nofRecord;i++){
        for(int j=0;j<3;j++){
            in >> v3b;
            //result.append((float)v3b.b.v*0.000038259);
            m_waveResult[j].append((float)v3b.b.v*m_adxlScale);

        }
    }
    //emit new_data();
    //emit newRaw(bout);
    //qDebug()<<Q_FUNC_INFO<<"Emit";
//    if(result.size())
//        emit newSeries(m_result);
    for(int i=0;i<3;i++){
        if(m_waveResult[i].size() > 2048){
            m_waveResult[i].remove(0,m_waveResult[i].size()-2048);
        }
        //qDebug()<<"Sz:"<<m_waveResult[i].size();
    }
    emit newWave(m_waveResult[0],m_waveResult[1],m_waveResult[2]);
    if(fft()){
        genFFT();
    }
}

void snode_vnode_codec::genFFT()
{
    //qDebug()<<Q_FUNC_INFO;
    QVector<float> fftResult[3];
    kiss_fft_cpx *in,*out;
    kiss_fft_cfg p;

    int n = m_freqDomainParam.bins;
    float sampleRate = 4000/(1 << m_adxl.outputrate);
    float deltaF = sampleRate / n;
    //qDebug()<<QString("Rate=%1,delta f=%2").arg(sampleRate).arg(deltaF);
    in = (kiss_fft_cpx*)kiss_fft_alloc(n,0,nullptr,nullptr);
    out = (kiss_fft_cpx*)kiss_fft_alloc(n,0,nullptr,nullptr);
    p = kiss_fft_alloc(n,0,nullptr,nullptr);
    if(!p) return;

    for(int w=0;w<3;w++){
        int sz = m_waveResult[w].size();
        if(sz > n){
            int start = m_waveResult[w].size()-n-1;
            int end = m_waveResult[w].size();
            for(int i=0;i<n;i++){
                in[i].r = m_waveResult[w][i];
                in[i].i = 0;
            }
        }
        else{
            for(int i=0;i<sz;i++){
                in[i].r = m_waveResult[w][i];
                in[i].i = 0;
            }
            for(int i=sz;i<n;i++){
                in[i].r = in[i].i = 0;
            }
        }

        kiss_fft(p,in,out);
        float norm;
        for(int i=0;i<n/2;i++){
            norm = sqrt(out[i].r*out[i].r + out[i].i*out[i].i);
            fftResult[w].append(norm);
        }
    }
    //qDebug()<<"Emit data";
    emit newFFT(fftResult[0],fftResult[1],fftResult[2]);
}

void snode_vnode_codec::on_encoder_received(QByteArray b){
    qDebug()<<Q_FUNC_INFO;
    emit send_log(Q_FUNC_INFO);
    m_incomingData.append(b);
    if(m_incomingData.size() < HEADER_SIZE) return;

    cmd_header_t header,h_resp;
    QByteArray resp;
    memcpy((char*)&header,m_incomingData.constData(),HEADER_SIZE);
    h_resp = header;
    if((header.magic1 == MAGIC1) && (header.magic2 == MAGIC2)){
        if(m_incomingData.size() < header.len) return;
        if((header.pid & 0x80) == 0x00){
        // check crc
            quint16 crc = checksum(m_incomingData,header.len);
            if(crc != header.crc){
                emit send_log("Checksum Error");
                m_incomingData.remove(0,header.len);
                return;
            }
            emit send_log("Checksum OK, Parsing vnode data");
        }
        quint8 mask = header.type;// & 0xf0;
        //emit send_log("VNODE PARSING DATA");
        qDebug()<<"Vnode parsing data, type = "<<header.type;
        if(mask == (MASK_CMD | CMD_CONTROL)){
            qDebug()<<"Receive control command";
            quint8 pid = header.pid & 0x7f;
            emit send_command(pid & 0x7f);
            h_resp.type = MASK_CMD_RET_OK;
            h_resp.pid = 0;
            h_resp.len = HEADER_SIZE;
            h_resp.crc = checksum((quint8*)&h_resp,HEADER_SIZE);
            m_incomingData.remove(0,HEADER_SIZE);
            resp.append((char*)&h_resp,HEADER_SIZE);
            emit write_data(resp);
        }
        else if(mask == (MASK_CMD | CMD_SETUP)){
            quint8 pid = header.pid & 0x7f;
            if(header.len == HEADER_SIZE){
                switch(pid){
                case PARAM_SENSOR_BASE:
                    emit send_log("Get timedomain setting");
                    resp.append((char*)&m_timeDomainParam,sizeof(m_timeDomainParam));
                    h_resp.len = HEADER_SIZE + resp.size();
                    h_resp.crc = checksum(resp,resp.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
                    resp.insert(0,(char*)&h_resp,HEADER_SIZE);
                    m_incomingData.remove(0,header.len);
                    emit write_data(resp);
                    break;
                case PARAM_SENSOR_BASE_P1:
                    emit send_log("Get freqdomain setting");
                    resp.append((char*)&m_freqDomainParam,sizeof(m_freqDomainParam));
                    h_resp.len = HEADER_SIZE + resp.size();
                    h_resp.crc = checksum(resp,resp.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
                    resp.insert(0,(char*)&h_resp,HEADER_SIZE);
                    m_incomingData.remove(0,header.len);
                    emit write_data(resp);
                    break;
                default:
                    snode_codec::on_encoder_received(b);
                    break;
                }
            }else{
                QByteArray b = m_incomingData.mid(HEADER_SIZE,header.len-HEADER_SIZE);
                switch(pid){
                case PARAM_SENSOR_BASE:
                    memcpy((char*)&m_timeDomainParam,b.constData(),header.len - HEADER_SIZE);
                    m_incomingData.remove(0,header.len);
                    break;
                case PARAM_SENSOR_BASE_P1:
                    memcpy((char*)&m_timeDomainParam,b.constData(),header.len - HEADER_SIZE);
                    m_incomingData.remove(0,header.len);
                    break;
                default:
                    snode_codec::on_encoder_received(b);
                    break;
                }
            }
        }
    }
}

void snode_vnode_codec::generateRecord(int nofRecord)
{
    qDebug()<<Q_FUNC_INFO;
    cmd_header_t h_resp;
    h_resp.magic1 = MAGIC1;
    h_resp.magic2 = MAGIC2;
    h_resp.type = MASK_DATA;
    h_resp.pid = 0;

    QByteArray resp;
    resp.resize(48);
    h_resp.crc = checksum(resp,resp.size()) + checksum((quint8*)&h_resp,HEADER_SIZE);
    resp.insert(0,(char*)&h_resp,HEADER_SIZE);
    emit write_data(resp);


}

void snode_vnode_codec::setParam(int id, QByteArray json)
{
    emit send_log(Q_FUNC_INFO);
    QJsonParseError e;
    QJsonDocument d = QJsonDocument::fromJson(json,&e);
    if(d.isNull()){
        return;
    }
    QJsonObject obj = d.object();

    QString r;
    QByteArray ba;
    switch(id){
    case PARAM_SENSOR_BASE:
        if(obj.contains("MODE")){
            m_nodeParam.opMode = opmode_map.value(obj.value("MODE").toString());
        }
        if(obj.contains("COMM")){
            m_nodeParam.commType = obj.value("COMM").toInt();
        }
        break;
    case PARAM_SENSOR_BASE_P1:
        if(obj.contains("RANGE")){
            m_adxl.fullscale = accRange_map.value(obj.value("RANGE").toString());
        }
        if(obj.contains("RATE")){
            qDebug()<<"Set Rate:"<<obj.value("RATE").toString()<<odrRange_map.value(obj.value("RATE").toString());
            m_adxl.outputrate = odrRange_map.value(obj.value("RATE").toString());
        }
        if(obj.contains("HPF")){
            m_adxl.highpassfilter = (obj.value("HPF").toInt());
        }

        break;
    case PARAM_SENSOR_BASE_P2:
        if(obj.contains("SAMP_NUMBER")){
            m_timeDomainParam.sampleNumber = obj.value("SAMP_NUMBER").toInt();
        }
        if(obj.contains("SAMP_PERIOD")){
            m_timeDomainParam.samplePeriodMs = obj.value("SAMP_PERIOD").toInt();
        }
        break;
    case PARAM_SENSOR_BASE_P3:
        if(obj.contains("BINS")){
            m_freqDomainParam.bins = obj.value("BINS").toInt();
        }
        if(obj.contains("WINDOW")){
            m_freqDomainParam.window = fft_window_map.value(obj.value("WINDOW").toString());
        }
        if(obj.contains("OVERLAP")){
            m_freqDomainParam.overlap = obj.value("OVERLAP").toInt();
        }
        break;
    default:
        snode_codec::setParam(id,json);
        break;
    }
}

void snode_vnode_codec::setParam(QString name, QByteArray json)
{
    qDebug()<<Q_FUNC_INFO<<name;
    setParam(config_map.value(name),json);
}

QByteArray snode_vnode_codec::getParam(QString name)
{
    qDebug()<<Q_FUNC_INFO<<name<<config_map.value(name);
    return getParam(config_map.value(name));
}

QByteArray snode_vnode_codec::getParam(int id)
{
    qDebug()<<Q_FUNC_INFO<<id;
    QJsonObject obj;
    QByteArray ba;
    QByteArray ret;
    QJsonDocument d;
    switch(id){
    case PARAM_SENSOR_BASE:
        obj.insert("MODE",opmode_map.key(m_nodeParam.opMode));
        d = QJsonDocument(obj);
        ret = d.toJson();
        break;
    case PARAM_SENSOR_BASE_P1:
        obj.insert("RANGE",accRange_map.key(m_adxl.fullscale));
        obj.insert("RATE",odrRange_map.key(m_adxl.outputrate));
        obj.insert("HPF",(m_adxl.highpassfilter));
        d = QJsonDocument(obj);
        ret = d.toJson();
        switch(m_adxl.fullscale){
        case 0x1:default:m_adxlScale = 9.81*.0000039;break;
        case 0x2:m_adxlScale = 9.81*.0000078;break;
        case 0x3:m_adxlScale = 9.81*.0000156;break;
        }
        break;
    case PARAM_SENSOR_BASE_P2:
        obj.insert("SAMP_NUMBER",m_timeDomainParam.sampleNumber);
        obj.insert("SAMP_PERIOD",m_timeDomainParam.samplePeriodMs);
        d = QJsonDocument(obj);
        ret = d.toJson();
        break;
    case PARAM_SENSOR_BASE_P3:
        obj.insert("BINS",(m_freqDomainParam.bins));
        obj.insert("WINDOW",fft_window_map.key(m_freqDomainParam.window));
        obj.insert("OVERLAP",(m_freqDomainParam.overlap));
        d = QJsonDocument(obj);
        ret = d.toJson();
        break;
    default:
        ret = snode_codec::getParam(id);
        break;
    }
    return ret;
}

//QStringList snode_vnode_codec::configList()
//{
//    qDebug()<<Q_FUNC_INFO;
//    QStringList lst = snode_codec::configList();
//    lst <<"Time"<<"Frequency";
//    return QStringList(m_supportConfig.keys());
//}
