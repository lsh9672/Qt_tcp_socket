#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QtNetwork>
#include <QTcpServer>
#include "myserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



signals:
    void broadcast_data(QByteArray bdata);


public slots:

    //클라이언트 연결시 ui표시
    void showConnect(qintptr socketDescriptor);

    //소켓 연결해제시에 UI 표시를 위한 slot
    void showDisconnected(qintptr socketDescriptor);

    //읽어온 데이터를 UI에 표시하기 위한 slot
    void showReadData(qintptr socketDescriptor,QString rData);

    //write 성공여부
    void showWriteSuccess(qintptr socketDescriptor);

    //write 실패했을때
    void showWriteFail(qintptr socketDescriptor);

    //소켓에러
    void showSocketError(QTcpSocket::SocketError socketerror);

    //listen성공
    void showListenSuccess(QString slog,quint16 listen_port);

    //listen 실패
    void showListenFail(QString flog);

    //클라이언트 정보 모니터링
    void showClientInfo(qintptr socketInfo,QString connectIp,quint16 connectPort,time_t connectTime);

    //연결이 끊어졌을때 저장한 클라이언트 정보 삭제
    void delClientInfo(qintptr TsocketInfo);

    void showFileSave(QString filePath);



private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_clicked();

private:

    Ui::MainWindow *ui;
    QList<qintptr> infoList;
    qint32 port;

};
#endif // MAINWINDOW_H
