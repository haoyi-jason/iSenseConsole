#include "filemanage.h"
#include "ui_filemanage.h"

fileManage::fileManage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fileManage)
{
    ui->setupUi(this);

    fileListModel *m = new fileListModel;
    ui->tableView->setModel(m);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setItemDelegateForColumn(3,new ProgressDelegate(this));

    //file_info *info = new file_info("\/","123.txt\0",123);
    //m->insertItem(info);

    bReadFile = false;


    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::NoDotAndDotDot |QDir::AllDirs);
    QString mPath = QDir::currentPath()+"/";
    qDebug()<<"Current Path:"<<mPath;
    dirModel->setRootPath(mPath);
    ui->treeView->setModel(dirModel);
    fileModel = new QFileSystemModel(this);
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    ui->listView->setModel(fileModel);

    cv = new chartView2();
    cv->hide();

    connect(cv,&chartView2::requestData,this,&fileManage::plotData);

    mSensorID = -1;
    m_rootPath = "";
}

fileManage::~fileManage()
{
    delete ui;
}

int fileManage::parsePacket(quint8 pid,QByteArray b)
{
    qDebug()<<Q_FUNC_INFO<<"PID:"<<pid;
    quint8 cmd = pid & 0xf;
    switch(cmd){
    case FILE_LIST:
    {
        qDebug()<<"List file";
        fileListModel *m = (fileListModel*)ui->tableView->model();
        //qDebug()<<"MODEL SIZE:"<<m->rowCount();
        if((pid & 0x10) == 0x10){
            m->insertRows(m->rowCount(),1);
            int row = m->rowCount()-1;
            file_info *info = m->getItem(row);
            if(info != nullptr){
                info->fromByteArray(b);
                emit file_op(FILE_LIST,b);
            }
        }
        ui->tableView->viewport()->update();
    }
        break;
    case FILE_READ:
        if(bReadFile){
            if((pid & 0x10) == 0x10){
                //qDebug()<<"file op OK";
                file_op_t fop;
                fop.fromByteArray(b);
                if(fop.size()){
                    QByteArray bb = b.mid(40,b.size()-40);
                    qint64 sz = m_fop.writeFile(bb);
                    m_finfo->setBytesTransferred(sz);
                    ui->tableView->viewport()->update();
                    if(sz == m_finfo->size()){
                        //qDebug()<<"File transfer Done";
                        m_fop.closeFile();
                        bReadFile =false;
                    }
                    else{
                        emit file_op(FILE_READ,m_fop.toByteArray());
                    }
    //            }
    //            if(fop.size()==256){
                    //qDebug()<<"Get next packet";
                    //m_fop.setOffset(fop.offset()+fop.size());
                    //QByteArray bb = m_fop.toByteArray();
                }
                else{
                    m_fop.closeFile();
                    bReadFile =false;
                }
            }
            else{
                //qDebug()<<"file op ERR";
                m_fop.closeFile();
            }
        }
        break;
    case FILE_REMOVE:
    {
        fileListModel *m = (fileListModel*)ui->tableView->model();
        QModelIndexList lst = ui->tableView->selectionModel()->selectedRows();
        m->removeRows(lst.at(0).row(),1);
    }
        break;
    }

    return 0;
}

void fileManage::on_pbls_clicked()
{
    file_info info("/");
    emit file_op(FILE_OP_CHDIR|FILE_LIST,info.toByteArray());
}

void fileManage::on_pbTransfer_clicked()
{
    QModelIndexList lst = ui->tableView->selectionModel()->selectedRows();
    if(lst.size() > 0){
        fileListModel *m = (fileListModel*)ui->tableView->model();
        m_finfo = m->getItem(lst.at(0).row());
        file_op_t fop(m_finfo->fileName());
        m_fop.setFileName(m_finfo->fileName());
        m_fop.setOffset(0);
        m_fop.setSize(256);
        if(m_fop.openFile(m_rootPath)){
            qDebug()<<"Start file transfer"<<m_fop.fileName();
            bReadFile = true;
            emit file_op(FILE_READ,m_fop.toByteArray());
        }else{
            qDebug()<<"fail to open local file";
        }
    }
}

