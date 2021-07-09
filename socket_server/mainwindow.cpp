#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //서버 소켓 만듦
    server_socket = new QTcpServer(this);
    quint32 port_number = 9999;

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
        //qDebug() << "Sever Started!";
        //qDebug("Qt Version Hex : %s", QT_VERSION_STR);
    }

    //연결이 들어오면(newConnection Signal이 발생하면) 해당 시그널을 처리하도록 connect 함수로 시그널과 슬롯을 연결
    connect(server_socket,SIGNAL(newConnection()),this,SLOT(newConnection()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

//클라이언트로 부터 연결이 들어왔을때 처리하는 함수
void MainWindow::newConnection()
{

    //hasPendingConnection() => 서버에 보류중인 연결이 있으면 true, 아니면 false
    //연결이 있으면 true가 되어서 로직을 실행 - 연결시도 여부를 확인
    while(server_socket->hasPendingConnections())
    {
        //nextPendingConnection을 이용해서 접속한 클라이언트 정보를 소켓에 넘겨줌
        QTcpSocket *next_socket = server_socket->nextPendingConnection();

        //readyRead시그널이 발생하면 readData함수 실행
        connect(next_socket,SIGNAL(readyRead()),SLOT(readData()));

        //연결종료 시그널이 발생하면 disconnected 실행
        connect(next_socket,SIGNAL(disconnected()),SLOT(disconnected()));
    }
}

//클라이언트로 부터 연결이 종료되면 처리하는 함수
void MainWindow::disconnected()
{
    //sender()=> Signal에 의해 활성화된 Slot에서 호출하면 신호를 보낸 객체(쉽게 이야기 해서 슬롯안에서 호출이 되면 서버에 접속한 클라이언트의 소켓정보를 넘겨준다.)
   QTcpSocket *client = (QTcpSocket*)sender();

   //소켓을 닫는다.
   client->close();
   ui->textBrowser->insertPlainText("disconnected client\n");
}

void MainWindow::readData()
{
    //데이터를 스트림으로 보내기 위해서 Bytearray를 사용한다.
    QByteArray data2;
    QTcpSocket *client = (QTcpSocket*)sender();

    //소켓으로 부터 읽을수 있는 바이트가 1개 이상이면 실행한다.
    if(client->bytesAvailable()>0)
    {
        //소켓으로 부터 모든 데이터를 읽어서 저장한다.
        data2 = client->readAll();
        ui->textBrowser->insertPlainText(QString("Client to Server : %1\n").arg(QString(data2)));
        //qDebug() << "client to server : " << QString(data2);
    }

    //writeData함수의 결과가 true 이면 데이터를 전부 write하는데 성공 , false이면 데이터를 write하는데 실패함(또는 아직 전부다 write 못함)
    send_flag=writeData(data2);

    if(send_flag)
    {

        ui->textBrowser->insertPlainText("Data Return Success!\n");
        //qDebug() << "Data Return Success!";
    }
    else
    {
        ui->textBrowser->insertPlainText("Data Return Fail\n");
        //qDebug() << "Data Return Fail";
    }
}

//클라이언트 쪽으로 데이터를 보내는 함수, bool 타입으로 쓰기 작업이 완전히 끝났는지 확인을 한다.
bool MainWindow::writeData(QByteArray mydata)
{
    QTcpSocket *client = (QTcpSocket*)sender();
    QString returnData = "server_send=>";
    returnData.append(QString(mydata));
    //qDebug()<< "server to client : " << returnData;
    client->write(returnData.toStdString().c_str());
    /*
    if(client->waitForBytesWritten())
    {
        ui->textBrowser->insertPlainText(QString("Server to Client : %1\n").arg(returnData));
    }
    */
    return client->waitForBytesWritten();
}


