#ifndef SNODE_INTERFACE_H
#define SNODE_INTERFACE_H

#include <QObject>
#include <QtCore>
#include <QThread>
#include <QDataStream>
#include <QDebug>
#include <QtSerialPort>
#include <QTcpSocket>

#include "snode_data_parser.h"
#include "chartview.h"

#define HEADER_SIZE 8
#define MAGIC   0xabba
#define MAGIC1  0xab
#define MAGIC2  0xba

#define MASK_CMD                0xa0
#define MASK_CMD_RET_CFG        0x10
#define MASK_DATA               0x30
#define MASK_CMD_RET_OK         0x50
#define MASK_CMD_RET_ERR        0x70
#define MASK_CMD_RET_BUSY       0x90

#define CMD_CONTROL        0x01
#define CMD_SETUP          0x02
#define CMD_FS           0x03
#define CMD_SYS       0x0C

#define NOCRC_MASK         0x80
#define CONTROL_STOP        0x00
#define CONTROL_START       0x01

#define MODE_SINGLE     0x00
#define MODE_STREAMING  0x01
#define MODE_CHARACTER  0x02
#define MODE_INCLINE    0x03
#define MODE_DISP       0x04

class QSerialPort;
class QTcpSocket;
class QTimer;


class cmd_handler:public QObject{
    Q_OBJECT
public:
    explicit cmd_handler(QObject *parent = nullptr, bool simulator = false):QObject(parent){
        m_wait = m_stop = false;
        tmr = new QTimer(this);
        m_timeoutMS = 5000;
        m_retry = 5;
        moveToThread(&m_workThread);
        if(simulator)
            connect(&m_workThread,&QThread::started,this,&cmd_handler::simulate);
        else
            connect(&m_workThread,&QThread::started,this,&cmd_handler::do_loop);
        m_port = nullptr;
        m_portNum = -1;
    }
    ~cmd_handler(){
        m_stop = true;
        m_workThread.quit();
        m_workThread.wait();
    }
    void clear_command(){
        m_cmdList.clear();
    }

    void set_responsed(bool v){
        tmr->stop();
        if(v){
            emit ret_ok();
            if(m_cmdList.size()){
                mutex.lock();
                m_cmdList.removeFirst();
                mutex.unlock();
            }
        }else{
            emit ret_ng();
        }
        m_wait = false;
//        if(m_cmdList.size()){
//            start();
//        }
    }
    void start(int records_per_packet=10, int period=10){
        m_stop = false;
        m_workThread.start();
        m_nofRecords = records_per_packet;
        m_record_period_ms = period;
    }

    void stop(){m_stop = true;}
    void setPeriod(int period){m_record_period_ms = period;}
    void setHost(int type, QString hostName, int port){
        m_commType = type;m_hostName = hostName;m_portNum = port;
    }
    bool open(){
        bool ret = false;
        if(m_commType == 0){
            if(m_port){
                m_port->close();m_port->deleteLater();m_port = nullptr;
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
            }
        }
        else if(m_commType == 1){

        }
        return ret;
    }

    bool close(){
        if(m_port){
            stop();
            m_port->close();
            m_port->deleteLater();
            m_port = nullptr;
        }
        return true;
    }

public slots:
    void add_cmd(QByteArray cmd){
//        QObject *obj = (QObject*)sender();
        //qDebug()<<Q_FUNC_INFO<<"Sender:"<<QObject::sender();
        mutex.lock();
        m_cmdList.append(cmd);
        mutex.unlock();
        //if(!m_wait) start();
    }

