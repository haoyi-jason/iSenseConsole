#ifndef FILEMANAGE_H
#define FILEMANAGE_H

#include <QDialog>
#include <QDebug>
#include <QAbstractTableModel>
#include <QFileSystemModel>
#include "chartview2.h"
#include "snode_data_parser.h"
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QStyleOptionProgressBar>

class file_info{
    char m_path[32];
    char m_fileName[32];
    quint32 m_fileSize;
    QString m_destPath;
    QString m_destFileName;
    int m_progress;
    qint64 m_bytesTransferred;
    qint64 mStartTime;
    QString mTransInfo;
public:
    file_info(QString path="", QString name="",int size=0){
        memset(m_path,0,32);
        memset(m_fileName,0,32);
        memcpy((void*)m_path,path.toUtf8(),path.size());
        memcpy((void*)m_fileName,name.toUtf8(),name.size());
        m_fileSize = size;
        m_progress = 0;
        m_bytesTransferred = 0;
    }
    QString fileName() const{return QString::fromUtf8(m_fileName);}
    QString path() const {return QString::fromUtf8(m_path);}
    unsigned int size() const{return m_fileSize;}

    void setDestPath(QString val){m_destPath = val;}
    void setDestFileName(QString val){m_destFileName = val;}

    QByteArray toByteArray(){
        QByteArray ret;
        QDataStream ds(&ret,QIODevice::WriteOnly);
        ds.writeRawData((char*)m_path,32);
        ds.writeRawData((char*)m_fileName,32);
        ds<<(m_fileSize);
        qDebug()<<Q_FUNC_INFO<<"Size = "<<ret.size();
        return ret;
    }

    void fromByteArray(QByteArray b){
        QDataStream ds(&b,QIODevice::ReadOnly);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds.readRawData(m_path,32);
        ds.readRawData(m_fileName,32);
        ds >> m_fileSize;

        qDebug()<<fileName()<<size();
    }
    void setBytesTransferred(qint64 v){
        qint64 elapsedms;
        double rate;
        if(m_bytesTransferred == 0){
            mStartTime = QDateTime::currentMSecsSinceEpoch();
        }else{
            elapsedms = QDateTime::currentMSecsSinceEpoch()-mStartTime;
        }
        m_bytesTransferred = v;

        double d = (double)m_bytesTransferred;
        rate = 1000.*d/elapsedms;
        //if(rate < 1000){
            mTransInfo=QString("%1 Bytes, Rate=%2 Bps").arg(m_bytesTransferred).arg(rate);
        //}
        m_progress = 100*(d/m_fileSize);


    }
    void setProgress(int v){m_progress = v;}
    int progress() const { return m_progress;}
    qint64 bytesTransferred() const{return m_bytesTransferred;}
    QString transInfo() const{return mTransInfo;}
};

#define FILE_OP_OK         0x10
#define FILE_OP_ERR        0x20

#define FILE_OP_CHDIR       0x10
#define FILE_LIST          0x01
#define FILE_OPEN          0x02
#define FILE_READ          0x03
#define FILE_WRITE         0x04
#define FILE_REMOVE        0x05
//#define FILE_CHDIR         0x06
#define FILE_CLOSE         0x07
#include <QFile>

class file_op_t{
    char m_fileName[32];
    quint32 m_offset;
    quint32 m_size;
    QFile *f;
public:
    file_op_t(QString fileName="",quint32 offset = 0, quint16 size=256){
        memcpy((char*)m_fileName,fileName.constData(),fileName.size());
        m_offset = offset;
        m_size = size;
        f = nullptr;
    }
    void setFileName(QString name){
        qDebug()<<"Set name"<<name<<name.size();
        memcpy((char*)m_fileName,name.toLatin1(),name.size());
        if(name.size() < 32)
            m_fileName[name.size()]=0x0;
        else
            m_fileName[31] = 0x0;
    }
    QString fileName() const{return QString::fromUtf8(m_fileName);}
    quint16 size() const {return m_size;}
    quint16 offset() const{return m_offset;}
    void setSize(quint16 v){m_size = v;}
    void setOffset(quint32 v){m_offset = v;}
    QByteArray toByteArray(){
        QByteArray ret;
        QDataStream ds(&ret,QIODevice::WriteOnly);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds.writeRawData((char*)m_fileName,32);
        ds<<(m_offset);
        ds<<m_size;
        return ret;
    }

    void fromByteArray(QByteArray b){
        QDataStream ds(&b,QIODevice::ReadOnly);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds.readRawData(m_fileName,32);
        ds >> m_offset;
        ds >> m_size;

    }

