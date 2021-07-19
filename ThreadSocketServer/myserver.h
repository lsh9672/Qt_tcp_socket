#ifndef MYSERVER_H
#define MYSERVER_H
#include <QTcpServer>
#include "myThread.h"
#include "mainwindow.h"

class MyServer : public QTcpServer
{
    //qt의 signal-slot등과 같은 meta-obect system을 사용하려고 하는 클래스에서 선언해줘야 하는 매크로이다.(이러한 기능을 사용하지 않아도 이 매크로를 사용하는 것을 권장한다.)
    //meta-object system은 object간 통신매커니즘을 위한 signal-slot,런타임 타입 정보, 동적 property 시스템을 제공한다.
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

