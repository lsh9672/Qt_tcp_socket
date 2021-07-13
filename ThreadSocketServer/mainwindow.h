#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTcpServer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void startServer();

signals:

public slots:

    //클라이언트 연결시 ui표시
    void showConnect(qintptr socketDescriptor);

    //소켓 연결해제시에 UI 표시를 위한 slot
    void showDisconnected(qintptr socketDescriptor);

    //읽어온 데이터를 UI에 표시하기 위한 slot
    void showReadData(qintptr socketDescriptor,QString rData);

    //write 성공여부
    void showWriteSuccess(qintptr socketDescriptor);

    //실패시
    void showWriteFail(qintptr socketDescriptor);


protected:
    void incomingConnection(qintptr socketDescriptor);


private:
    Ui::MainWindow *ui;
    QTcpServer *server_socket;
    bool send_flag;

};
#endif // MAINWINDOW_H