    void addMessage(QByteArray msg){
        qDebug()<<Q_FUNC_INFO<<msg;
    }



private slots:
    void do_loop(){
        qDebug()<<Q_FUNC_INFO<<"Start";
        int cntr = 0;
        int retry = 0;
        QTimer timer;
        QEventLoop loop;
        timer.setSingleShot(true);
        connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
        int readInterval = 1;
        while(!m_stop){
            if(m_wait){
                cntr++;
                if(cntr > 100){
                    m_timeout = true;
                }
                if(m_timeout){
//                    m_cmdList.removeFirst();
                    m_cmdList.clear();
                    retry = 0;
                    cntr = 0;
                    m_wait = false;
                    emit timeout();
                    qDebug()<<"send command timeout";
                }
            }
            else if(m_cmdList.size()>0){
                mutex.lock();
                emit send_command(m_cmdList.first());
                //tmr->start(m_timeoutMS);
                mutex.unlock();

//                if(m_timeout){
//                    retry++;
//                }else{
//                    retry = 0;
//                }
                m_wait = true;
                m_timeout = false;
                cntr = 0;

            }

            //check for serial port
//            if(readInterval){
//                readInterval--;
//            }else{
//                readInterval = 1;
//                if(m_port && m_port->isOpen()){
//                    if(m_port->bytesAvailable()){
//                        emit dataReady(m_port->readAll());
//                    }
//                }
//            }
            timer.start(50);
            loop.exec();
//            QThread::msleep(10);
        }
        qDebug()<<Q_FUNC_INFO<<"Stop";

        m_workThread.quit();
        //this->deleteLater();
    }

    void simulate(){
        qDebug()<<Q_FUNC_INFO<<"Start";
        while(!m_stop){
            if(m_cmdList.size()>0){
                mutex.lock();
                emit send_command(m_cmdList.first());
                m_cmdList.removeFirst();
                mutex.unlock();
            }
            emit generate_packet(m_nofRecords);
            QThread::msleep(m_record_period_ms);
        }
    }
    void handletimeout(){
        tmr->stop();
        m_timeout = true;
        m_wait = false;
    }

signals:
    void send_command(QByteArray b);
    void timeout();
    void send_done();
    void ret_ok();
    void ret_ng();
    void retry_overrun();
    void dataReady(QByteArray b);
    void generate_packet(int);

private:
    QMutex mutex;
    QThread m_workThread;
    bool m_wait;
    bool m_stop;
    bool m_timeout;
    QTimer *tmr;
    QList<QByteArray> m_cmdList;
    int m_timeoutMS;
    int m_retry;
    int m_nofRecords;
    int m_record_period_ms;
    QSerialPort *m_port;
    QString m_hostName;
    int m_portNum;
    int m_commType;
};


class snode_interface : public QObject
{
    Q_OBJECT
public:
    const QMap<QString,quint8> interface_map =
            QMap<QString,quint8>({{"SERIAL",0},{"SOCKET",1}});
    QStringList supportInterface() const{return QStringList(interface_map.keys());}
    QString currentInterface() const{return interface_map.key(m_commType);}
    const QMap<QString,quint8> model_map =
            QMap<QString,quint8>({{"BASE",0},{"SensorNode",1},{"DOORSPEED",2},{"VSS-II",3}});
    QStringList supportModels() const{return QStringList(model_map.keys());}

    enum commType{
        COMM_SERIAL,
        COMM_SOCKET
    };
    Q_ENUM(commType)
    enum logType{
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_CODEC,
    };
    Q_ENUM(logType)
    enum command_control{
      COMM_IF_CMD_STOP,
      COMM_IF_CMD_START,
      COMM_IF_CMD_SINGLE,
      CMMM_IF_CMD_LOAD_PARAM,
      COMM_IF_CMD_SAVE_PARAM,
      COMM_IF_CMD_RESET,
      COMM_IF_ERROR = 0xff,
    };
    Q_ENUM(command_control);

    enum param_id{
        PARAM_SENSOR_BASE = 0x00,
        PARAM_SENSOR_BASE_P1,
        PARAM_SENSOR_BASE_P2,
        PARAM_SENSOR_BASE_P3,
        PARAM_MODULE_CONFIG = 0x40,
        PARAM_MODULE_SERIAL,
        PARAM_MODULE_LAN,
        PARAM_MODULE_WLAN
    };
    Q_ENUM(param_id)
    typedef struct{
        quint8 magic1;
        quint8 magic2;
        quint8 type;
        quint8 pid;
        quint16 len;
        quint16 crc;
    }cmd_header_t;

