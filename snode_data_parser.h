#ifndef SNODE_DATA_PARSER_H
#define SNODE_DATA_PARSER_H

#include <QObject>
#include <QtCore>
#include <QDataStream>

#define HEADER_SIZE 8
#define MAGIC   0xabba
#define MAGIC1  0xab
#define MAGIC2  0xba

#define MASK_CMD                0xa0
#define MASK_DATA               0x30
#define MASK_CMD_RET_OK         0x50
#define MASK_CMD_RET_ERR        0x51
#define MASK_CMD_RET_BUSY       0x52
#define MASK_CMD_RET_CFG        0x5F

#define CMD_CONTROL        0x01
#define CMD_SETUP          0x02
#define CMD_FS           0x03
#define CMD_SYS       0x0C

#define NOCRC_MASK         0x80
#define CONTROL_STOP        0x00
#define CONTROL_START       0x01

#define CMD_SETUP_SAVE  0xff

#define MODE_SINGLE     0x00
#define MODE_STREAMING  0x01
#define MODE_CHARACTER  0x02
#define MODE_INCLINE    0x03
#define MODE_DISP       0x04

//class snode_data_parser : public QObject
//{
//    Q_OBJECT
//public:
//    explicit snode_data_parser(QObject *parent = nullptr);
//    void set_scale(QVector<double> v){m_scale = v;}
//    virtual QByteArray parseData(QByteArray b, int packet_len){
//        return QByteArray();
//    }
//    virtual QVector<double> parseScaledData(QByteArray b, int packet_len){
//        QVector<double> ret;
//        return ret;
//    }
//    int parse_command(QByteArray b){
//        int ret = 0;

//        return ret;
//    }
//    struct int_3b{
//        union{
//            qint32 v;
//            qint8 b[4];
//        }b;
//        friend QDataStream& operator<<(QDataStream &out, const int_3b &p){
//            out << p.b.b[3];
//            out << p.b.b[2];
//            out << p.b.b[1];
//            out << p.b.b[0];
//            return out;
//        }
//        friend QDataStream& operator>>(QDataStream &in, int_3b &p){
//            in >> p.b.b[3];
//            in >> p.b.b[2];
//            in >> p.b.b[1];
//            p.b.v >>= 12;
//            return in;
//        }
//    };

//signals:

//public slots:

//private:
//    QVector<double> m_scale;
//};

//class snode_char_parser:public snode_data_parser{
//public:
//    virtual QByteArray parseData(QByteArray b, int packet_len){
//        float fv;
//        int nofRecord = packet_len/4;
//        QVector<float> v;
//        QDataStream ds(&b,QIODevice::ReadOnly);
//        QByteArray d;
//        QDataStream out(&d,QIODevice::WriteOnly);
//        ds.setByteOrder(QDataStream::LittleEndian);
//        ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
//        out.setFloatingPointPrecision(QDataStream::SinglePrecision);
//        for(int i=0;i<nofRecord;i++){
//            ds >> fv;
//            out << fv;
//        }
//        qDebug()<<Q_FUNC_INFO<<d.size();
//        return d;
//    }

//    virtual QVector<double> parseScaledData(QByteArray b, int packet_len){
//        QVector<double> ret;
//        QDataStream ds(&b,QIODevice::ReadOnly);
//        int nofRecord = packet_len/4;
//        ds.setByteOrder(QDataStream::LittleEndian);
//        ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
//        float v;
//        for(int i=0;i<nofRecord;i++){
//            ds >>v;
//            ret << v;
//        }
//        return ret;
//    }
//};

//class snode_accel_hir_parser:public snode_data_parser{
//public:
//    virtual QByteArray parseData(QByteArray b, int packet_len){
//        struct int_3b v3b;
//        float fv;
//        int nofRecord = packet_len/3;
//        QDataStream in(&b,QIODevice::ReadOnly);
//        QByteArray bout;
//        QDataStream out(&bout,QIODevice::WriteOnly);
//        //in.setByteOrder(QDataStream::LittleEndian);
//        //out.setByteOrder(QDataStream::LittleEndian);
//        //out.setFloatingPointPrecision(QDataStream::SinglePrecision);
//        QByteArray s;
//        for(int i=0;i<nofRecord;i++){
//            s = b.mid(i*3,3);
//            in >> v3b;
//            out << v3b;
//            //qDebug()<<s.toHex()<<QString("%1").arg(v3b.b.v,16);
//        }
//        //qDebug()<<Q_FUNC_INFO<<bout.size();
//        return bout;
//    }
//};

