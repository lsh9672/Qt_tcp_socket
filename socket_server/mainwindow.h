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
    ~MainWindow();

signals:
    void dataReceived(QByteArray);

public slots:
    //서버에 소켓접속시 실행
    void newConnection();

    //소켓 연결해제시 실행
    void disconnected();

    //데이터 수신함수
    void readData();

    //클라이언트로 리턴데이터 써주는 함수
    bool writeData(QByteArray data);


private:
    Ui::MainWindow *ui;
    QTcpServer *server_socket;
    bool send_flag;


};
#endif // MAINWINDOW_H
