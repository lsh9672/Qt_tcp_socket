#ifndef MYSERVER_H
#define MYSERVER_H
#include <QTcpServer>
#include <QString>
#include <QDebug>


class MyServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit MyServer(QObject *parent=0);
    void startServer(qint32 listen_port);
    ~MyServer();
signals:
    //listen성공
    void listenSuccess(QString slog,quint16 listen_port);

    //listen 실패
    void listenFail(QString flog);

public slots:

private:
};

#endif // MYSERVER_H