//class snode_imu6_parser:public snode_data_parser{
//public:
//    virtual QByteArray parseData(QByteArray b, int packet_len){
//        int nofRecord = packet_len/12;
//        qint16 int16;
//        QDataStream in(&b,QIODevice::ReadOnly);
//        QByteArray bout;
//        QDataStream out(&bout,QIODevice::WriteOnly);
//        in.setByteOrder(QDataStream::LittleEndian);
//        //out.setFloatingPointPrecision(QDataStream::SinglePrecision);
//        for(int i=0;i<nofRecord;i++){
//            in >> int16;
//            out << int16;
//        }
//        qDebug()<<Q_FUNC_INFO<<bout.size();
//        return bout;
//    }
//};


class snode_codec:public QObject{
    Q_OBJECT
public:
    const QMap<QString,quint8> config_map =
            QMap<QString,quint8>({{"MODULE",0x40},{"SERIAL",0x41},{"LAN",0x42},{"WLAN",0x43},{"USER",0x4F}});



    virtual QStringList supportConfig() const{return QStringList(config_map.keys());}

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
        PARAM_SENSOR_BASE_P4,
        PARAM_SENSOR_BASE_P5,
//        PARAM_USER_DATA = 0x10,
        PARAM_MODULE_CONFIG = 0x40,
        PARAM_MODULE_SERIAL,
        PARAM_MODULE_LAN,
        PARAM_MODULE_WLAN,
        PARAM_USER_DATA = 0x4f,
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

    //typedef struct module_setting module_setting_t;
    typedef struct module_setting{
      quint32 flag;
      quint8 name[32];
      quint32 verNum;
      quint32 serialNum;
      quint8 vender[32];
      quint8 user[32];
      friend QDataStream& operator<<(QDataStream &out, const module_setting &p){
        out << p.flag;
        out.writeBytes((char*)p.name,32);
        out << p.verNum;
        out <<p.serialNum;
        out.writeRawData((char*)p.vender,32);
        out.writeRawData((char*)p.user,32);
        return out;
      }

      friend QDataStream& operator>>(QDataStream &in, module_setting &p){
        in >> p.flag;
        in.readRawData((char*)p.name,32);
        in >> p.verNum;
        in >> p.serialNum;
        in.readRawData((char*)p.vender,32);
        in.readRawData((char*)p.user,32);
        return in;
      }

    }module_setting_t;

    typedef struct{
      quint8 ip[6];
      quint8 mask[6];
      quint8 gateway[6];
      quint8 macaddr[6];
    }lan_setting_t;
    typedef struct{
      quint8 slave_address;
      quint8 baudrate_id;
      quint32 baudrate_val;
      quint32 parity;
      quint32 stop;
      quint32 data;
    }serial_setting_t;

    typedef struct{
      quint8 wlan_mode;    // wifi-sta, wifi-ap, bt-slave, bt-master, etc..
      quint8 prefix1[16];     // ap-mode ssid prefix, the ssid = ssid_prefix & serial number
      quint8 passwd1[16];    // ap-mode password, default="53290921"
      quint8 prefix2[16];     // sta-mode ssid
      quint8 passwd2[16];    // sta-mode password, default="53290921"
      quint16 connectionTimeout;   // timeout for searching available AP
      uint8_t secType;      // WEP-WPA2
    }wireless_param_t;

    explicit snode_codec(QObject *parent = nullptr);
    ~snode_codec();
    int parse_command(QByteArray b);

//    void setModuleParam(module_setting_t setting);
//    module_setting_t getModuleParam() const{return m_moduleSetting;}

    quint16 checksum(const quint8 *data, quint16 len);
    quint16 checksum(QByteArray b, int len);
    QVector<float> result() const {return m_result;}
    virtual QVector<double> sampleParams() const{return QVector<double>();}

    virtual QByteArray getParam(int id);
    virtual QByteArray getParam(QString name);
    virtual void setParam(int id, QByteArray json);
    virtual void setParam(QString name, QByteArray json);
    virtual bool decodeData();

    //virtual QStringList configList();
public:
    module_setting_t m_moduleSetting;
    serial_setting_t m_serialSetting;
    lan_setting_t m_lanSetting;
    wireless_param_t m_wlanSetting;
    bool m_bWaitResponse;
    QByteArray m_userData;
    QByteArray m_incomingData;
    QVector<float> m_result;
    int m_genRecords;
    quint8 m_expPID;
    quint32 pidIn, pidExp;
    //QHash<QString, quint8> m_supportConfig;

public slots:
    virtual void issue_param_read(quint8 cmd);
    virtual void issue_param_read(QString name);
    virtual void issue_param_write(quint8 cmd);
    virtual void issue_param_write(QString name);

//    virtual void on_data_in(QByteArray b);
    virtual void on_decoder_received(QByteArray b);
    virtual void on_encoder_received(QByteArray b);
    virtual void issue_command(quint8 cmd);
    void clear(){m_incomingData.clear();}
    virtual void issue_active(bool act);
    virtual void issue_param_save();

signals:
    void send_command(QByteArray);
    void send_resp(QByteArray);
    void new_data();
    void setupReceived(int);
    void modelNameUpdate(QString);

