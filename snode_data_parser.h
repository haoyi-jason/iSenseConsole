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



class snode_codec:public QObject{
    Q_OBJECT
public:
//    const QMap<QString,quint8> config_map =
//            QMap<QString,quint8>({{"MODULE",0x40},{"SERIAL",0x41},{"LAN",0x42},{"WLAN",0x43},{"USER",0x4F}});
    const QMap<QString,quint8> config_map =
            QMap<QString,quint8>({{"MODULE",0x40},{"USER",0x4F}});



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
        PARAM_SENSOR_BASE_P6,
        PARAM_SENSOR_BASE_P7,

//        PARAM_USER_DATA = 0x10,
        PARAM_MODULE_TRH = 0x0c,
        PARAM_MODULE_BATTERY = 0x0d,
        PARAM_MODULE_RTC=0x0e,
        PARAM_MODULE_CONFIG = 0x40,
        PARAM_MODULE_SERIAL,
        PARAM_MODULE_LAN,
        PARAM_MODULE_WLAN,
        PARAM_USER_DATA = 0x4F,
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
      quint8 pairedInfo[32];
    }wireless_param_t;

    typedef struct{
        quint8 yy,mm,dd,hh,nn,ss;
    }rtc_param_t;

    typedef struct{
        quint16 b1,b2;
    }battery_param_t;

    typedef struct{
        float temp,rh;
    }trh_param_t;

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
    quint8 nodeMode() const{return 0;}

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
    virtual void issue_file_command(quint8 pid, QByteArray b);

signals:
    void send_command(QByteArray);
    void send_resp(QByteArray);
    void new_data();
    void setupReceived(int);
    void unResolvedPacket(quint8,QByteArray);
    void modelNameUpdate(QString);

    void write_data(QByteArray);
    void send_log(QString);
    void new_stream(QStringList,QVector<float>);
};

class snode_vnode_codec:public snode_codec{
    Q_OBJECT
    const QMap<QString,quint8> config_map =
            QMap<QString,quint8>({
                                     {"MODULE",0x40},{"SERIAL",0x41},{"LAN",0x42},{"WLAN",0x43},{"USER",0x4F},
                                     {"NODE",0x0},{"ACCEL",0x1},{"IMU",0x2},{"TIME",0x3},{"FREQ",4},{"OLED",5}
                                 });

//    const QMap<QString,quint8> config_map =
//            QMap<QString,quint8>({
//                                     {"MODULE",0x40},
//                                     {"NODE",0x0},{"ACCEL",0x1},{"IMU",0x2},{"TIME",0x3},{"OLED",5}
//                                 });
    const  QMap<QString, quint8> opmode_map =
            QMap<QString, quint8>({{"STREAM",0},{"VNODE",1},{"FNODE",2},{"OLED",3}});

    const  QMap<QString, quint8> accRange_map =
            QMap<QString, quint8>({{"WRONG",0},{"2G",1},{"4G",2},{"8G",3}});

    const  QMap<QString, quint8> odrRange_map =
            QMap<QString, quint8>({{"4000",0},{"2000",1},{"1000",2},{"500",3},{"250",4},{"125",5},{"62.5",6},{"31.25",7}});

    const  QMap<QString, quint8> fft_window_map =
            QMap<QString, quint8>({{"NONE",0},{"HAMMING",1},{"HANNING",2},{"GAUSS",3}});


public:
    QStringList supportConfig() const{
        return QStringList(config_map.keys());
    }
    typedef struct {
      uint8_t opMode;
      uint8_t commType;
      uint8_t activeSensor;
    }node_param_t;


    typedef struct {
      uint8_t  fullscale;
      uint8_t outputrate;
      uint8_t highpassfilter;
      uint8_t intmask;
//      int32_t offset_cali[3];
//      float sensitivity[3];
//      float bias[3];
    }adxl355_config_t;
    typedef struct{
        quint8 power;
        quint8 odr;
        quint8 range;
        quint8 lpf;
    }_imu_element_cfg;
    typedef struct{
        _imu_element_cfg accel;
        _imu_element_cfg gyro;
    }imu_config_t;

