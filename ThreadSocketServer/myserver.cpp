#include "myserver.h"
#include "myThread.h"

MyServer::MyServer(QObject *parent):
    QTcpServer(parent)
{
}

void MyServer::startServer()
{
    int port = 9999;
    if(!this->listen(QHostAddress::Any,port))
    {
        qDebug() << "Could not sart server";
    }
    else
    {
        qDebug() << "Listening to port" << port << "....";
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << socketDescriptor <<"Connecting...";
    emit connectClient(socketDescriptor);

    MyThread *thread = new MyThread(socketDescriptor,this);
    //thread->moveToThread(thread);

    connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));

    thread->start();

    qDebug() << "running State : " << thread->isRunning();


}