    void write_data(QByteArray);
    void send_log(QString);
};

class snode_vnode_codec:public snode_codec{
    Q_OBJECT
public:
    const QMap<QString,quint8> config_map =
            QMap<QString,quint8>({
                                     {"MODULE",0x40},{"SERIAL",0x41},{"LAN",0x42},{"WLAN",0x43},{"USER",0x4F},
                                     {"NODE",0x0},{"ACCEL",0x1},{"TIME",0x2},{"FREQ",0x3}
                                 });

    const  QMap<QString, quint8> opmode_map =
            QMap<QString, quint8>({{"STREAM",0},{"VNODE",1},{"FNODE",2},{"OLED",3}});

    const  QMap<QString, quint8> accRange_map =
            QMap<QString, quint8>({{"WRONG",0},{"2G",1},{"4G",2},{"8G",3}});

    const  QMap<QString, quint8> odrRange_map =
            QMap<QString, quint8>({{"4000",0},{"2000",1},{"1000",2},{"500",3},{"250",4},{"125",5},{"62.5",6},{"31.25",7}});

    const  QMap<QString, quint8> fft_window_map =
            QMap<QString, quint8>({{"NONE",0},{"HAMMING",1},{"HANNING",2},{"GAUSS",3}});
    QStringList supportConfig() const{
        return QStringList(config_map.keys());
    }
    typedef struct {
      uint8_t opMode;
      uint8_t commType;
    }node_param_t;


    typedef struct {
      uint8_t  fullscale;
      uint8_t outputrate;
      uint8_t highpassfilter;
      uint8_t intmask;
      int32_t offset_cali[3];
      float sensitivity[3];
      float bias[3];
    }adxl355_config_t;

    typedef struct{
      uint16_t sampleNumber;
      uint16_t samplePeriodMs;
    }time_domain_param_t;

    typedef struct{
      uint8_t window;
      uint8_t overlap;
      uint16_t bins;
    }freq_domain_param_t;
    struct int_3b{
        union{
            qint32 v;
            qint8 b[4];
        }b;
        friend QDataStream& operator<<(QDataStream &out, const int_3b &p){
            out << p.b.b[3];
            out << p.b.b[2];
            out << p.b.b[1];
            out << p.b.b[0];
            return out;
        }
        friend QDataStream& operator>>(QDataStream &in, int_3b &p){
            in >> p.b.b[3];
            in >> p.b.b[2];
            in >> p.b.b[1];
            p.b.v >>= 12;
            return in;
        }
    };

    explicit snode_vnode_codec(QObject *parent = nullptr);
    QVector<double> sampleParams();

    virtual QByteArray getParam(int id);
    virtual QByteArray getParam(QString name);
    virtual void setParam(int id, QByteArray json);
    virtual void setParam(QString name, QByteArray json);
    virtual bool decodeData();
    //virtual QStringList configList();
    void enableFFT(bool v){
        m_genFFT = v;
    }
    bool fft() const{return m_genFFT;}

    quint8 nodeMode() const{return m_nodeParam.opMode;}

signals:
    void send_command(quint8);
    void newRecord(QList<float>);
    void newRaw(QByteArray);
    void newSeries(QVector<float>);
    void newWave(QVector<float>,QVector<float>,QVector<float>);
    void newFFT(QVector<float>,QVector<float>,QVector<float>);
    void pidUpdate(quint8,quint8);
public slots:
    void issue_param_read(QString name);
    void issue_param_write(int cmd);
    void issue_param_write(QString name);
    virtual void on_decoder_received(QByteArray b);
    virtual void on_encoder_received(QByteArray b);
    void generateRecord(int nofRecord);
    //virtual void issue_command(quint8 cmd);

private:
    void parseVNode();
    void parseStream();
    void genFFT();

private:
    node_param_t m_nodeParam;
    adxl355_config_t m_adxl;
    time_domain_param_t m_timeDomainParam;
    freq_domain_param_t m_freqDomainParam;
    //QVector<float> m_fftResult[3];
    QVector<float> m_waveResult[3];
    float m_adxlScale;
    bool m_genFFT;

};



#endif // SNODE_DATA_PARSER_H
