#ifndef MYSERVER_H
#define MYSERVER_H
#include <QTcpServer>
#include "myThread.h"


class MyServer : public QTcpServer
{

    Q_OBJECT

public:
    explicit MyServer(QObject *parent=0);
    void startServer(qint32 listen_port);
signals:
    //클라이언트가 접속했을때 UI에 표시를 위하여 시그널을 보냄-인자로 접속한 소켓의 설명자(번호)
    void connectClient(qintptr socketDescriptor);

    //listen성공
    void listenSuccess(QString slog,quint16 listen_port);

    //listen 실패
    void listenFail(QString flog);

public slots:


protected:
    void incomingConnection(qintptr socketDescriptor);


private:
};

#endif // MYSERVER_H