    bool openFile(QString root=""){
        QString fname;
        if(root.size() == 0)
            fname = "./"+QString::fromUtf8(m_fileName);
        else
            fname = root+"/"+QString::fromUtf8(m_fileName);
        if(f == nullptr){
            f = new QFile;
            f->setFileName(fname);
            if(!f->open(QIODevice::WriteOnly)){
                f->close();
                f = nullptr;
                return false;
            }
        }
        return true;
    }

    void closeFile(){
        if(f != nullptr){
            f->close();
            f = nullptr;
        }
    }

    qint64 writeFile(QByteArray b){
        //qDebug()<<Q_FUNC_INFO<<b.size()<<b;
        if(f == nullptr) return -1;
        QDataStream ds(f);
        //qDebug()<<"File size"<<f->size();
        ds.writeRawData(b.constData(),b.size());
        //qDebug()<<"File size"<<f->size();
        m_offset += b.size();
        return f->size();
    }
};

class ProgressDelegate:public QStyledItemDelegate{
    Q_OBJECT
public:
    ProgressDelegate(QObject *parent=nullptr):QStyledItemDelegate (parent){}
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index)const{
        double progress = index.model()->data(index).toInt();
        QStyleOptionProgressBar bar;
        bar.rect = option.rect;
        bar.minimum = 0;
        bar.maximum = 100;
        bar.progress = progress;
        bar.text = QString::number(progress)+"%";
        bar.textVisible = true;
        QApplication::style()->drawControl(QStyle::CE_ProgressBar,&bar,painter);
    }
};

class fileListModel:public QAbstractTableModel{
    Q_OBJECT
    const QStringList header={"path","filename","size(bytes)","progress","bytes transffered"};
    QList<file_info*> m_data;
public:
    explicit fileListModel(QObject *parent=nullptr){

    }
    ~fileListModel(){};
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_data.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return header.size();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        switch(role){
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch(index.column()){
            case 0:return m_data.at(index.row())->path();break;
            case 1:return m_data.at(index.row())->fileName();break;
            case 2:return m_data.at(index.row())->size();break;
            case 3:return m_data.at(index.row())->progress();break;
            case 4:return m_data.at(index.row())->transInfo();break;
            default:return QVariant();break;
            }
            break;
        }
        return QVariant();
    }
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override{
        //qDebug()<<Q_FUNC_INFO<<section;
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
            return header[section];
        }
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        qDebug()<<Q_FUNC_INFO<<index.row()<<value;
//        if(role == Qt::EditRole){
//        }
        return false;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override{
        return QAbstractTableModel::flags(index);
    }

    bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex()) override{
        file_info *v = new file_info;
        beginInsertRows(parent,row,row+count-1);
        m_data.append(v);
        endInsertRows();
        return true;
    }
    bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex()) override{
        if(row >= m_data.size()) return false;
        if(row < 0) return false;
        beginRemoveRows(parent,row,row+count-1);
        m_data.removeAt(row);
        endRemoveRows();
        return true;
    }
    file_info *getItem(int row){
        file_info *ret = nullptr;
        if(row < m_data.size())
            ret = m_data.at(row);
        return ret;
    }
};

namespace Ui {
class fileManage;
}

class fileManage : public QDialog
{
    Q_OBJECT

public:
    explicit fileManage(QWidget *parent = nullptr);
    ~fileManage();
    void setCaption(QString cap);

signals:
    void ls(QByteArray b);
    void read(QByteArray b);
    void rm(QByteArray b);
    void file_op(quint8 pid, QByteArray b);

public slots:
    int parsePacket(quint8 pid, QByteArray b);
    void plotData(int start, int end);

private slots:
    void on_pbls_clicked();

    void on_pbTransfer_clicked();

    void on_treeView_clicked(const QModelIndex &index);

    void on_listView_doubleClicked(const QModelIndex &index);

    void on_pbShowChart_clicked();

    void on_pbClearChart_clicked();

    void on_pbPlot_clicked();

    void on_pbRemove_clicked();

    void on_pbParse_clicked();

private:
private:
    Ui::fileManage *ui;
    bool bReadFile;
    file_info *m_finfo;
    file_op_t m_fop;
    QFileSystemModel *dirModel;
    QFileSystemModel *fileModel;
    QFile mFile;
    chartView2 *cv;
    int mSensorID;
    QString m_rootPath;
    QString m_suffix;
};

#endif // FILEMANAGE_H
