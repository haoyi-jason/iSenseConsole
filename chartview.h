#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QDialog>
#include <QtCharts>
#include <QChartView>

class dataModel{
    public:
    float value,threshold;
};

class paramModel:public QAbstractTableModel{
    Q_OBJECT
    typedef struct{
        QString parName;
        float value;
    }_par_t;

public:
    explicit paramModel(QObject *parent = nullptr){
        m_data.append({"X-Min",0});
        m_data.append({"X-Max",1000});
        m_data.append({"Y-Min",-10});
        m_data.append({"Y-Max",10});
    }
    ~paramModel(){}
    int rowCount(const QModelIndex &parent = QModelIndex()) const override{
        return 4;
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 1;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        switch(role){
        case Qt::DisplayRole:
            return QVariant::fromValue(m_data.at(index.row()).value);
            break;
//        case Qt::CheckStateRole:
//            return m_visible[index.column()];
//            break;
        }
        return QVariant();
    }
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override{
        if(role == Qt::DisplayRole && orientation == Qt::Vertical){
            return m_data.at(section).parName;
        }
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) override{
        qDebug()<<Q_FUNC_INFO<<index.row()<<value;
        if(role == Qt::EditRole){
            float v = value.toFloat();
            _par_t p = m_data.at(index.row());
            p.value = v;
            qDebug()<<"NEW VALUE:"<<p.value;
            //m_data.replace(index.row(),p);
            m_data[index.row()].value = v;
        }
        update();
        return false;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override{
        return  Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }

    void setChart(QChart *c){
        m_chart = c;
        update();
    }

    void setValue(int id, float value){
        if(id < m_data.count()){
            m_data[id].value = value;
        }
    }
private:
    void update(){
        if(!m_chart) return;
        m_chart->axes(Qt::Horizontal).back()->setRange(m_data[0].value,m_data[1].value);
        m_chart->axes(Qt::Vertical).back()->setRange(m_data[2].value,m_data[3].value);

        qDebug()<<Q_FUNC_INFO<<m_data[1].value;
    }

private:
    QList<_par_t> m_data;
    QChart *m_chart;
};

class vnodeDataModel:public QAbstractTableModel
{
    Q_OBJECT
public:
    const QMap<QString,quint8> id_map =
            QMap<QString,quint8>({{"X Peak",0x0},{"Y Peak",0x1},{"Z Peak",0x2},{"X RMS",0x3},{"Y RMS",0x4},{"Z RMS",0x5},{"Ymin/max",0x6},{"Y max",0x7}});
    const QStringList header={"X Peak","Y Peak","Y Peak","Y RMS","Z Peak","Z RMS"};
    const QStringList rowHeader = {"Value(m/s^2)","Threshold(m/s^2)","Visible"};
    QList<dataModel*> m_data;
    QList<bool> m_visible;
    QChart *m_chart;