    typedef struct{
      uint16_t sampleNumber;
      uint16_t samplePeriodMs;
    }time_domain_param_t;

    typedef struct{
      uint8_t window;
      uint8_t overlap;
      uint16_t bins;
    }freq_domain_param_t;

    typedef struct{
        uint16_t base;
        uint16_t type;
    }oled_param_t;

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
        //qDebug()<<Q_FUNC_INFO<<v;
    }
    bool fft() const{
        //qDebug()<<Q_FUNC_INFO<<m_genFFT;
        return m_genFFT;}

    quint8 nodeMode() const{return m_nodeParam.opMode;}
    quint8 activeSensor() const{return m_nodeParam.activeSensor;}
    float accel_range() const{
        switch(activeSensor()){
        case 1:
            switch(m_adxl.fullscale){
            case 0x1:return 2;break;
            case 0x2:return 4;break;
            case 0x3:return 8;break;
            default:return 1;break;
            }
            break;
        case 2:
            switch(m_imuParam.accel.range){
            case 0x3:return 2;break;
            case 0x5:return 4;break;
            case 0x8:return 8;break;
            case 0xc:return 16;break;
            default:return 0;
            }
            break;
        case 4:
            switch(m_imuParam.accel.range){
            case 0x0:return 2;break;
            case 0x1:return 16;break;
            case 0x2:return 4;break;
            case 0x3:return 8;break;
            default:return 1;
            }
            break;
        }
        return 1;
    }
    float accel_rate() const{
        switch(activeSensor()){
        case 1:
            return (4000./(1 << m_adxl.outputrate));
            break;
        case 4:
            return 6666./(1 << (m_imuParam.accel.odr-0xa));
            break;
        }
        return 1;
    }
    float gyro_range() const{
        switch(activeSensor()){
        case 2:
            return 2000./(1 << m_imuParam.gyro.range);
            break;
        case 4:
            switch(m_imuParam.gyro.range){
            case 0:return 250;break;
            case 1:return 125;break;
            case 2:return 500;break;
            case 4:return 1000;break;
            case 6:return 2000;break;
            default:return 250;break;
            }
            break;
        default:
            return 0.;
        }
        return 0.;
    }
    float gyro_rate() const{
        return accel_rate();
    }

    void genFFT();

signals:
    void send_command(quint8);
    void newRecord(QList<float>);
    void newRaw(QByteArray);
    void newSeries(QVector<float>);
    void newWave(QVector<float>,QVector<float>,QVector<float>);
    void newFFT(QVector<float>,QVector<float>,QVector<float>);
    void newFFTDF(QVector<float>,QVector<float>,QVector<float>,float df);
    void pidUpdate(quint8,quint8);
    void filePacket(quint8,QByteArray);
    //void unResolvedPacket(quint8,QByteArray);
public slots:
    virtual void issue_param_read(QString name);
    virtual void issue_param_write(int cmd);
    virtual void issue_param_write(QString name);
    virtual void on_decoder_received(QByteArray b);
    virtual void on_encoder_received(QByteArray b);
    void generateRecord(int nofRecord);
    //virtual void issue_command(quint8 cmd);

private:
    void parseVNode();
    void parseStream();
    float hamming(int i, int n);

public:
    node_param_t m_nodeParam;
    adxl355_config_t m_adxl;
    time_domain_param_t m_timeDomainParam;
    freq_domain_param_t m_freqDomainParam;
    imu_config_t m_imuParam;
    float m_adxlScale;
    bool m_genFFT;
    QVector<float> m_waveResult[6];
    oled_param_t m_oledParam;
private:
    //QVector<float> m_fftResult[3];

};

class snode_vss_codec:public snode_vnode_codec{
    Q_OBJECT
    const QMap<QString,quint8> config_map =
            QMap<QString,quint8>({
                                     {"MODULE",0x40},{"SERIAL",0x41},{"LAN",0x42},{"WLAN",0x43},{"USER",0x4F},
                                     {"NODE",0x0},{"ACCEL",0x1},{"IMU",0x2},{"TIME",0x3},{"FREQ",0x4},{"SDC",0x5}
                                 });

