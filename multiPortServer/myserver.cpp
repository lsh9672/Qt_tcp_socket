#pragma once
#include "myserver.h"
#include <QtNetwork>


MyServer::MyServer(QObject *parent):
    QTcpServer(parent)
{
    ;
}
MyServer::~MyServer()
{
    this->close();
    this->deleteLater();
}

void MyServer::startServer(qint32 listen_port)
{

    if(this->listen(QHostAddress::Any,listen_port))
    {

        emit listenSuccess("Listening to port",this->serverPort());
        qDebug() << "Listening to port" << listen_port << "....";
    }
    else
    {
        emit listenFail("Could not start server");
        qDebug() << "Could not start server";
    }
}



