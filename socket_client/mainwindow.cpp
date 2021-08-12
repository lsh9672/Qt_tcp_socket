#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtEndian>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TCP client");

    QString temp_mac;

    //mac 주소 가져오기
    QNetworkInterface interface;
    QList<QNetworkInterface> macList = interface.allInterfaces();
    for(int i = 0; i < macList.size(); i++)
    {
        QString str = macList.at(i).hardwareAddress();
        //윈도우용
       if(str != "")
       //리눅스 용
       //if(str != "00:00:00:00:00:00")
       {
           //임시로 마지막 맥주소 저장(현재 가상 네트워크때문에 맥주소가 여러개 나옴 - 실제 디바이스에서는 한개만 잡힐것이라고 생각)
          temp_mac = str;

       }
    }
    qDebug()<<"mac address check : " << temp_mac;

    //저장한 맥주소를 hex값만 뽑아서 QByteArray에 넣기
    QStringList temp_list = temp_mac.split(":");
    for(int i = 0; i<temp_list.size(); i++)
    {
        QString temp_num;
        temp_num.sprintf("%02X", temp_list.at(i).toInt(NULL,16));
        mac_addr.append(QByteArray::fromHex(temp_num.toUtf8()));
    }

    qDebug() << "mac address : "<< mac_addr;
    qDebug() << "mac address size : "<< mac_addr.size();


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
    //ui->textBrowser->insertPlainText("sending message......\n");
    //연결이 되어있는 상태
    if(client_socket->state() == QAbstractSocket::ConnectedState)
    {
        //gui에 입력한 클라이언트 메시지
        //QString client_message;
        //QByteArray header,send_message;

        QByteArray client_message,header,send_message;


        //입력한 메시지를 가져옴
        client_message=ui->textEdit->toPlainText().toUtf8();
        //ui->textBrowser_socket_state->setText(QString::number(client_message.toUtf8().size()));

        //file_size 추가(4Byte)
        QString file_size;
        file_size.sprintf("%08X",client_message.size());
        QByteArray temp3= QByteArray::fromHex(file_size.toUtf8());

        for(int i = temp3.size()-1; i>=0; i--)
        {
            header.append(temp3.at(i));
        }


        //evt 추가(메시지의 경우 십진수 1로 함,1Byte)
        QString evt;
        evt.sprintf("%02X",1);
        header.prepend(QByteArray::fromHex(evt.toUtf8()));

        //dev_id 추가 - 맥주소 받아와서 처리(프로그램 실행시 처리) - 6Byte
        header.prepend(mac_addr);

        //sid 추가 - 임의로 0x22값을 넣음(1Byte)
        QByteArray sid = QByteArray::fromHex("22");
        header.prepend(sid);

        //페이로드 크기 - 지금까지 추가한 header크기+메시지 크기 - 4byte
        QString payload_length;
        payload_length.sprintf("%08X",client_message.size()+header.size());
        QByteArray temp2 = QByteArray::fromHex(payload_length.toUtf8());
        QByteArray payload2;
        //little endian으로 변환.
        for(int i = temp2.size()-1; i>=0; i--)
        {
            payload2.append(temp2.at(i));
            qDebug()<<"little check : "<<payload2.toHex();
        }

        header.prepend(payload2);

        //파일 타입 - 메시지이면 ms(0x00, 파일이면 fl - 0xff) - 2byte
        QByteArray type_header = QByteArray::fromHex("0000");
        header.prepend(type_header);

        //보낼 데이터
        client_message.prepend(header);
        send_message = client_message;


        qDebug() << "size2 : "<< send_message.size();
        qDebug()<< "type2 : " <<send_message.mid(0,2).toHex();
        ui->textBrowser->insertPlainText("type : ");
        ui->textBrowser->insertPlainText(send_message.mid(0,2).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "length2 : " <<send_message.mid(2,4).toHex();
        ui->textBrowser->insertPlainText("length(little endian) : ");
        ui->textBrowser->insertPlainText(send_message.mid(2,4).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "sid2 : " <<send_message.mid(6,1).toHex();
        ui->textBrowser->insertPlainText("sid : ");
        ui->textBrowser->insertPlainText(send_message.mid(6,1).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "dev_id2 : " <<send_message.mid(7,6).toHex();
        ui->textBrowser->insertPlainText("dev_id : ");
        ui->textBrowser->insertPlainText(send_message.mid(7,6).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "evt2 : " <<send_message.mid(13,1).toHex();
        ui->textBrowser->insertPlainText("evt : ");
        ui->textBrowser->insertPlainText(send_message.mid(13,1).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "file_size2 : " <<send_message.mid(14,4).toHex();
        ui->textBrowser->insertPlainText("file_size(little endian) : ");
        ui->textBrowser->insertPlainText(send_message.mid(14,4).toHex());
        ui->textBrowser->insertPlainText("\n");

        client_socket->write(send_message);
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

    if(client_socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray header,send_message;
        QString file_path = ui->textBrowser_file_path->toPlainText();
        qDebug() << file_path;



        //파일 이름이 비어있으면 에러문구 출력
        if(file_path.isEmpty())
        {
            QMessageBox::information(this,"QTcpClient","file path is empty!!");
            return;
        }

        ui->textBrowser->insertPlainText("file uploading........\n");
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
            send_message = file.readAll();
            qDebug() << "file size : " << send_message.size();
            //경로명에서 확장자 추출
            qDebug() <<"extension(type2) : " <<file_path.split(".")[1].toLower();

            if(file.size() == send_message.size())
            {
                QByteArray header;


                //file_size 추가(4Byte) - little endian으로 변환
                QString file_size;
                file_size.sprintf("%08X",send_message.size());
                QByteArray temp3 = QByteArray::fromHex(file_size.toUtf8());
                for(int i = temp3.size()-1; i>=0; i--)
                {
                    header.append(temp3.at(i));
                }

                //evt 추가(jpg:2,png:3,gif:4,txt:5,json:6,xml:7,mp4:8,avi:9 // 1Byte)
                QString evt;
                int evt_num=5;
                if(file_path.split(".")[1].toLower() == "jpg") evt_num = 2;
                else if(file_path.split(".")[1].toLower() == "png") evt_num = 3;
                else if(file_path.split(".")[1].toLower() == "gif") evt_num = 4;
                else if(file_path.split(".")[1].toLower() == "txt") evt_num = 5;
                else if(file_path.split(".")[1].toLower() == "json") evt_num = 6;
                else if(file_path.split(".")[1].toLower() == "xml") evt_num = 7;
                else if(file_path.split(".")[1].toLower() == "mp4") evt_num = 8;
                else if(file_path.split(".")[1].toLower() == "avi") evt_num = 9;
                else evt_num = 10;

                evt.sprintf("%02X",evt_num);
                header.prepend(QByteArray::fromHex(evt.toUtf8()));

                //dev_id 추가 - 맥주소 받아와서 처리(프로그램 실행시 처리) - 6Byte
                header.prepend(mac_addr);

                //sid 추가 - 임의로 0x22값을 넣음(1Byte)
                QByteArray sid = QByteArray::fromHex("22");
                header.prepend(sid);

                //페이로드 크기 - 지금까지 추가한 header크기+메시지 크기 - 4byte
                QString payload_length;
                payload_length.sprintf("%08X",send_message.size()+header.size());
                QByteArray temp2 = QByteArray::fromHex(payload_length.toUtf8());
                QByteArray payload2;
                //little endian으로 변환.
                for(int i = temp2.size()-1; i>=0; i--)
                {
                    payload2.append(temp2.at(i));
                }

                header.prepend(payload2);

                //파일 타입 - 메시지이면 ms(0x0000, 파일이면 fl - 0xffff) - 2byte
                QByteArray type_header = QByteArray::fromHex("ffff");
                header.prepend(type_header);
                qDebug() << "size2 : "<< header.size();

                //보낼 데이터에 만든 헤드 붙이기
                send_message.prepend(header);



                qDebug()<< "type2 : " <<send_message.mid(0,2).toHex();
                ui->textBrowser->insertPlainText("type : ");
                ui->textBrowser->insertPlainText(send_message.mid(0,2).toHex());
                ui->textBrowser->insertPlainText("\n");

                qDebug()<< "length2 : " <<send_message.mid(2,4).toHex();
                ui->textBrowser->insertPlainText("length(little endian) : ");
                ui->textBrowser->insertPlainText(send_message.mid(2,4).toHex());
                ui->textBrowser->insertPlainText("\n");

                qDebug()<< "sid2 : " <<send_message.mid(6,1).toHex();
                ui->textBrowser->insertPlainText("sid : ");
                ui->textBrowser->insertPlainText(send_message.mid(6,1).toHex());
                ui->textBrowser->insertPlainText("\n");

                qDebug()<< "dev_id2 : " <<send_message.mid(7,6).toHex();
                ui->textBrowser->insertPlainText("dev_id : ");
                ui->textBrowser->insertPlainText(send_message.mid(7,6).toHex());
                ui->textBrowser->insertPlainText("\n");

                qDebug()<< "evt2 : " <<send_message.mid(13,1).toHex();
                ui->textBrowser->insertPlainText("evt : ");
                ui->textBrowser->insertPlainText(send_message.mid(13,1).toHex());
                ui->textBrowser->insertPlainText("\n");

                qDebug()<< "file_size2 : " <<send_message.mid(14,4).toHex();
                ui->textBrowser->insertPlainText("file_size : ");
                ui->textBrowser->insertPlainText(send_message.mid(14,4).toHex());
                ui->textBrowser->insertPlainText("\n");

                qDebug()<< "header size : " << header.size();

                /*
                //소켓으로 데이터 쓰기
                QDataStream socketStream(client_socket);

                socketStream.setVersion(QDataStream::Qt_5_12);

                socketStream << send_message;
                */

                client_socket->write(send_message);

            }

            else
            {
                QMessageBox::critical(this,"socket client","file read error");
            }

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

        tempData=client_socket->readAll();


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
        connect(client_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(showSocketError(QAbstractSocket::SocketError)));
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
//소켓 에러처리 슬롯
void MainWindow::showSocketError(QAbstractSocket::SocketError socketError)
{
    //ui->textBrowser->insertPlainText(QString("Error! : %1").arg(socketError));
    switch(socketError)
    {
        //document에서 제공하는 오류
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this,"QTcpServer","The connection was refused by the peer (or timed out).Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this,"QTcpServer","The host address was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::SocketResourceError:
            QMessageBox::information(this,"QTcpServer","The local system ran out of resources (e.g., too many sockets).");
        break;
        case QAbstractSocket::NetworkError:
            QMessageBox::information(this,"QTcpServer","An error occurred with the network (e.g., the network cable was accidentally plugged out).");
        break;
        case QAbstractSocket::UnsupportedSocketOperationError:
            QMessageBox::information(this,"QTcpServer","The requested socket operation is not supported by the local operating system (e.g., lack of IPv6 support).");
        break;
        default:
            QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
            QMessageBox::information(this,"QTcpServer",QString("Error : %1").arg(socket->errorString()));

    }
}



//센서 데이터 전송-17bytes
void MainWindow::on_pushButton_sensor_clicked()
{



    //연결이 되어있는 상태
    if(client_socket->state() == QAbstractSocket::ConnectedState)
    {
        //gui에 입력한 센서데이터
        QByteArray header,send_message,sensor_data;
        float temp_sensor_data;



        //dev_id 추가 - 맥주소 받아와서 처리(프로그램 실행시 처리) - 6Byte
        header = mac_addr;

        //sid 추가 - 센서의 경우 0x11값을 넣음(1Byte)
        QByteArray sid = QByteArray::fromHex("11");
        header.prepend(sid);

        //페이로드 크기 - 센서데이터는 크기가 11바이트로 고정(little endian)
        QString payload_length;
        payload_length.sprintf("%08X",11);
        QByteArray temp2 = QByteArray::fromHex(payload_length.toUtf8());
        QByteArray payload2;
        //little endian으로 변환.
        for(int i = temp2.size()-1; i>=0; i--)
        {
            payload2.append(temp2.at(i));
        }

        header.prepend(payload2);

        //파일 타입 - 센서데이터는 ffff - 2byte
        QByteArray type_header = QByteArray::fromHex("ffff");
        header.prepend(type_header);


        /*
        float *test;
        float *test2;

        float temp_sensor_data=ui->textEdit_sensor->toPlainText().toFloat();
        //변환시 little endian으로 변환됨.
        QByteArray sensor_data = QByteArray::fromRawData(reinterpret_cast<char *>(&temp_sensor_data),sizeof(float));

        test = reinterpret_cast<float*>(sensor_data.data());
        test2 = reinterpret_cast<float*>(temp.data());
        float test3 = *test;
        float test4 = *test2;
        qDebug()<< "test1 : " << sensor_data.toHex();
        qDebug()<< "test2 : " << temp.toHex();
        qDebug()<< "test2 : " << test3;
        qDebug()<< "test2 : " << test4;
         */

        //입력한 메시지를 가져옴
        temp_sensor_data=ui->textEdit_sensor->toPlainText().toFloat();

        //16진수로 변환
        sensor_data = QByteArray::fromRawData(reinterpret_cast<char *>(&temp_sensor_data),sizeof(float));
        /*sensor_data를 little endian으로
        QByteArray temp;
        for(int i  = sensor_data.size()-1 ; i >=0; i--)
        {
            temp.append(sensor_data.at(i));
        }
        */

        //big endian 그대로
        sensor_data.prepend(header);


        qDebug() << "size2 : "<< sensor_data.size();
        qDebug()<< "type2 : " <<sensor_data.mid(0,2).toHex();
        ui->textBrowser->insertPlainText("type : ");
        ui->textBrowser->insertPlainText(sensor_data.mid(0,2).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "length2 : " <<sensor_data.mid(2,4).toHex();
        ui->textBrowser->insertPlainText("length(little endian) : ");
        ui->textBrowser->insertPlainText(sensor_data.mid(2,4).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "sid2 : " <<sensor_data.mid(6,1).toHex();
        ui->textBrowser->insertPlainText("sid : ");
        ui->textBrowser->insertPlainText(sensor_data.mid(6,1).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "dev_id2 : " <<sensor_data.mid(7,6).toHex();
        ui->textBrowser->insertPlainText("dev_id : ");
        ui->textBrowser->insertPlainText(sensor_data.mid(7,6).toHex());
        ui->textBrowser->insertPlainText("\n");

        qDebug()<< "sensor_data : " <<sensor_data.mid(13,4).toHex();
        ui->textBrowser->insertPlainText("sensor_data : ");
        ui->textBrowser->insertPlainText(sensor_data.mid(13,4).toHex());
        ui->textBrowser->insertPlainText("\n");

        client_socket->write(sensor_data);
     }

      //연결이 잘 안되었으면
    else if(client_socket->state() == QAbstractSocket::UnconnectedState)
    {
         ui->textBrowser->insertPlainText("connect fail! - please connect server\n");
    }

}

