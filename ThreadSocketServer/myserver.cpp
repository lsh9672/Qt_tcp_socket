#pragma once
#include "myserver.h"
#include <ctime>


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

    if(!this->listen(QHostAddress::Any,listen_port))
    {
        emit listenFail("Could not start server");
        qDebug() << "Could not start server";
    }
    else
    {
        emit listenSuccess("Listening to port",this->serverPort());
        qDebug() << "Listening to port" << listen_port << "....";
    }
    connect(this,SIGNAL(newConnection()),this,SLOT(testSlot()));
}

void MyServer::incomingConnection(qintptr socketDescriptor)
{

    qDebug() << socketDescriptor <<"Connecting...";
    emit connectClient(socketDescriptor);

    //연결이 들어오면 신호를 보내고 ui클래스 쪽에서 이 신호를 받아서 스레드를 만들어서 처리함.



}


//timer는 유닉스 타임으로 1970.1.1.00:00:00 부터 시작하여 현재까지의 초
