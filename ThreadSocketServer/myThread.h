#ifndef MYTHREAD_H
#define MYTHREAD_H

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
    void showDisconnected(qintptr socketDescriptor);

    //client로 부터 데이터를 읽었을 때 gui화면 출력을 위해 데이터와 socketDescriptor를 넘겨줌
    void showReadData(qintptr socketDescriptor,QString rData);

    //다시 client 쪽으로 write 성공했을때
    void showWriteSuccess(qintptr socketDescriptor);


    //write실패시
    void showWriteFail(qintptr socketDescriptor);

public slots:
    //readyRead 시그널 발생시 처리
    void readData();
    //disconnected 시그널 발생시 처리
    void disconnected();


private:
    //서버로 연결요청이 들어오면 처리하기 위한 소켓
     QTcpSocket *client_socket;

     //소켓 설명자(포인터 타입- 소켓 객체의 주소), 소켓을 나타낸다 생각.
     qintptr socketDescriptor;
     bool send_flag;

};


#endif // MYTHREAD_H

