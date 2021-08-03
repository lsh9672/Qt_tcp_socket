#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QtNetwork>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    //연결체크용 함수, 인자로 연결할 서버의 ip주소를 받음
    bool connectCheck(QString server_ip,qint32 server_port);

    //서버로 부터 연결이 끊어졌을 경우
    void disconnected();

private slots:

    //소켓에러
    void showSocketError(QAbstractSocket::SocketError socketError);
    //send 버튼
    void on_pushButton_clicked();
    //데이터 수신
    void readData();

    //connect 버튼
    void on_pushButton_2_clicked();


    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_file_select_clicked();

private:
    Ui::MainWindow *ui;

    //소켓을 생성할 변수
    QTcpSocket *client_socket;

    //연결확인을 위한 변수
    bool connect_flag=false;

    float data3;

    //mac주소 헤더에 붙이기 위해서 bytearray로
    QByteArray mac_addr;


};
#endif // MAINWINDOW_H
