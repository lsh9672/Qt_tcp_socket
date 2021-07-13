#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//연결확인을 위한 함수, 인자로 연결할 서버의 ip를 받음
bool MainWindow::connectCheck(QString server_ip)
{
    //서버와 연결을 위한 ip와 포트번호넣어주기
    client_socket->connectToHost(server_ip,9999);

    //연결 여부를 알려줌(bool 타입)
    return client_socket->waitForConnected();
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
    return client_socket->waitForBytesWritten();
}


//send 버튼
void MainWindow::on_pushButton_clicked()
{
    QString client_message,temp;

    //client가 보낼 메시지를 QString변수에 담음(Qstring은 qt에서 사용하는 String 타입정도로 생각)
    client_message=ui->textEdit->toPlainText();

    //ui->textEdit_2->setText(QString("client_message %1").arg(temp));
    
    //소켓 생성
    client_socket = new QTcpSocket(this);
    
    //연결 시도 및 연결확인

    connect_flag = connectCheck("127.0.0.1"); 
    if(connect_flag)
    {
        ui->textBrowser->insertPlainText(QString("Connected Server!!\n"));
        //QString.toStdString().c_str => char* 형태로 만듦
        send_flag = writeData(client_message.toStdString().c_str());

        //디버깅용
        temp=ui->textEdit->toPlainText();
        qDebug()<< temp;

        //데이터가 제대로 안보내지면
        if(!send_flag)
        {
            ui->textEdit->setText("send fail");
        }

        //잘 보내졌으면 서버로부터 리턴 받은 데이터 출력
        else
        {
            connect(client_socket,SIGNAL(readyRead()),SLOT(readData()));
            //서버로부터 리턴되는 데이터를 읽음

            if(client_socket->bytesAvailable()>=0)
            {

                QByteArray tempData = client_socket->readAll();
                qDebug() << QString(tempData);

                //printf("check return data %s",tempData.data());
                //ui->textEdit_2->setText("asdfasdfasfdas");
            }


        }
        //client_socket->close();

    }

    //연결이 잘 안되었으면
    else
    {

        ui->textBrowser->insertPlainText("connect fail!\n");
    }
}

void MainWindow::readData()
{
    if(client_socket->bytesAvailable()>=0)
    {
        qDebug()<<client_socket->bytesAvailable();

        //서버로부터 받은 데이터
        QByteArray tempData = client_socket->readAll();
        qDebug() << QString(tempData);
        if(tempData=="" && client_socket->bytesAvailable()<=0)
        {
            ui->textBrowser->insertPlainText("nullpointException\n");
        }
        else
        {
            ui->textBrowser->insertPlainText(QString("Server to Client : %1\n").arg(QString(tempData)));
        }
    }

}

