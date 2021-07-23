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

    //데이터 전달 확인용 함수
    bool writeData(QByteArray data);

    //서버로 부터 연결이 끊어졌을 경우
    void disconnected();

private slots:
    //send 버튼
    void on_pushButton_clicked();
    //데이터 수신
    void readData();

    //connect 버튼
    void on_pushButton_2_clicked();


    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;

    //소켓을 생성할 변수
    QTcpSocket *client_socket;

    //연결확인을 위한 변수
    bool connect_flag=false;

    //데이터 전송이 제대로 되었는지 확인용 변수
    bool send_flag=true;


};
#endif // MAINWINDOW_H