void fileManage::on_pbRemove_clicked()
{
    QModelIndexList lst = ui->tableView->selectionModel()->selectedRows();
    if(lst.size() > 0){
        fileListModel *m = (fileListModel*)ui->tableView->model();
        m_finfo = m->getItem(lst.at(0).row());
        m_fop.setFileName(m_finfo->fileName());
        m_fop.setOffset(0);
        m_fop.setSize(256);
        emit file_op(FILE_REMOVE,m_fop.toByteArray());
    }
}

void fileManage::on_treeView_clicked(const QModelIndex &index)
{
    QString mPath = dirModel->fileInfo(index).absoluteFilePath();
    ui->listView->setRootIndex(fileModel->setRootPath(mPath));
    m_rootPath = mPath;
}

void fileManage::on_listView_doubleClicked(const QModelIndex &index)
{
    QString mPath = fileModel->fileInfo(index).absoluteFilePath();
    m_suffix = fileModel->fileInfo(index).completeSuffix();
    //qDebug()<<"Clicked file:"<<mPath << " suffix:"<<suffix;
    if(QFile(mPath).exists()){
        if(mFile.isOpen()){
            mFile.close();
        }
        mFile.setFileName(mPath);
        if(m_suffix == "bin"){
            if(mFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                QByteArray tmp = mFile.read(512);
                ui->textEdit->setText(tmp);
                if(ui->textEdit->toPlainText().contains("ADXL")){
                    mSensorID=1;
                    cv->clearSeries();
                    cv->addSeries("AX");
                    cv->addSeries("AY");
                    cv->addSeries("AZ");
                    cv->setAxisScale(0,0,1000);
                    cv->setAxisScale(2,-10,10);
                    int rec = (mFile.size()-512)/9;
                    cv->setAxisBuffer(0,rec);
                }
                else if(ui->textEdit->toPlainText().contains("BMI")){
                    mSensorID = 2;
                    cv->clearSeries();
                    cv->addSeries("AX");
                    cv->addSeries("AY");
                    cv->addSeries("AZ");
                    cv->addSeries("GX",0,3);
                    cv->addSeries("GY",0,3);
                    cv->addSeries("GZ",0,3);
                    cv->setAxisScale(0,0,1000);
                    cv->setAxisScale(2,-10,10);
                    cv->setAxisScale(3,-2000,2000);
                    int rec = (mFile.size()-512)/12;
                    cv->setAxisBuffer(0,rec);
                }
            }else{
                qDebug()<<"File open error";
            }
            mFile.close();
        }
        else if(m_suffix == "csv"){

            cv->clearSeries();
            cv->addSeries("X-PEAK",1,2);
            cv->addSeries("Y-PEAK",1,2);
            cv->addSeries("Z-PEAK",1,2);
            cv->addSeries("X-RMS",1,2);
            cv->addSeries("Y-RMS",1,2);
            cv->addSeries("Z-RMS",1,2);
            cv->setAxisScale(1,QDateTime::currentDateTime(),QDateTime::currentDateTime().addDays(100));
            cv->setAxisScale(2,-10,10);
//            cv->setAxisBuffer(0,rec);
        }
        //updateSeries(0,1000);
    }else{
        qDebug()<<"File not exist";
    }

}

