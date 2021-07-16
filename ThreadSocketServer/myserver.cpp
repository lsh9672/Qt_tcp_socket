#pragma once
#include "myserver.h"
#include <ctime>


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
        emit listenSuccess("Listening to port",this->serverPort());
        qDebug() << "Listening to port" << listen_port << "....";
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor)
{
    time_t timer = time(NULL);

    /*
    struct tm* t = localtime(&timer);
    qDebug() << "년 : " <<t->tm_year + 1900;
    qDebug() << "월 : " <<t->tm_mon + 1;
    qDebug() << "일 : " <<t->tm_mday;
    qDebug() << "시 : " <<t->tm_hour;
    qDebug() << "분 : " <<t->tm_min;
    qDebug() << "초 : " <<t->tm_sec;
    */

    qDebug() << socketDescriptor <<"Connecting...";

    //연결이 들어오면 신호를 보내고 ui클래스 쪽에서 이 신호를 받아서 스레드를 만들어서 처리함.
    emit connectClient(socketDescriptor,timer);


}

//timer는 유닉스 타임으로 1970.1.1.00:00:00 부터 시작하여 현재까지의 초