    vnodeDataModel(QObject *parent = nullptr){
        m_visible << true<<true<<true<<true<<true<<true;
        for(int i=0;i<6;i++){
            m_data.append(new dataModel);
            m_data.at(i)->value = 0;
            m_data.at(i)->threshold = 0;
        }
        m_data.append(new dataModel);
        m_data[6]->value = 0;
        m_data[6]->threshold = 10;
        m_chart = nullptr;
    }
    ~vnodeDataModel(){
        foreach(dataModel *m, m_data){
            delete m;
        }
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override{
        return 7;//id_map.count();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 3;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        switch(role){
        case Qt::DisplayRole:
            if(index.column() == 0)
                return m_data.at(index.row())->value;
            else if(index.column() == 1)
                return m_data.at(index.row())->threshold;
            break;
//        case Qt::CheckStateRole:
//            return m_visible[index.column()];
//            break;
        }
        return QVariant();
    }
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override{
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
            return rowHeader.at(section);
        }
        if(role == Qt::DisplayRole && orientation == Qt::Vertical){
            return id_map.key(section);
        }
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) override{
        if(role == Qt::DisplayRole || role == Qt::EditRole){
            float v = value.toFloat();
            if(index.column() == 0)
                m_data.at(index.row())->value = v;
            else if(index.column() == 1)
                m_data.at(index.row())->threshold = v;
            if(index.row() == 6){
                qDebug()<<"Set Y range";
                updateChart();
            }
        }

        return false;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override{
        return  Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override{
        dataModel *data = new dataModel;
        beginInsertRows(parent,row,row+count-1);
        m_data.append(data);
        endInsertRows();
        return true;

    }
//    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    dataModel *getItem(int id) {
        if(id < m_data.size()){
            return m_data.at(id);
        }
        return nullptr;
    }

    dataModel *getItem() {
        if(m_data.size()){
            return m_data.last();
        }
        return nullptr;
    }
    void setChart(QChart *c){
        m_chart = c;
        updateChart();
    }

public slots:
    void setVisible(bool v){
        QPushButton *btn = (QPushButton*)sender();
        int id = btn->property("ID").toInt();
        if(id < m_data.size()){
            m_visible[id] = v;
            if(v)
                btn->setText("SHOW");
            else
                btn->setText("HIDE");
            emit showCurve(id,v);
        }
    }

signals:
    void itemChecked(int,bool);
    void showCurve(int,bool);

private:
    void updateChart(){
        if(!m_chart){
            qDebug()<<Q_FUNC_INFO<<"No chart";
            return;
        }
//        m_chart->axes(Qt::Horizontal).back()->setRange(m_data[6].value,m_data[1].value);
        m_chart->axes(Qt::Vertical).back()->setRange(m_data[6]->value,m_data[6]->threshold);


    }


};

namespace Ui {
class chartView;
}



class chartView : public QDialog
{
    Q_OBJECT

public:
    const QMap<QString,quint8> chartType_map =
            QMap<QString,quint8>({{"TIME",0x0},{"TREND",0x1},{"IMU",0x2}});

    const QMap<QString,quint8> seriesNameXYZ_map =
            QMap<QString,quint8>({{"X",0x0},{"Y",0x1},{"Z",0x2},{"GX",0x3},{"GY",0x4},{"GZ",0x5}});

    const QMap<QString,quint8> seriesNameRMS_map =
            QMap<QString,quint8>({{"X-Peak",0x0},{"Y-PEAK",0x1},{"Z-Peak",0x2},{"X-RMS",0x3},{"Y-RMS",0x4},{"Z-RMS",0x5}});

    explicit chartView(QWidget *parent = nullptr);
    ~chartView();
    void setTitle(QString title);
    QLineSeries *getSeries(QString name);

    QString chartType() const{return chartType_map.key(m_chartType);}
    void setChartType(QString name);
    void setChartType(int id);

public slots:
    void addData(QString name, QPointF value);
    void setData(QString name, QList<QPointF> values);
    void addRecord(QList<float> v);
    void addStream(QByteArray v);
    void updateSeries(QVector<float>);
    void updateWave(QVector<float>,QVector<float>,QVector<float>);
    void updateGYRO(QVector<float>,QVector<float>,QVector<float>);
    void updateFFT(QVector<float>,QVector<float>,QVector<float>);
    void showTrend(int ,bool);
private slots:
    void handle_chart_update();

    void on_pbLog_clicked();

    void on_pbCapture_clicked();

private:
    Ui::chartView *ui;
    QChartView *m_cv;

    vnodeDataModel *m_dataModel;
    QList<QLineSeries*> m_series;
    QList<QChart*> m_charts;
    int m_chartType;
    QVector<float> m_seriesData[6];
    QVector<float> m_seriesFFTData[6];
    int nofXPoints;
    QTimer *m_tmr;
    QMutex m_mutex;
    QThread *m_workThread;
};

#endif // CHARTVIEW_H
