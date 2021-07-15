#pragma once
#include "myserver.h"


MyServer::MyServer(QObject *parent):
    QTcpServer(parent)
{
    ;
}

void MyServer::startServer(qint32 listen_port)
{

    if(!this->listen(QHostAddress::Any,listen_port))
    {
        emit listenFail("Could not start server");
        qDebug() << "Could not start server";

    }
    else
    {
        emit listenSuccess("Listening to port",this->serverAddress(),this->serverPort());
        qDebug() << "Listening to port" << listen_port << "....";

    }
}

void MyServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << socketDescriptor <<"Connecting...";



    //연결이 들어오면 신호를 보내고 ui클래스 쪽에서 이 신호를 받아서 스레드를 만들어서 처리함.
    emit connectClient(socketDescriptor);


}
