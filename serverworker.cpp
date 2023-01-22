#include "serverworker.h"
//global TODO: use interface for server
//global TODO: send JSON instead str
ServerWorker::ServerWorker()
{
    //TODO: load options from file

    //initialize config
   config = new TConfig;
   //initialize resources
   resourceController = new TResourceController(*config);

   if(this->listen(QHostAddress::Any,  8080))
       qDebug() << "server started on port" << 8080;
   else
       qDebug()<< "server not started";
}

void ServerWorker::incomingConnection(qintptr socketDescriptor){
    //TODO : add connectedUsers and userSockets fields to ServerWorker
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &ServerWorker::ReadMessage);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater );

    //maybe incorrect place (beter after authentification check)
    sockets.push_back(socket);

    qDebug() << "connected client" << socketDescriptor;
}


void ServerWorker::SendMessage(const QJsonObject& json, QTcpSocket* socket){
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    // we send the message to the socket in the exact same way we did in the client
    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_6_4);
    socketStream << jsonData;
}


void ServerWorker::ReadMessage()
{
    socket = (QTcpSocket*)sender();
    QByteArray jsonData;
    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_6_4);

    while (true) {
        socketStream.startTransaction();
        socketStream >> jsonData;

        QJsonObject response;

        if (socketStream.commitTransaction()) {
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject()){
                    QJsonObject request = jsonDoc.object();

                    //TODO: проверить, это запрос на авторизацию, на получение ресурса или невалидный
                    qDebug() << "json received:"  << request;
                    // authentification check - not finaly version
                    if(!resourceController->CheckUserAccess(request["username"].toString())){
                        socket->disconnectFromHost();
                        qDebug() << "connection refused, invalid username" << request["username"].toString();
                        break;
                    }

                    //TODO 1: try get resource
                    //TODO 2: form response
                    QString resources = request["resources"].toString();
                    for(quint32 it = 0; it < resources.size(); it++){
                        if(resources[it] == '1'){
                            bool reserve_result = resourceController->ReserveResource(request["username"].toString(), it, request["time"].toInteger());
                            response[QStringLiteral("username")] = request["username"].toString();
                            response[QStringLiteral("resource")] = QJsonValue::fromVariant(it+1);
                            response[QStringLiteral("status")] = reserve_result;
                            SendMessage(response, socket);
                        }
                    }
                }
                else
                    qDebug() << ("Invalid message: ") + QString::fromUtf8(jsonData);
                    //emit logMessage(QLatin1String("Invalid message: ") + QString::fromUtf8(jsonData)); //notify the server of invalid data
            } else {
                //emit logMessage(QLatin1String("Invalid message: ") + QString::fromUtf8(jsonData)); //notify the server of invalid data
            }
        } else {
            break;
        }
    }
}
