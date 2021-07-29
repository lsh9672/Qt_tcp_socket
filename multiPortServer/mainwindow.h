#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QtNetwork>
#include <QTcpServer>
#include "myserver.h"
#include <QFile>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //클라이언트 소켓 정보를 리스트에 추가하는 함수
    void appendSocketInfo(QTcpSocket *client_socket);

    //클라이언트 정보 모니터링
    void showClientInfo(qintptr socketInfo,QString connectIp,quint16 connectPort,quint16 work_port,time_t connectTime);


signals:


public slots:

    //소켓에러
    void showSocketError(QAbstractSocket::SocketError socketError);

    //listen성공
    void showListenSuccess(QString slog,quint16 listen_port);

    //listen 실패
    void showListenFail(QString flog);

    //연결이 끊어졌을때 저장한 클라이언트 정보 삭제
    void delClientInfo();

    //클라이언트 접속시에 처리할 슬롯
    void newConnection();

    //readyRead 시그널 발생시 처리
    void readData();

private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_clicked();

private:

    Ui::MainWindow *ui;
    //소켓 기술자 정보를 저장하는 리스트 - 모니터링 테이블 업데이트를 위해
    QList<qintptr> infoList;
    //qint32 port;
    //브로드 캐스트 또는 선택한 클라이언트에게 데이터 전달을 위해 소켓 저장 - set자료형은 해쉬맵 형식으로 데이터 조회가 빠름
    QSet<QTcpSocket*> connection_socket;
    //key-value 형식으로 소켓에 대응되는 소켓기술자를 저장 - 연결이 끊어졌을떄 테이블에서 정보 삭제를 위해
    QHash<QTcpSocket*,qintptr> connection_socketDescriptor;

    //종료시 서버소켓 close를 위해
    QSet<QTcpServer*> connection_server;

};
#endif // MAINWINDOW_H
