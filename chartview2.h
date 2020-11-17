#ifndef CHARTVIEW2_H
#define CHARTVIEW2_H

#include <QDialog>
#include <QFileSystemModel>
#include <QtCharts>
#include <QChartView>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QStyleOptionSlider>

namespace Ui {
class chartView2;
class seriesModel;
//class seriesData;
}

class SliderDelegate:public QStyledItemDelegate{
    Q_OBJECT
public:
    SliderDelegate(QObject *parent=nullptr):QStyledItemDelegate(parent){}
//    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index)const{
//        int max = index.model()->data(index).toInt();
//        QStyleOptionSlider slider;
//        slider.rect = option.rect;
//        slider.minimum = 0;
//        slider.maximum = max;
//        QWidget *w = qobject_cast<QWidget*>(option.styleObject);
//        //QApplication::style()->drawComplexControl()
//        QApplication::style()->drawComplexControl(QStyle::CC_Slider,&slider,painter,w);
//       // QApplication::style()->drawControl(QStyle::CC_Slider,&slider,painter);
//    }
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override{
        QSlider *s = new QSlider(parent);
        s->setMaximum(1000);
        s->setOrientation(Qt::Horizontal);
        return s;
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const override{

        QSlider *slider = (QSlider*)editor;
        int max = index.model()->data(index).toInt();
        qDebug()<<Q_FUNC_INFO<<max;
        slider->setValue(max);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override{
        QSlider *slider = static_cast<QSlider*>(editor);
        //slider->interpretText();
        int value = slider->value();

        model->setData(index, value, Qt::EditRole);
    }
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override{
        editor->setGeometry(option.rect);
    }
};

class seriesModel:public QAbstractTableModel{
    Q_OBJECT
public:
    seriesModel(QObject *parent = nullptr){
        for(int i=0;i<6;i++) m_thresHold[i] = 0;
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override{
        return m_data.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 5;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        switch(role){
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch(index.column()){
            case 0:return m_data.at(index.row())->name();break;
            case 3:return m_data.at(index.row())->at(m_data.at(index.row())->count()-1).y();
            case 4:return m_thresHold[index.row()];break;
            }
            break;
        case Qt::CheckStateRole:
            if(index.column() == 0){
                int r = index.row();
                if(r < m_data.count()){
                    if(m_data.at(r)->isVisible())
                        return Qt::Checked;
                    else
                        return Qt::Unchecked;
                }
            }
            break;
        case Qt::BackgroundRole:
            if(index.column() == 1){
                int r = index.row();
                if(r < m_data.count()){
                    return m_data.at(r)->color();
                }
            }else if(index.column() == 3){
                QLineSeries *s = m_data.at(index.row());
                if(s->at(s->count()-1).y() > m_thresHold[index.row()]){
                    return QColor(Qt::red);
                }else{
                    return QColor(Qt::white);
                }
            }
            break;
        case Qt::DecorationRole:
            if(index.column() == 2){
                int r = index.row();
                if(r < m_data.count()){
                    QPixmap pix(100,20);
                    pix.fill(QColor(255,255,255));
                    QPainter painter;
                    painter.begin(&pix);
                    painter.setPen(m_data.at(r)->pen());
                    painter.drawLine(2,10,90,10);
                    painter.end();
                    return pix;
                }
            }
            break;
        }
        return QVariant();
    }
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override{
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
            switch(section){
            case 0:return "Series Name";break;
            case 1:return "Color";break;
            case 2:return "Line Style";break;
            case 3:return "CV";break;
            case 4:return "Threshold";break;
            }
        }
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override{
        if(role == Qt::CheckStateRole){
            bool check = false;
            if((Qt::CheckState)value.toInt() == Qt::Checked){
                check = true;
            }
            int r = index.row();
            if(r < m_data.count()){
                m_data.at(r)->setVisible(check);
            }
        }
        else if(role == Qt::BackgroundRole){
            int r = index.row();
            int c = index.column();
            if(c == 1){
                if(r < m_data.count()){
                    QColor color = value.value<QColor>();
                    m_data.at(r)->setColor(color);
                }
            }
        }
        else if(role == Qt::EditRole){
            if(index.column() == 0){
                m_data.at(index.row())->setName(value.toString());
            }else if(index.column() == 4){
                m_thresHold[index.row()] = value.toDouble();
            }
        }
        emit dataChanged(index,index);
        return true;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override{
        if(index.column() == 0)
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
        else
            return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override{
        QLineSeries *s = new QLineSeries;
        beginInsertRows(parent,row,row+count-1);
        m_data.append(s);
        endInsertRows();
        return true;

    }
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override{
        qDebug()<<Q_FUNC_INFO<<count;
        beginRemoveRows(parent,row,row+count-1);
        if(count){
            for(int i=0;i<count;i++){
                m_data.removeAt(row);
            }
        }
        endRemoveRows();
        return true;
    }

    QLineSeries *getItem(QString name){
        foreach(QLineSeries *s,m_data){
            if(s->name() == name)
                return s;
        }
        return nullptr;
    }

    QLineSeries *getItem(int row){
        if(row < m_data.count())
            return m_data.at(row);
        else
            return nullptr;
    }

    void clearSeriesData(){
        foreach(QLineSeries *s,m_data){
            s->clear();
        }
    }
private:
    QList<QLineSeries *> m_data;
    float m_thresHold[6];
};

class axisModel:public QAbstractTableModel{
    Q_OBJECT
    const QStringList header = {"AXIS","MIN","MAX"};
public:
    axisModel(QObject *parent = nullptr):QAbstractTableModel(parent){
        x1 = new QValueAxis;
        x2 = new QDateTimeAxis;
        y1 = new QValueAxis;
        y2 = new QValueAxis;
        x1->setMax(1000);
        x1->setMin(0);
        x1->setTitleText("record");
        x2->setMax(QDateTime::currentDateTime().addSecs(100));
        x2->setMin(QDateTime::currentDateTime());
        x2->setTitleText("date time");
        y1->setMax(10);
        y1->setMin(-10);
        y1->setTitleText("Amplitude");
        y2->setMax(10);
        y2->setMin(-10);
        y2->setTitleText("Amplitude");
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override{
        return 4;
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 5;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        switch(role){
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch(index.column()){
            case 0:
                switch(index.row()){
                case 0:return x1->titleText();break;
                case 1:return x2->titleText();break;
                case 2:return y1->titleText();break;
                case 3:return y2->titleText();break;
                }
                break;
            case 1:
                switch(index.row()){
                case 0:return x1->min();break;
                case 1:return x2->min();break;
                case 2:return y1->min();break;
                case 3:return y2->min();break;
                }
                break;
            case 2:
                switch(index.row()){
                case 0:return x1->max();break;
                case 1:return x2->max();break;
                case 2:return y1->max();break;
                case 3:return y2->max();break;
                }
                break;
            case 3:
                if(index.row() < 2) return m_recordCount[index.row()];
                else return 0;
                break;
            default:
                return QVariant();
                break;
            }
            break;
        case Qt::CheckStateRole:
            if(index.column() == 0){
                switch(index.row()){
                case 0:return x1->isVisible()?Qt::Checked:Qt::Unchecked;break;
                case 1:return x2->isVisible()?Qt::Checked:Qt::Unchecked;break;
                case 2:return y1->isVisible()?Qt::Checked:Qt::Unchecked;break;
                case 3:return y2->isVisible()?Qt::Checked:Qt::Unchecked;break;
                default:return QVariant();break;
                }
           }
            break;
//        case Qt::BackgroundRole:
//            if(index.column() == 2){
//                int r = index.row();
//                if(r < m_data.count()){
//                    return m_data.at(r)->color();
//                }
//            }
//            break;
//        case Qt::DecorationRole:
//            if(index.column() == 3){
//                int r = index.row();
//                if(r < m_data.count()){
//                    QPixmap pix(100,20);
//                    pix.fill(QColor(255,255,255));
//                    QPainter painter;
//                    painter.begin(&pix);
//                    painter.setPen(m_data.at(r)->pen());
//                    painter.drawLine(2,10,90,10);
//                    painter.end();
//                    return pix;
//                }
//            }
//            break;
        }
        return QVariant();
    }
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override{
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
            switch(section){
            case 0:return "AXIS";break;
            case 1:return "MIN";break;
            case 2:return "MAX";break;
            }
        }
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override{
        if(role == Qt::CheckStateRole){
            bool check = false;
            if((Qt::CheckState)value.toInt() == Qt::Checked){
                check = true;
            }
            qDebug()<<Q_FUNC_INFO<<check;
            if(index.column() == 0){
                switch(index.row()){
                case 0:x1->setVisible(check);break;
                case 1:x2->setVisible(check);break;
                case 2:y1->setVisible(check);break;
                case 3:y2->setVisible(check);break;
                }
            }
        }
        if(role == Qt::EditRole){
            switch(index.column()){
            case 0:
                switch(index.row()){
                case 0:x1->setTitleText(value.toString());break;
                case 1:x2->setTitleText(value.toString());break;
                case 2:y1->setTitleText(value.toString());break;
                case 3:y2->setTitleText(value.toString());break;
                }
                break;
            case 1:
                switch(index.row()){
                case 0:x1->setMin(value.toDouble());break;
                case 1:x2->setMin(value.toDateTime());break;
                case 2:y1->setMin(value.toDouble());break;
                case 3:y2->setMin(value.toDouble());break;
                }
                break;
            case 2:
                switch(index.row()){
                case 0:x1->setMax(value.toDouble());break;
                case 1:x2->setMax(value.toDateTime());break;
                case 2:y1->setMax(value.toDouble());break;
                case 3:y2->setMax(value.toDouble());break;
                }
                break;
            }
        }
//        else if(role == Qt::BackgroundRole){
//            int r = index.row();
//            if(r < m_data.count()){
//                QColor color = value.value<QColor>();
//                m_data.at(r)->setColor(color);
//            }
//        }
        emit dataChanged(index,index);
        return true;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override{
        if(index.column() == 0)
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
        else
            return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override{
//        QAbstractAxis *s = new QAbstractAxis();
//        beginInsertRows(parent,row,row+count-1);
//        m_data.append(s);
//        endInsertRows();
        return true;

    }
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override{}

    QAbstractAxis *getItem(QString name){
        foreach(QAbstractAxis *s,m_data){
            if(s->titleText() == name)
                return s;
        }
        return nullptr;
    }

    QAbstractAxis *getItem(int row){
        switch(row){
        case 0:return (QValueAxis*)x1;break;
        case 1:return x2;break;
        case 2:return (QValueAxis*)y1;break;
        case 3:return (QValueAxis*)y2;break;
        default:return nullptr;break;
        }
    }

    void setSize(int id, double v){
        if (id < 2){
            m_recordCount[id] = v;
            emit dataChanged(this->index(id,3),this->index(id,3));
        }
    }


private:
    QList<QAbstractAxis*> m_data;
    QValueAxis *x1;
    QValueAxis *y1,*y2;
    QDateTimeAxis *x2;
    float m_recordCount[4];

};

class chartView2 : public QDialog
{
    Q_OBJECT

public:
    explicit chartView2(QWidget *parent = nullptr);
    ~chartView2();
    void clearSeries();
    void clearData();
    void addValueAxis(Qt::Alignment align,QString title="", float min=-10, float max=10);
    void addDateTimeAxis(Qt::Alignment align,QString title="", QDateTime min=QDateTime::currentDateTime(), QDateTime max=QDateTime::currentDateTime().addSecs(10));
    QLineSeries *addSeries(QString title="",int axis1=0, int axis2=2);
    QLineSeries *addTimeSeries(QString title="");
    QLineSeries *findSeries(QString title);
    void setSeriesData(QString name, QVector<QPointF> data);
    void appendSeriesData(QStringList name, QVector<QPointF> data);
    void setCaption(QString cap);
    void setAxisScale(int id, float min, float max);
    void setAxisScale(int id, QDateTime min, QDateTime max);
    void setAxisBuffer(int id, int sz);

signals:
    void requestData(int,int);

private slots:
    void on_pbHBand_clicked();

    void on_pbVBand_clicked();

    void on_pbRBand_clicked();

    void on_tableView_clicked(const QModelIndex &index);

    void on_pbRstView_clicked();

    void on_pbCapture_clicked();

    void handleAxisModelChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight);//, QVector<int> &roles);

private:
    void showChartContextMenu(const QPoint &);
    void handleContextClick();
private:
    Ui::chartView2 *ui;
    QList<QLineSeries *> m_series;
    seriesModel *mModel;
    axisModel *m_axisModel;
    int m_recordSize;
};

#endif // CHARTVIEW2_H
