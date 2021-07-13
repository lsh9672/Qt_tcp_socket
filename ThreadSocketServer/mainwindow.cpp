#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "myThread.h"
#include "myserver.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->textBrowser->insertPlainText(QString("Qt Version : %1\n").arg(QT_VERSION_STR));
    ui->textBrowser->insertPlainText("Sever Started!\n");
    ui->textBrowser->insertPlainText("-------------------------------logs-------------------------------\n");

    //server_socket = new QTcpServer(this);
    //quint32 port_number = 9999;

    MyServer *ms = new MyServer(this);
    ms->startServer();
    //연결정보 ui출력
    connect(ms,SIGNAL(connectClient(qintptr)),this,SLOT(showConnect(qintptr)));

    //읽은 데이터 ui출력
    connect(thread(),SIGNAL(showReadData(qintptr,QString)),this,SLOT(showReadData(qintptr,QString)));

    //write 성공여부 ui 출력 - 성공
    connect(thread(),SIGNAL(showWriteSuccess(qintptr)),this,SLOT(showWriteSuccess(qintptr)));

    //write 성공여부 ui 출력 - 실패
    connect(thread(),SIGNAL(showWriteFail(qintptr)),this,SLOT(showWriteFail(qintptr)));

    //연결해제 ui 출력
    connect(thread(),SIGNAL(showDisconnected(qintptr)),this,SLOT(showDisconnected(qintptr)));


    /*
    //listen 상태에서 클라이언트가 연결할 때마다 newConnection Signal이 발생
    if(!server_socket->listen(QHostAddress::Any,port_number))
    {
        //ui->textBrowser->append("Server could not Start");
        ui->textBrowser->insertPlainText("Server could not Start\n");

        //qDebug() << "Server could not Start";
    }
    else
    {
        //ui->textBrowser->append("Sever Started!2");
        //ui->textBrowser->append("Sever Started!");
        ui->textBrowser->insertPlainText(QString("Qt Version : %1\n").arg(QT_VERSION_STR));
        ui->textBrowser->insertPlainText(QString("listen port : %1\n").arg(port_number));
        ui->textBrowser->insertPlainText("Sever Started!\n");
        ui->textBrowser->insertPlainText("-------------------------------logs-------------------------------\n");
        qDebug() << "Sever Started!";
        //qDebug("Qt Version Hex : %s", QT_VERSION_STR);
    }

    //연결이 들어오면(newConnection Signal이 발생하면) 해당 시그널을 처리하도록 connect 함수로 시그널과 슬롯을 연결
    //connect(server_socket,SIGNAL(newConnection()),this,SLOT(newConnection(socketDescriptor())));
    */
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showConnect(qintptr socketDescriptor)
{
    qDebug() << "check_signal";
    ui->textBrowser->insertPlainText(QString("%1 client connected\n").arg(socketDescriptor));
}


//연결해제 ui 출력을 위한 slot함수
void MainWindow::showDisconnected(qintptr socketDescriptor)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] disconnected client\n").arg(socketDescriptor));
}

//읽은 데이터 ui 출력
void MainWindow::showReadData(qintptr socketDescriptor,QString rData)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] Client to Server : %2\n").arg(socketDescriptor).arg(QString(rData)));
}

//write 성공여부 ui 출력 - 성공시
void MainWindow::showWriteSuccess(qintptr socketDescriptor)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] return success!\n").arg(socketDescriptor));
}

//write 성공여부 ui 출력 - 실패시
void MainWindow::showWriteFail(qintptr socketDescriptor)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] return Fail!\n").arg(socketDescriptor));
}