    const  QMap<QString, quint8> opmode_map =
            QMap<QString, quint8>({{"STREAM",0},{"VNODE",1},{"FNODE",2},{"OLED",3},{"SDLOG",4}});

    const  QMap<QString, quint8> accRange_map =
            QMap<QString, quint8>({{"WRONG",0},{"2G",1},{"4G",2},{"8G",3}});

    const  QMap<QString, quint8> odrRange_map =
            QMap<QString, quint8>({{"4000",0},{"2000",1},{"1000",2},{"500",3},{"250",4},{"125",5},{"62.5",6},{"31.25",7}});

    const  QMap<QString, quint8> fft_window_map =
            QMap<QString, quint8>({{"NONE",0},{"HAMMING",1},{"HANNING",2},{"GAUSS",3}});

    const  QMap<QString, quint8> imuaccRange_map =
            QMap<QString, quint8>({{"2G",0x3},{"4G",0x5},{"8G",0x8},{"16G",0xc}});

    const  QMap<QString, quint8> imugyroRange_map =
            QMap<QString, quint8>({{"2000 DPS",0},{"1000 DPS",1},{"500 DPS",2},{"250 DPS",3},{"125 DPS",4}});

public:
    QStringList supportConfig() const{
        return QStringList(config_map.keys());
    }

    typedef struct{
        uint8_t savdSd;
        uint8_t prefix[16];
        uint32_t szConstrain;
        uint32_t capacity;
    } sd_config_t;

    explicit snode_vss_codec(QObject *parent = nullptr);
    virtual QByteArray getParam(int id);
    virtual QByteArray getParam(QString name);
    virtual void setParam(int id, QByteArray json);
    virtual void setParam(QString name, QByteArray json);
    virtual bool decodeData();

    QVector<int> parseStreamEx(QByteArray b, quint8 sensor);

    float accel_range() const{
        switch(activeSensor()){
        case 1:
            switch(m_adxl.fullscale){
            case 0x1:return 2;break;
            case 0x2:return 4;break;
            case 0x3:return 8;break;
            default:return 1;break;
            }
            break;
        case 2:
            switch(m_imuParam.accel.range){
            case 0x3:return 2;break;
            case 0x5:return 4;break;
            case 0x8:return 8;break;
            case 0xc:return 16;break;
            default:return 0;
            }
            break;
        }
        return 1;
    }
    float accel_rate() const{
        switch(activeSensor()){
        case 1:
            return (4000./(1 << m_adxl.outputrate));
            break;
        case 2:
            return 1600./(1 << (m_imuParam.accel.odr-0xc));
            break;
        }
        return 1;
    }

    float gyro_range() const{
        switch(activeSensor()){
        case 2:
            return 2000./(1 << m_imuParam.gyro.range);
            break;
        case 4:
            break;
        default:
            return 0.;
        }
        return 0.;
    }
    float gyro_rate() const{
        return 1600./(1 << (m_imuParam.accel.odr-0xc));
    }
    void genFFT();
signals:
    void newGYRO(QVector<float>,QVector<float>,QVector<float>);
    //void unResolvedPacket(quint8,QByteArray);

public slots:
    virtual void issue_param_read(QString name);
    virtual void issue_param_write(int cmd);
    virtual void issue_param_write(QString name);

    virtual void on_decoder_received(QByteArray b);
    virtual void on_encoder_received(QByteArray b);

private:
    void parseVNode();
    void parseStream();

private:
//    node_param_t m_nodeParam;
//    adxl355_config_t m_adxl;
//    time_domain_param_t m_timeDomainParam;
//    freq_domain_param_t m_freqDomainParam;
    sd_config_t m_sdParam;
//    QVector<float> m_waveResult[6];

};


#endif // SNODE_DATA_PARSER_H
