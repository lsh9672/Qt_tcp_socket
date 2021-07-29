#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    //클라이언트로 리턴데이터 써주는 함수
    bool writeData(QByteArray data);

    ~MainWindow();

signals:

public slots:
    //서버에 소켓접속시 실행
    void newConnection();

    //소켓 연결해제시 실행
    void disconnected();

    //데이터 수신함수
    void readData();




private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    //QTcpServer *server_socket;
    bool send_flag;
    QTcpServer *server_socket2;
    qint32 port1 = 9999;
    qint32 port2 =8888;

    int a=0;

};
#endif // MAINWINDOW_H
