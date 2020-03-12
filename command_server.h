#ifndef COMMAND_SERVER_H
#define COMMAND_SERVER_H

#include <QObject>
#include "snode_interface.h"

class QTcpServer;
class QTcpSocket;
class command_server : public QObject
{
    Q_OBJECT
public:
    explicit command_server(QObject *parent = nullptr);
    ~command_server();
    bool listen(int port);
    void close();
signals:
    void message_received(QTcpSocket *s, QByteArray b);
    void interface_add(snode_interface::commType, QString, int);
    void add_interface(int, QString, int);
    void setParam(QByteArray json);
    void getParam(QByteArray json);
    void message_received(QByteArray);
public slots:
    void send_data(QTcpSocket *s, QByteArray &b);

private slots:
    void on_message_in();
    void on_newconnection();
    void on_accept_error();
    void on_client_disconnect();
    void onRecordReady(QVector<float> record);
    void onStreamReady(QByteArray b);
private:
    void parse_json(QByteArray b);
private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    QByteArray m_recv;
};

#endif // COMMAND_SERVER_H
