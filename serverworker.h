#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>
#include <ResourceController.h>

class ServerWorker:public QTcpServer
{
    Q_OBJECT
public:
    ServerWorker();
private:
    QVector <QTcpSocket*> sockets;
    QTcpSocket *socket;

    QVector <QString*> connectedUsers;
    QVector <QPair<QString*, QTcpSocket*>> usersSockets;

    TConfig* config;
    TResourceController* resourceController;


    void SendMessage(const QJsonObject &json, QTcpSocket* socket);

public slots:
    void incomingConnection(qintptr socketDescriptor);
    void ReadMessage();
};

#endif // SERVERWORKER_H