void fileManage::plotData(int start, int end)
{
    QString header = ui->textEdit->toPlainText();
    QRegExp rx("(\\d+)(?:\\s*)(G)");
    QRegExp rx2("(\\d+)(?:\\s*)(DPS)");
    QRegExp rx3("(\\d+)(?:\\s*)(SPS)");

    float ar=1.0,gr=1.0,rate=100;
    if(rx.indexIn(header) != -1){
      ar = rx.cap(1).toDouble();
    }

    if(rx2.indexIn(header) != -1){
      gr = rx2.cap(1).toDouble();
    }
    if(rx3.indexIn(header) != -1){
      rate = rx3.cap(1).toDouble();
    }
    //return;

    if(ui->textEdit->toPlainText().contains("ADXL")){
        mSensorID=1;
    }
    else if(ui->textEdit->toPlainText().contains("BMI")){
        mSensorID = 2;
    }
    qDebug()<<"Sensor ID:"<<mSensorID;
    if(mSensorID == -1) return;
    int recordSize = 0;
    if(mSensorID == 1)
        recordSize = 9;
    else if(mSensorID == 2)
        recordSize = 12;

    if(mFile.open(QIODevice::ReadOnly)){
        int s = 512+start*recordSize;
        int e = s+end*recordSize;
        mFile.seek(s);
        qDebug()<<QString("Read record from %1 to %2").arg(s).arg(e);
        QByteArray b = mFile.read(e-s);

        snode_vss_codec codec;
        QVector<int> ret = codec.parseStreamEx(b,mSensorID);
        qDebug()<<Q_FUNC_INFO<<"Parse record:"<<ret.size();
        switch(mSensorID){
        case 1:
        {
            QVector<QPointF> d[3];
            ar /= (1 << 19);
            for(int i=0;i<ret.size()/3;i++){
                for(int j=0;j<3;j++)
                    d[j].append(QPointF(i,ret[i*3+j]*ar));
            }
            cv->setSeriesData("X",d[0]);
            cv->setSeriesData("Y",d[1]);
            cv->setSeriesData("Z",d[2]);
        }
            break;
        case 2:
        {
            QVector<QPointF> d[6];
            ar /= (1 << 15);
            gr /= (1 << 15);
            for(int i=0;i<ret.size()/6;i++){
                for(int j=0;j<3;j++)
                    d[j].append(QPointF(i,ret[i*6+j]*gr));
                for(int j=3;j<6;j++)
                    d[j].append(QPointF(i,ret[i*6+j]*ar));
            }
            cv->setSeriesData("GX",d[0]);
            cv->setSeriesData("GY",d[1]);
            cv->setSeriesData("GZ",d[2]);
            cv->setSeriesData("AX",d[3]);
            cv->setSeriesData("AY",d[4]);
            cv->setSeriesData("AZ",d[5]);
        }
            break;
        }

    }
    qDebug()<<"close file";
    mFile.close();
}

void fileManage::on_pbShowChart_clicked()
{
    if(cv) cv->show();
}

void fileManage::on_pbClearChart_clicked()
{
    if(cv) cv->clearData();
}

void fileManage::on_pbPlot_clicked()
{
    int start = ui->leStart->text().toInt();
    int end = ui->leEnd->text().toInt();
    plotData(start,end);
}

void fileManage::setCaption(QString cap)
{
    this->setWindowTitle(cap);
    //this->setCaption(cap);
    cv->setCaption(cap);
}


void fileManage::on_pbParse_clicked()
{

    QString str = ui->teParseSrc->toPlainText();
    QString dst = "";
    if(!str.isEmpty()){
        QByteArray ab = QByteArray::fromHex(str.toLatin1());
        qDebug()<<ab.count()<<ab;
        snode_vss_codec codec;
        QVector<int> ret = codec.parseStreamEx(ab.mid(8,ab.count()-8),1);
        QVector<QPointF> d[3];
        float ar = 4.0/(1 << 19);
        for(int i=0;i<ret.size()/3;i++){
            for(int j=0;j<3;j++){
                d[j].append(QPointF(i,ret[i*3+j]*ar));

            }
            dst+= QString("X:%1\tY:%2\tZ:%3\n").arg(d[0].last().y(),8,'f',5,'0').arg(d[1].last().y(),8,'f',5,'0').arg(d[2].last().y(),8,'f',5,'0');
        }
        ui->teParseDest->setText(dst);
        cv->setSeriesData("X",d[0]);
        cv->setSeriesData("Y",d[1]);
        cv->setSeriesData("Z",d[2]);
        cv->show();
    }
}