    typedef struct{
      uint32_t flag;
      uint8_t name[32];
      uint32_t verNum;
      uint32_t serialNum;
      uint8_t vender[32];
      uint8_t user[32];
    }module_setting_t;
    typedef struct{
      uint8_t ip[6];
      uint8_t mask[6];
      uint8_t gateway[6];
      uint8_t macaddr[6];
    }lan_setting_t;
    typedef struct{
      uint8_t slave_address;
      uint8_t baudrate_id;
      uint32_t baudrate_val;
      uint32_t parity;
      uint32_t stop;
      uint32_t data;
    }serial_setting_t;

    typedef struct{
      uint8_t wlan_mode;    // wifi-sta, wifi-ap, bt-slave, bt-master, etc..
      char prefix1[16];     // ap-mode ssid prefix, the ssid = ssid_prefix & serial number
      char passwd1[16];    // ap-mode password, default="53290921"
      char prefix2[16];     // sta-mode ssid
      char passwd2[16];    // sta-mode password, default="53290921"
      uint16_t connectionTimeout;   // timeout for searching available AP
      uint8_t secType;      // WEP-WPA2
    }wireless_param_t;

    typedef struct {
      uint8_t opMode;
      uint8_t commType;
    }node_param_t;

    typedef struct{
      uint16_t sampleNumber;
      uint16_t samplePeriodMs;
    }time_domain_param_t;

    typedef struct{
      uint8_t window;
      uint8_t overlap;
      uint16_t bins;
    }freq_domain_param_t;

    typedef struct {
      uint8_t  fullscale;
      uint8_t outputrate;
      uint8_t highpassfilter;
      uint8_t intmask;
      int32_t offset_cali[3];
      float sensitivity[3];
      float bias[3];
    }adxl355_config_t;

    explicit snode_interface(QObject *parent = nullptr);
    ~snode_interface();
    virtual bool open();
    virtual bool close();
    virtual void setCodec(int index);


    bool isOpen() const ;
    bool isActive() const{return m_isActive;}

    snode_interface::commType type() const {return m_commType;};
    void setType(snode_interface::commType v){m_commType = v;}

    void setHostName(QString name){m_hostName = name;};
    QString hostName() const {return m_hostName;};

    void setPortNumber(int port){m_portNum = port;};
    int portNumber() const{return m_portNum;};

    //void setParser(snode_data_parser *p);

    void log(logType type, QString message);

    void query_device(param_id id);
    void config_device(param_id id, QByteArray b);

    void setDevName(QString name){m_devName = name;}
    QString devName() const{return m_devName;}

    QByteArray get_param_json_by_id(int id);
    void set_param_json_by_id(int id, QByteArray json);

    QByteArray getCodecParam(QString name);
    QByteArray getCodecParam(int id);

    void setCurrSetting(int index){m_currentSetting = index;}
    int getCurrentSetting() const {return m_currentSetting;}



    virtual void read_config(uint8_t cfg_id);
    virtual void write_config(uint8_t cfg_id);
    virtual void send_command(uint8_t command);
    virtual QStringList codec_configs();

    chartView *chart() const{return m_chart;}
    void autoConfig(){m_configOnConnect = true;}
    QString pidInfo() const {return m_pidInfo;}

Q_SIGNALS:
    //void newData(QString host,QByteArray data);
    void newData(QByteArray);
    void timeout();
    void connected();
    void connecting();
    void sendLog(QString message);
    void codec_setup_updated(QByteArray);
    void codec_model_change(QString, int);
    void enableOperation(bool);

public slots:
    void setCommand(QByteArray b);
    void writeCommand(QByteArray b);
    void on_codec_message(QString);
    void on_codec_resp(QByteArray b);
    void on_codec_setup_received(int);
    void on_codec_model_changed(QString name);
//    virtual void setConfigItem(QString name);
//    virtual void getConfigItem(QString name);
    virtual void setConfigItem();
    virtual void getConfigItem();
    //virtual void setConfigContent(QByteArray b){m_configContent = b;}
    void setCodecParam(QString name, QByteArray json);
    void setCodecParam(QByteArray json);

