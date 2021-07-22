#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TCP client");

    //소켓 생성
    client_socket = new QTcpSocket(this);

    //서버로부터 리턴되는 데이터를 읽음
    connect(client_socket,SIGNAL(readyRead()),SLOT(readData()));

    connect(client_socket,SIGNAL(disconnected()),SLOT(disconnected()));

}

MainWindow::~MainWindow()
{

}

//연결확인을 위한 함수, 인자로 연결할 서버의 ip를 받음
bool MainWindow::connectCheck(QString server_ip, qint32 server_port)
{
    //서버와 연결을 위한 ip와 포트번호넣어주기
    client_socket->connectToHost(server_ip,server_port);

    //연결 여부를 알려줌(bool 타입)
    if(client_socket->waitForConnected(1000))
    {
        return true;
    }
    else
    {
        qDebug() << client_socket->error();
        return false;
    }

}

//서버로 보낼 데이터를 쓰고, 여부를 확인. 인자로는 서버로 보낼 데이터
bool MainWindow::writeData(QByteArray data)
{
    //state() 소켓의 상태 리턴, 연결이 되어있으면 데이터를 씀
    if(client_socket->state() == QAbstractSocket::ConnectedState)
    {

        client_socket->write(data);

    }
    //client_socket->flush();
    //소켓에 있는 데이터를 다 쓰면 리턴
    if(client_socket->waitForBytesWritten(3000))
    {
        return true;
    }
    else
    {
        qDebug() << client_socket->error();
        return false;
    }
}


//send 버튼
void MainWindow::on_pushButton_clicked()
{
    QString client_message;

    //client가 보낼 메시지를 QString변수에 담음(Qstring은 qt에서 사용하는 String 타입정도로 생각)
    client_message=ui->textEdit->toPlainText();

    //ui->textEdit_2->setText(QString("client_message %1").arg(temp));
    

    //연결이 되었으면
    if(connect_flag)
    {

        //QString.toStdString().c_str => char* 형태로 만듦
        send_flag = writeData(client_message.toStdString().c_str());


        //데이터가 제대로 안보내지면
        if(!send_flag)
        {
            ui->textBrowser->setText("send fail");
        }

    }

    //연결이 잘 안되었으면
    else
    {

        ui->textBrowser->insertPlainText("connect fail! - please connect server\n");
    }
}

//보낸 데이터에 대해서 리턴
void MainWindow::readData()
{
    if(client_socket->bytesAvailable()>0)
    {
        qDebug()<<client_socket->bytesAvailable();

        //서버로부터 받은 데이터
        //QByteArray tempData = client_socket->readAll();
        qDebug()<< "data type : "<< typeid (client_socket->readAll()).name();
        //QDataStream in(&tempData,QIODevice::ReadWrite);
        //qDebug() << "QByteArray Test : " << tempData;
        //qDebug() << QString(tempData);

        //ui->textBrowser->insertPlainText(QString("data sent by the server  :  %1\n").arg(tempData));

    }

}

//ip,port입력후 서버와 연결시도
void MainWindow::on_pushButton_2_clicked()
{

    //디폴트 ip,port
    QString ip_num;
    qint32 port_num;
    //ip,port 입력이 비어있으면 실행안함.
    if(!ui->lineEdit_ip->text().isEmpty() && !ui->lineEdit_port->text().isEmpty())
    {
        //입력받은 ip와 포트 추출
        ip_num = ui->lineEdit_ip->text();
        port_num = (ui->lineEdit_port->text()).QString::toUInt();


        //연결 시도 및 연결확인
        connect_flag = connectCheck(ip_num,port_num);

        //연결되어있으면 Connected success 메시지 출력
        if(connect_flag)
        {
            ui->textBrowser_socket_state->setText(QString("Connected success!!\n"));
        }
        //연결 안되어있으면 Connected fail 출력
        else
        {
            ui->textBrowser_socket_state->setText(QString("Connected fail;;;\n"));
        }
    }

    qDebug() << "ip : " << client_socket->localAddress() ;
    qDebug() << "input : " << ip_num;
    qDebug() << "port : " << client_socket->localPort() ;
    qDebug() << "Name : " << client_socket->peerName() ;


}

//연결해제 버튼
void MainWindow::on_pushButton_3_clicked()
{
    //연결이 되어있으면 해제하고 상태창에 disconnect success!
    if(client_socket->state() == QAbstractSocket::ConnectedState)
    {
        client_socket->disconnectFromHost();
        ui->textBrowser_socket_state->setText("disconnect success!");
    }
    //연결이 안되어있으면 상태창에 disconnected 출력
    else if(client_socket->state() == QAbstractSocket::UnconnectedState)
    {
        ui->textBrowser_socket_state->setText("disconnected");
    }
    //나머지 상태에 대하여
    else
    {
        ui->textBrowser_socket_state->setText("unknown");
    }
}

//서버로부터 연결이 끊어졌을 경우
void MainWindow::disconnected()
{
    client_socket->deleteLater();
    ui->textBrowser->insertPlainText("disconnect from server");
}

