#ifndef MYTHREAD_H
#define MYTHREAD_H
#pragma once
#include "mainwindow.h"

#include <QThread>
#include <QDebug>
#include <QtNetwork>
#include <QString>


class MyThread : public QThread
{
    Q_OBJECT

public:
    //스레드 ID를 생성자의 첫번째 매개변수로 추가
    explicit MyThread(qintptr ID, QObject *parent=0);
    void run();
    bool writeData(QByteArray mydata);

signals:
    //에러처리를 위한 시그널
    void error(QTcpSocket::SocketError socketerror);

    //연결이 끊어졌을때 ui에 알리기 위한 시그널 - 인자: 소켓 설명자를 받아서 어떤 소켓이 연결을 끊었는지 표시
    void sigDisconnected(qintptr socketDescriptor);

    //client로 부터 데이터를 읽었을 때 gui화면 출력을 위해 데이터와 socketDescriptor를 넘겨줌
    void sigReadData(qintptr socketDescriptor,QString rData);

    //다시 client 쪽으로 write 성공했을때
    void sigWriteSuccess(qintptr socketDescriptor);

    //write실패시
    void sigWriteFail(qintptr socketDescriptor);

    //연결정보 모니터링을 위한 gui에 표시를 위한 시그널 - 소켓 기술자,클라이언트 ip, 클라이언트 port, 접속시간
    void sigClientInfo(qintptr socketInfo,QString connectIp,quint16 connectPort,time_t connectTime);

    //연결이 끊어졌을때 모니터링 정보 삭제
    void sigClientDel(qintptr socketInfo);


public slots:
    //readyRead 시그널 발생시 처리
    void readData();

    //disconnected 시그널 발생시 처리
    void disconnected();

    //gui에서 전송버튼을 누를경우 발생하는 시그널처리
    void broadcast_data_send(QByteArray bdata);



private:
    //서버로 연결요청이 들어오면 처리하기 위한 소켓
     QTcpSocket *client_socket;

     //소켓 기술자(소켓을 식별할수 있는 값)
     qintptr socketDescriptor;

     //데이터가 전부 write되었는지 확인
     bool send_flag;

     //클라이언트의 상태정보 저장
     //소켓 기술자(식별할수 있는 값)
     qintptr TsocketInfo;
     //클라이언트의 ip
     QString TconnectIp;
     //클라이언트의 port
     quint16 TconnectPort;
     //연결시간(단위: 초)
     time_t TconnectTime;

};

#endif // MYTHREAD_H