    void setCurrConfigName(const QString &name){
        qDebug()<<Q_FUNC_INFO;
        m_currConfigName = name;}
    void connectToHost();
    void setInterfaceType(QString name){
        qDebug()<<Q_FUNC_INFO;
        m_commType = static_cast<commType>(interface_map.value(name));
    }
    virtual void start_stop();
    void showChart(){
        //if(!isOpen()) return;
        m_chart->setTitle(m_hostName+"-"+m_devName);
        m_chart->show();
    }

    void enableLog(bool v);
    void enableFFT(bool v);

    void setCurrentConfigParam(QString content){
        switch(m_codecID){
        case 0:
            m_codec->setParam(m_currConfigName,content.toLatin1());
            break;
        case 1:
            ((snode_vnode_codec*)m_codec)->setParam(m_currConfigName,content.toLatin1());
            break;
        }
    }

    void setContent(){
        QTextEdit *edt = (QTextEdit*)sender();
        qDebug()<<Q_FUNC_INFO;
        m_tempContent = edt->toPlainText();
    }

    void setLogPath(QString path){m_logPath = path;}
    QString losgPath() const{return m_logPath;}

    void receiveVNodeData(QList<float> v);
    void receiveStreamData(QByteArray b);

    void receivePID(quint8 e,quint8 i){
        m_pidInfo = QString("E:%1/I:%2").arg(e).arg(i);
    }

private slots:
    void handleIncomingData();
private:
    quint16 checksum(const quint8 *data, quint16 length);
    void parseData(QByteArray b);
    //virtual void writeRecord(QList<double> v);

public:
    snode_interface::commType m_commType;
    bool m_validMatch,mLogCSV;
    QTcpSocket *m_socket;
    QSerialPort *m_port;
    bool m_connected,m_connecting;
    QString m_hostName;
    QString m_devName;
    int m_portNum;
    QThread workThread;
    QByteArray m_data;
    //snode_data_parser *m_parser;
    cmd_handler *m_cmdHandler;
    QThread m_workThread;
    snode_codec *m_codec;
    int m_codecID;
    int m_currentSetting;
    quint8 m_currConfigItem;
    bool m_isActive; // data transfer or not
    chartView *m_chart;
    QString m_currConfigName;
   // binPacket *mPacketValid;
    QString m_tempContent;
    bool m_log;
    QFile *m_file;
    QString m_logPath;
    bool m_stream;
    bool m_configOnConnect;
    QString m_pidInfo;
};

class snode_interface_model:public QAbstractTableModel{
    Q_OBJECT
public:
    snode_interface_model(QObject *parent = nullptr);
    ~snode_interface_model();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    snode_interface *getItem(QString name);
    snode_interface *getItem(int id);
    int getItemId(QString name);
public slots:
private:
    QList<snode_interface*> m_data;
signals:
    void editCompleted(const QString &);
};


///** snode simulator */

class snode_simulator:public snode_interface{
    Q_OBJECT

public:
    explicit snode_simulator(QObject *parent = nullptr);
    //~snode_simulator();
    virtual bool open();
    virtual bool close();
    void start();
    void stop();
    void setCodec(int index);

signals:
    void dataReady(QByteArray);

public slots:
    void on_encoder_command(quint8 cmd);

public:
    QThread m_workThread;
    bool m_stop;

private slots:
    void handleIncomingData();
    void on_data_out(QByteArray b);

};

class snode_simulator_model:public QAbstractTableModel{
    Q_OBJECT
public:
    snode_simulator_model(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    snode_simulator *getItem(QString name);
    snode_simulator *getItem(int id);


private:
    QList<snode_simulator*> m_data;
};



#endif // SNODE_INTERFACE_H
