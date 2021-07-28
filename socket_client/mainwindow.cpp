#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TCP client");

    //소켓 생성
    client_socket = new QTcpSocket(this);


    qDebug() <<"buffer Size"<<client_socket->readBufferSize();


    //서버로부터 리턴되는 데이터를 읽음
    connect(client_socket,SIGNAL(readyRead()),SLOT(readData()));

    connect(client_socket,SIGNAL(disconnected()),SLOT(disconnected()));

}

MainWindow::~MainWindow()
{
    if(client_socket->isOpen())
    {
        client_socket->close();
    }
    delete ui;
}

//연결확인을 위한 함수, 인자로 연결할 서버의 ip를 받음
bool MainWindow::connectCheck(QString server_ip, qint32 server_port)
{
    //서버와 연결을 위한 ip와 포트번호넣어주기
    client_socket->connectToHost(server_ip,server_port);

    QDir dir;
    QString temp_path ="socket_file";
    dir.mkpath(temp_path);
    qDebug() << QApplication::applicationDirPath();
    qDebug() <<  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

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

//메세지 보내는 버튼(send)
void MainWindow::on_pushButton_clicked()
{
    //연결이 되어있는 상태
    if(client_socket->state() == QAbstractSocket::ConnectedState)
    {

        QString client_message;
        QByteArray header,send_message,test1;


        //client가 보낼 메시지를 QString변수에 담음(Qstring은 qt에서 사용하는 String 타입정도로 생각)
        client_message=ui->textEdit->toPlainText();
        ui->textBrowser_socket_state->setText(QString::number(client_message.toUtf8().size()));

        //메시지 일때 헤더만들기
        header.prepend(QString("type1:ms,type2:non,length:%1").arg(client_message.size()).toUtf8());

        //헤더크기 128로 고정
        header.resize(128);

        //보낼 메시지 바이트로 만들기
        send_message = client_message.toUtf8();

        //보낼메시지에 헤더 붙이기
        send_message.prepend(header);


        //소켓으로 데이터 쓰기
        QDataStream socketStream(client_socket);

        socketStream.setVersion(QDataStream::Qt_5_12);

        socketStream << send_message;

     }

      //연결이 잘 안되었으면
    else if(client_socket->state() == QAbstractSocket::UnconnectedState)
    {

         ui->textBrowser->insertPlainText("connect fail! - please connect server\n");
    }

}


//파일 보내는 버튼
void MainWindow::on_pushButton_4_clicked()
{
    QByteArray header,send_message;
    QString file_path = ui->textBrowser_file_path->toPlainText();
    qDebug() << file_path;

    //파일 이름이 비어있으면 에러문구 출력
    if(file_path.isEmpty())
    {
        ui->textBrowser->setText("file path is empty!!\n");
        return;
    }

    //입력받은 파일을 byte array로 가져오기
    QFile file(file_path);

    //파일을 읽을 수 없으면
    if(!file.open(QFile::ReadOnly))
    {
        ui->textBrowser->setText("file path is incorrect");
        QMessageBox::critical(this,"socket client","file is no readable");
    }

    //파일이 제대로 열리면
    else
    {
        //파일경로가 제대로 되었을때만 실행
        ui->textBrowser->setText("");
        send_message = file.readAll();
        qDebug() << "file size : " << send_message.size();
        //경로명에서 확장자 추출
        qDebug() <<"extension(type2) : " <<file_path.split(".")[1].toLower();

        if(file.size() == send_message.size())
        {
            //파일 일때 헤더만들기
            header.prepend(QString("type1:file,type2:%1,length:%2").arg(file_path.split(".")[1].toLower()).arg(send_message.size()).toUtf8());

            //헤더크기 128byte로 고정
            header.resize(128);
            qDebug()<< "header size : " << header.size();

            //보낼메시지에 헤더 붙이기
            send_message.prepend(header);

            //소켓으로 데이터 쓰기
            QDataStream socketStream(client_socket);

            socketStream.setVersion(QDataStream::Qt_5_12);

            socketStream << send_message;

            //client_socket->write(send_message);
        }
        else
        {
            QMessageBox::critical(this,"socket client","file read error");
        }
    }

}


//보낸 데이터에 대해서 리턴
void MainWindow::readData()
{
    //서버로부터 받은 데이터
    QByteArray tempData;


    if(client_socket->bytesAvailable()>0)
    {

          tempData = client_socket->readAll();
          QByteArray a = "a";

          qDebug() << "read data" << tempData[0];

          //qDebug() << tempData[0] << tempData[1] << tempData[2] << tempData[3] << tempData[4] << tempData[5];
          //qDebug()<< "data type : "<< typeid (client_socket->readAll()).name(); => 변수타입 확인.
          //QDataStream in(&tempData,QIODevice::ReadWrite);
          //qDebug() << "QByteArray Test : " << tempData;
          //qDebug() << QString(tempData);

            ui->textBrowser->insertPlainText(QString("data sent by the server  :  %1\n").arg(QString(tempData)));
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
    ui->textBrowser->insertPlainText("disconnect from server\n");
    ui->textBrowser_socket_state->setText("disconnected;");
}




void MainWindow::on_pushButton_file_select_clicked()
{
    //파일 선택창 만들기.
    QString file_path = QFileDialog::getOpenFileName(this,"select file","C:\\Users\\etri","Files(*.*)");


    //file path 필드에 경로 보여주기
    ui->textBrowser_file_path->setText(file_path);
}

