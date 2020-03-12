#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include "command_server.h"

command_server::command_server(QObject *parent) : QObject(parent)
{
    m_server = nullptr;
}
command_server::~command_server()
{
    if(m_server && m_server->isListening()){
        m_server->close();
        delete m_server;
    }
}

bool command_server::listen(int port)
{
    qDebug()<<Q_FUNC_INFO<<"Listen on port:"<<port;
    if(m_server != nullptr) return false;
    m_server = new QTcpServer(this);
    if(!m_server->listen(QHostAddress::LocalHost,port)){
        return false;
    }
    m_server->setMaxPendingConnections(2);
    connect(m_server,&QTcpServer::newConnection,this,&command_server::on_newconnection);
    connect(m_server,&QTcpServer::acceptError,this,&command_server::on_accept_error);
    return true;
}

void command_server::close()
{
    qDebug()<<Q_FUNC_INFO;
    if(m_server == nullptr) return;
    if(m_server->isListening()) m_server->close();
    m_server->deleteLater();
    m_server = nullptr;
}

void command_server::send_data(QTcpSocket *s, QByteArray &b)
{

}

void command_server::parse_json(QByteArray b)
{
    m_recv.append(b);
    qDebug()<<"RECV:"<<QString(m_recv);
    QJsonParseError e;
    QJsonDocument d = QJsonDocument::fromJson(m_recv,&e);
    if(d.isNull()){
        qDebug()<<Q_FUNC_INFO<<"Parse Json Error:"<<e.errorString();
        return;
    }

    QJsonObject json_obj = d.object();

    qDebug()<<d.toJson().toStdString().data();
    qDebug()<<"-----------------------------";
    QJsonObject obj2 = json_obj["Interface"].toObject();

    if(obj2.contains("BAUDRATE")){
        qDebug()<<"OBJ type:"<<obj2.value("BAUDRATE").type();
        QString hostname = obj2.value("HOSTNAME").toString();
        int baud = obj2.value("BAUDRATE").toInt();
        emit add_interface(0,hostname,baud);
    }
    else if(obj2.contains("PORT")){
        emit add_interface(1,obj2.value("HOSTNAME").toString(), obj2.value("PORT").toInt());
    }
//    foreach(const QString& key,obj2.keys()){
//        qDebug()<<"Key="<<key<<", Value="<<obj2.value(key).toString();
//    }

    m_recv.clear();
}

void command_server::on_message_in()
{
    QTcpSocket *s = (QTcpSocket*)sender();
    QByteArray b = s->readAll();
    //parse_json(b);
    emit message_received(b);
//    qDebug()<<Q_FUNC_INFO<<" Receive data from:"<<s->peerAddress()<<": "<<b.size();
//    s->write(b);
//    emit message_received(s,b);
//    QString cmd = QString(b);
//    QStringList sl = cmd.split(',');
//    if(sl.size()){
//        if(sl.at(0) == "ADDIF" && sl.size() == 3){
//            if(sl.at(1).contains("COM")){
//                qDebug()<<QString("Add serial interface, host= %1,port=%2").arg(sl.at(1)).arg(sl.at(2));
//                emit interface_add(snode_interface::COMM_SERIAL,sl.at(0),sl.at(1).toInt());
//            }
//            else if(!QHostAddress(sl.at(1)).isNull()){
//                qDebug()<<QString("Add lan interface, host= %1,port=%2").arg(sl.at(1)).arg(sl.at(2));
//                emit interface_add(snode_interface::COMM_SERIAL,sl.at(0),sl.at(1).toInt());
//            }
//        }
//    }
}

void command_server::on_newconnection()
{
    qDebug()<<Q_FUNC_INFO<<"New Connect arrived:";
    QTcpSocket *s = m_server->nextPendingConnection();
    connect(s,&QTcpSocket::readyRead,this,&command_server::on_message_in);
    connect(s,&QTcpSocket::disconnected,this,&command_server::on_client_disconnect);
    m_recv.clear();
}

void command_server::on_accept_error()
{

}

void command_server::on_client_disconnect()
{
    QTcpSocket *s = (QTcpSocket*)sender();
    qDebug()<<Q_FUNC_INFO<<" Client disconnect:"<<s->peerAddress();
    s->deleteLater();
}

void command_server::onRecordReady(QVector<float> rec)
{

}

void command_server::onStreamReady(QByteArray b)
{

}
