#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "myThread.h"

//클라이언트 정보 출력을 위한 구조체

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Server");



    ui->textBrowser->insertPlainText(QString("Qt Version : %1\n").arg(QT_VERSION_STR));
    ui->textBrowser->insertPlainText("Sever Started!\n");
    ui->textBrowser->insertPlainText("-------------------------------logs-------------------------------\n");

    //QList<QNetworkAddressEntry> QNetworkInterface::addressEntries();


    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    QString all_ip ="";
    //서버의 listen address상태가 Any라면, 서버의 네트워크 인터페이스에 있는 모든 주소를 뽑아내서 보여준다.

    for (const QHostAddress &address: QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
        {
            //172의 경우 보통 도커에서 사용하는 네트워크이므로 제외한다.
            //192의 경우 가상ip 이므로 표시를 해준다.

            if(address.toString().left(3)!="172" && address.toString().left(3) != "192")
            {
                    all_ip.append(address.toString());
                    all_ip.append("\n");
            }
            else if(address.toString().left(3) == "192")
            {
                all_ip.append(address.toString());
                all_ip.append("(veth)");
                all_ip.append("\n");
            }

        }
    }
    ui->textBrowser_server_ip->setText(QString("%1").arg(all_ip));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showConnect(qintptr socketDescriptor, time_t timer)
{
    test = socketDescriptor;

    ui->textBrowser->insertPlainText(QString("[%1 socket] client connected\n").arg(socketDescriptor));
    QTcpSocket *client_s = new QTcpSocket();

    MyThread *thread = new MyThread(socketDescriptor,this,client_s);
    //infoList.append(SocketInfo {socketDescriptor,client_s,timer,client_s->peerAddress().toString(),client_s->peerPort()});

    /*
    foreach(SocketInfo test, infoList)
    {
        qDebug() << QString("%1 , %2 , %3 , %4").arg(test.socketInfo).arg(test.connectTime).arg(test.connectIp).arg(test.connectPort);
    }
    */

    //connect(thread,SIGNAL(test_Signal(QTcpSocket*)),this,SLOT(test_Slot(QTcpSocket*)));

    //읽은 데이터 ui출력
    connect(thread,SIGNAL(sigReadData(qintptr,QString)),this,SLOT(showReadData(qintptr,QString)));

    //write 성공여부 ui 출력 - 성공
    connect(thread,SIGNAL(sigWriteSuccess(qintptr)),this,SLOT(showWriteSuccess(qintptr)));

    //write 성공여부 ui 출력 - 실패
    connect(thread,SIGNAL(sigWriteFail(qintptr)),this,SLOT(showWriteFail(qintptr)));

    //연결해제 ui 출력
    connect(thread,SIGNAL(sigDisconnected(qintptr)),this,SLOT(showDisconnected(qintptr)));

    //소켓설정중에 에러 발생시 처리
    connect(thread,SIGNAL(error(QTcpSocket::SocketError)),this,SLOT(showSocketError(QTcpSocket::SocketError)));


    connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
    thread->start();

}


//연결해제 ui 출력을 위한 slot함수
void MainWindow::showDisconnected(qintptr socketDescriptor)
{
    ui->textBrowser->insertPlainText(QString("[socket] disconnected client\n"));
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

void MainWindow::showSocketError(QTcpSocket::SocketError socketerror)
{
    ui->textBrowser->insertPlainText(QString("Error! : %1").arg(socketerror));
}

//listen성공
void MainWindow::showListenSuccess(QString slog,quint16 listen_port)
{
    /*
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    QString all_ip ="";
    //서버의 listen address상태가 Any라면, 서버의 네트워크 인터페이스에 있는 모든 주소를 뽑아내서 보여준다.
    if(server_address == QHostAddress::Any)
    {
        for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
                //172의 경우 보통 도커에서 사용하는 네트워크 이므로 제외한다.
                if(address.toString().left(3)!="172")
                {
                    all_ip.append(address.toString());
                    all_ip.append("\n");
                }
        }
        ui->textBrowser_server_ip->setText(QString("%1").arg(all_ip));
    }
    */
    //ui->label_server_ip->setText(server_address);
    ui->textBrowser_server_state->insertPlainText(QString("%1(%2)\n").arg(slog).arg(listen_port));
}
//listen 실패
void MainWindow::showListenFail(QString flog)
{
    ui->textBrowser_server_state->setText(flog);
}

//서버 실행하는 버튼
void MainWindow::on_pushButton_start_clicked()
{
    //포트번호를 입력했을때만 실행하도록 함.
    if(!ui->lineEdit_port->text().isEmpty())
    {

        port = (ui->lineEdit_port->text()).QString::toUInt();
        //서버 실행 - 실행시 입력한 포트번호를 넘겨줌.

        MyServer *mserver = new MyServer(this);

        connect(mserver,SIGNAL(listenSuccess(QString,quint16)),this,SLOT(showListenSuccess(QString,quint16)));
        connect(mserver,SIGNAL(listenFail(QString)),this,SLOT(showListenFail(QString)));

        mserver->startServer(port);
        //연결정보 ui출력
        connect(mserver,SIGNAL(connectClient(qintptr,time_t)),this,SLOT(showConnect(qintptr,time_t)));
    }


}

//broadcasting! 버튼을 눌렀을때 연결된 모든 클라이언트로 데이터를 보냄 - broadcast
void MainWindow::on_pushButton_clicked()
{
    //입력데이터를 ByteArray로 변환하여 보냄
    emit broadcast_data((ui->textEdit_send_client->toPlainText()).toStdString().c_str());

    /*
    //broadcast용 소켓을 생성해서 저장한 socket
    QTcpSocket *test_sock = new QTcpSocket(this);
    test_sock->setSocketDescriptor(test);
    test_sock->write((ui->textEdit_send_client->toPlainText()).toStdString().c_str());
    if(test_sock->waitForBytesWritten())
    {
        test_sock->close();
    }
    */
    //testsocke->write((ui->textEdit_send_client->toPlainText()).toStdString().c_str());
}

void MainWindow::test_Slot(QTcpSocket *p)
{
    testsocke = p;
}
