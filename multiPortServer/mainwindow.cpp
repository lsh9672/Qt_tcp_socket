#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <ctime>
#include <Point.h>
#include <InfluxDB.h>
#include <InfluxDBFactory.h>
#include <cppkafka/cppkafka.h>

using namespace std;
using namespace cppkafka;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TCP Server");

    qDebug() << "ui address"<< ui;

    ui->textBrowser->insertPlainText(QString("Qt Version : %1\n").arg(QT_VERSION_STR));
    ui->textBrowser->insertPlainText("Sever Started!\n");
    ui->textBrowser->insertPlainText("-------------------------------logs----------------------------\n");

    //qRegisterMetaType()에 qintptr타입이 등록되어있지 않아서 에러가 발생한다. 따라서 이를 추가해준다.
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<time_t>("time_t");

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    QString all_ip ="";
    //서버의 listen address상태가 Any라면, 서버의 네트워크 인터페이스에 있는 모든 주소를 뽑아내서 보여준다.

    for (const QHostAddress &address: QNetworkInterface::allAddresses())
    {

        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
        {
            //172의 경우 보통 도커에서 사용하는 네트워크이므로 제외한다.
            //192의 경우 가상ip 이므로 표시를 해준다.
            qDebug() << address.toString();


            if(address.toString().left(3)!="172" && address.toString().left(3) != "192")
            {
                all_ip.append(address.toString());
                all_ip.append("\n");
            }
            else if(address.toString().left(3) == "192" || address.toString().left(3) == "172")
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
    foreach(QTcpSocket * socket, connection_socket)
    {
        socket->close();
        socket->deleteLater();
    }
    foreach(QTcpServer * server, connection_server)
    {
        server->close();
        server->deleteLater();
    }

    delete ui;

}

//서버 실행하는 버튼
void MainWindow::on_pushButton_start_clicked()
{
    //포트번호 범위 입력 둘다 했을 경우 실행
    if(!ui->lineEdit_port->text().isEmpty() && !ui->lineEdit_port_2->text().isEmpty() && (ui->lineEdit_port->text()).QString::toUInt()<=(ui->lineEdit_port_2->text()).QString::toUInt())
    {
        //서버 실행 - 실행시 입력한 포트번호를 넘겨줌.
        int port_start = (ui->lineEdit_port->text()).QString::toUInt();
        int port_end = (ui->lineEdit_port_2->text()).QString::toUInt();

        while(port_start<=port_end)
        {
            //MyServer의 함수를 실행하기 위해 객체 생성
            MyServer *mserver = new MyServer(this);
            connection_server.insert(mserver);
            //listen 실행성공시
            connect(mserver,SIGNAL(listenSuccess(QString,quint16)),this,SLOT(showListenSuccess(QString,quint16)));
            //listen 실패시
            connect(mserver,SIGNAL(listenFail(QString)),this,SLOT(showListenFail(QString)));
            //listen하는 startServer
            mserver->startServer(port_start);

            //클라이언트 연결이 들어오면 처리할 슬롯 연결
            connect(mserver,SIGNAL(newConnection()),this,SLOT(newConnection()));
            port_start++;
        }

    }
    //둘중 하나에만 값이 입력된경우
    else if(!ui->lineEdit_port->text().isEmpty() && ui->lineEdit_port_2->text().isEmpty())
    {
        int port_start = (ui->lineEdit_port->text()).QString::toUInt();

        //MyServer의 함수를 실행하기 위해 객체 생성
        MyServer *mserver = new MyServer(this);
        connection_server.insert(mserver);
        //listen 실행성공시
        connect(mserver,SIGNAL(listenSuccess(QString,quint16)),this,SLOT(showListenSuccess(QString,quint16)));
        //listen 실패시
        connect(mserver,SIGNAL(listenFail(QString)),this,SLOT(showListenFail(QString)));
        //listen하는 startServer
        mserver->startServer(port_start);
        //연결정보 ui출력
        connect(mserver,SIGNAL(newConnection()),this,SLOT(newConnection()));
    }
    else if(ui->lineEdit_port->text().isEmpty() && !ui->lineEdit_port_2->text().isEmpty())
    {
        int port_start = (ui->lineEdit_port_2->text()).QString::toUInt();
        //MyServer의 함수를 실행하기 위해 객체 생성
        MyServer *mserver = new MyServer(this);
        connection_server.insert(mserver);
        //listen 실행성공시
        connect(mserver,SIGNAL(listenSuccess(QString,quint16)),this,SLOT(showListenSuccess(QString,quint16)));
        //listen 실패시
        connect(mserver,SIGNAL(listenFail(QString)),this,SLOT(showListenFail(QString)));
        //listen하는 startServer
        mserver->startServer(port_start);
        //연결정보 ui출력
        connect(mserver,SIGNAL(newConnection()),this,SLOT(newConnection()));
    }
    else
    {
        QMessageBox::information(this,"QTcpServer","please insert port number");
    }
}
//새로운 클라이언트 연결이 들어왔을떄 처리하는 슬롯
void MainWindow::newConnection()
{
    QTcpServer *qserver = reinterpret_cast<QTcpServer*>(sender());

    //서버에 보류중인 연결이 있으면 true를 반환하여 반복문 실행
    while(qserver->hasPendingConnections())
    {
        appendSocketInfo(qserver->nextPendingConnection());
    }
}

//QSet에 소켓정보 추가하는 함수 + 모니터링 정보 표시
void MainWindow::appendSocketInfo(QTcpSocket *client_socket)
{
    //연결요청된 소켓 추가
    qDebug() << client_socket->socketDescriptor();
    connection_socket.insert(client_socket);
    //클라이언트 접속정보 화면에 출력
    ui->textBrowser->insertPlainText(QString("[%1 socket] client connected\n").arg(client_socket->socketDescriptor()));

    //소켓기술자 저장 -> Qtcpsocket 객체로 소켓기술자를 찾을수 있도록 해쉬 이용
    connection_socketDescriptor.insert(client_socket,client_socket->socketDescriptor());

    /* ui에 표시할 값들 - 클라이언트 정보 모니터링 */
    // 생성된 소켓의 기술자
    qintptr TsocketInfo = client_socket->socketDescriptor();
    //클라이언트의 ip
    QString TconnectIp = client_socket->peerAddress().toString();
    //클라이언트의 port
    quint16 TconnectPort = client_socket->peerPort();
    //접속한 서버의 로컬포트(작업중인 포트)
    quint16 work_port = client_socket->localPort();
    //소켓 기술자가 셋팅된 시점을 접속시간으로 판단 -단위:sec
    time_t TconnectTime = time(NULL);


    //연결되었을 때, 표에 표시하는 함수
    showClientInfo(TsocketInfo,TconnectIp,TconnectPort,work_port,TconnectTime);



    //소켓에서 데이터를 읽을수 있을떄.
    connect(client_socket,&QTcpSocket::readyRead,this,&MainWindow::readData);


    //소켓연결이 끊어졌을때 - 모니터링 정보 삭제
    connect(client_socket,&QTcpSocket::disconnected,this,&MainWindow::delClientInfo);
    //소켓에러발생시
    connect(client_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(showSocketError(QAbstractSocket::SocketError)));

}

//소켓에서 데이터를 읽을수 있을때


//연결이 끊어졌을때 삭제 - 작업중
void MainWindow::delClientInfo()
{
    QTcpSocket *TsocketInfo  = reinterpret_cast<QTcpSocket*>(sender());
    qintptr temp_sd = connection_socketDescriptor[TsocketInfo];

    qint32 temp2 = infoList.indexOf(temp_sd);
    ui->table_client_info->removeRow(temp2);

    //다음 입력,삭제를 위해 삭제한 값을 리스트에도 삭제
    infoList.removeAt(temp2);

    //저장한 tcp 리스트에서도 삭제
    connection_socketDescriptor.erase(connection_socketDescriptor.find(TsocketInfo));
    //연결해제 정보 화면에 출력
    ui->textBrowser->insertPlainText(QString("[%1 socket] disconnected client\n").arg(temp_sd));

    TsocketInfo->deleteLater();
    TsocketInfo->close();

}

//소켓으로 부터 읽을 데이터가 있을때
void MainWindow::readData()
{

    QTcpSocket* c_socket = reinterpret_cast<QTcpSocket*>(sender());
    //읽어온 데이터 저장을 위한 버퍼
    QByteArray temp_buffer;

    //데이터스트림을 이용해서 소켓 데이터를 받는다.
    if(c_socket->bytesAvailable()>0)
    {

        temp_buffer = c_socket->readAll();

        qDebug()<< "socket test : "<< temp_buffer.size();
        qDebug()<< "socket check all : "<< buffer.size();
        //헤더인지 아닌지 판단
        //헤더가 아닌 잘려서 온 데이터 일 경우 - 온 데이터의 첫번째 2바이트를 열어봤을떄 형식에 맞지 않는 경우
        if(temp_buffer.mid(0,2).toHex() != "ffff" && temp_buffer.mid(0,2).toHex() != "0000")
        {
            buffer.append(temp_buffer);

            //헤더 형식이 아니고 현재 헤더가 비어있다면 형식이 안맞다고 판단
            if(header.isNull())
            {
                qDebug() <<"header" << header.toHex();
                ui->textBrowser->insertPlainText("Unidentifiable format(type field)!");
                buffer.clear();
                return;
            }
            else
            {
                //length 필드를 big endian으로 변환
                QByteArray temp_con = header.mid(2,4);
                QByteArray endian_convert;
                for(int i = temp_con.size()-1; i>=0; i--)
                {
                    endian_convert.append(temp_con.at(i));
                }


                //파일의 데이터 부분이 file_size필드의 값보다 작으면 아직 안왔다고 판단해서 return
                if(buffer.mid(6).size() < endian_convert.toHex().toInt(NULL,16)) return;
                else if(buffer.mid(6).size() > endian_convert.toHex().toInt(NULL,16))
                {
                    //헤더길이(6)+length필드 길이다음의 데이터 저장.
                    QByteArray temp_buf2 = buffer.mid(6+endian_convert.toHex().toInt(NULL,16));
                    //붙어온 다음 데이터의 첫부분을 열어보고 헤더 포맷이 맞으면 저장, 아니면 버림.
                    if(temp_buf2.mid(0,2).toHex() != "ffff" && temp_buf2.mid(0,2).toHex() != "0000")
                    {
                        next_buffer = temp_buf2;
                        next_header = temp_buf2.mid(0,6);
                    }

                }
            }

        }
        //헤더인 경우(첫 2바이트가 형식에 맞을 경우)
        else
        {
            //첫 2바이트가 형식에 맞는데 이미 헤더가 있을경우 그냥 데이터라고 판단.
            buffer.append(temp_buffer);
            if(header.isNull())
            {
                //6바이트까지 헤더
                header = buffer.mid(0,6);
            }

            qDebug() <<"header check1 : "<<header.size();
            //헤더의 length와 페이로드 비교해서 길이가 안맞으면 안왔다고 판단해서 return
            QByteArray temp_con = header.mid(2,4);
            QByteArray endian_convert;
            for(int i = temp_con.size()-1; i>=0; i--)
            {
                endian_convert.append(temp_con.at(i));
            }
            //페이로드 length가 실제 페이로드보다 크면 데이터가 아직 안옴
            if(buffer.mid(6).size() < endian_convert.toHex().toInt(NULL,16)) return;
            //페이로드 length가 실제 페이로드보다 작으면 다음 데이터가 붙어서 옴(이경우 임시로 가지고 있다가 하나의 데이터 처리가 끝났을떄 다음 처리를 위해 넣어둠)
            else if(buffer.mid(6).size() > endian_convert.toHex().toInt(NULL,16))
            {
                QByteArray temp_buf2 = buffer.mid(6+endian_convert.toHex().toInt(NULL,16));
                //붙어온 다음 데이터의 첫부분을 열어보고 헤더 포맷이 맞으면 저장, 아니면 버림.
                if(temp_buf2.mid(0,2).toHex() != "ffff" && temp_buf2.mid(0,2).toHex() != "0000")
                {
                    next_buffer = temp_buf2;
                    next_header = temp_buf2.mid(0,6);
                }
            }
        }

        qDebug()<< "buffer size check : "<< buffer.size();
        //이부분이 실행되면 보낸 데이터를 buffer에 다 썼다는 이야기
        //헤더열어보기//
        //type(2Byte)
        QByteArray rec_type = header.mid(0,2);
        ui->textBrowser->insertPlainText("type : ");
        ui->textBrowser->insertPlainText(header.mid(0,2).toHex());
        ui->textBrowser->insertPlainText("\n");
        qDebug()<< "type : " <<header.mid(0,2).toHex();

        //length(4Byte)- little endian
        ui->textBrowser->insertPlainText("length(little endian) : ");
        ui->textBrowser->insertPlainText(header.mid(2,4).toHex());
        ui->textBrowser->insertPlainText("\n");
        qDebug()<< "length : " <<header.mid(2,4).toHex();
        QByteArray temp1 = header.mid(2,4);
        QByteArray rec_length;
        for(int i = temp1.size()-1; i>=0; i--)
        {
            rec_length.append(temp1.at(i));
        }

        //sid(1Byte)
        QByteArray rec_sid = buffer.mid(6,1);
        ui->textBrowser->insertPlainText("sid : ");
        ui->textBrowser->insertPlainText(buffer.mid(6,1).toHex());
        ui->textBrowser->insertPlainText("\n");
        qDebug()<< "sid : " <<buffer.mid(6,1).toHex();

        //dev_id(6Byte)
        QByteArray rec_dev_id = buffer.mid(7,6);
        ui->textBrowser->insertPlainText("dev_id : ");
        ui->textBrowser->insertPlainText(buffer.mid(7,6).toHex());
        ui->textBrowser->insertPlainText("\n");
        qDebug()<< "dev_id : " <<buffer.mid(7,6).toHex();


        //메시지 처리.
        if(rec_type.toHex()=="0000")
        {
            //evt(1Byte)
            QByteArray rec_evt = buffer.mid(13,1);
            ui->textBrowser->insertPlainText("evt : ");
            ui->textBrowser->insertPlainText(buffer.mid(13,1).toHex());
            ui->textBrowser->insertPlainText("\n");
            qDebug()<< "evt : " <<buffer.mid(13,1).toHex();

            //file_size(4Byte) - little endian
            QByteArray temp2 = buffer.mid(14,4);
            ui->textBrowser->insertPlainText("file_size : ");
            ui->textBrowser->insertPlainText(buffer.mid(14,4).toHex());
            ui->textBrowser->insertPlainText("\n");
            qDebug()<< "file_size : " <<buffer.mid(14,4).toHex().toInt(NULL,16);
            qDebug()<< "buffer_size : " <<buffer.size();
            QByteArray rec_file_size;
            for(int i = temp2.size()-1; i>=0; i--)
            {
                rec_file_size.append(temp2.at(i));
            }

            //데이터 부분 저장.
            QByteArray body = buffer.mid(18);

            //length 필드와 받아서 저장한 데이터 크기가 같다면 실행
            if(rec_file_size.toHex().toInt(NULL,16) == body.size())
            {

                qDebug() << "data size : " << body.size();
                ui->textBrowser->insertPlainText(QString("[%1 socket] Client to Server : %2\n").arg(c_socket->socketDescriptor()).arg(QString(body)));

                QString returnData = QString("[%1 socket] message return success").arg(c_socket->socketDescriptor());
                //qDebug()<< "server to client : " << returnData;

                //클라이언트로 데이터 리턴
                c_socket->write(returnData.toUtf8());

                if(c_socket->waitForBytesWritten())
                {
                    qDebug() << "Data Return Success!";
                    ui->textBrowser->insertPlainText(QString("[%1 socket] return success!\n").arg(c_socket->socketDescriptor()));

                }
                else
                {
                    qDebug() << "Data Return Fail";

                }
                buffer.clear();
            }
            //데이터가 중간에 유실되거나 함-> 다시 보내야됨.
            else if(rec_file_size.toHex().toInt(NULL,16)>0)
            {
                qDebug() << "data send fail";

            }

        }
        // 파일인 경우
        else if(rec_type.toHex()=="ffff")
        {
            //sid가 11이면 센서데이터, 22이면 파일저장
            if(rec_sid.toHex() == "22")
            {
                //evt(1Byte)
                QByteArray rec_evt = buffer.mid(13,1);
                ui->textBrowser->insertPlainText("evt : ");
                ui->textBrowser->insertPlainText(buffer.mid(13,1).toHex());
                ui->textBrowser->insertPlainText("\n");
                qDebug()<< "evt : " <<buffer.mid(13,1).toHex();

                //file_size(4Byte) - little endian
                QByteArray temp2 = buffer.mid(14,4);
                ui->textBrowser->insertPlainText("file_size : ");
                ui->textBrowser->insertPlainText(buffer.mid(14,4).toHex());
                ui->textBrowser->insertPlainText("\n");
                qDebug()<< "file_size : " <<buffer.mid(14,4).toHex().toInt(NULL,16);
                qDebug()<< "buffer_size : " <<buffer.size();
                QByteArray rec_file_size;
                for(int i = temp2.size()-1; i>=0; i--)
                {
                    rec_file_size.append(temp2.at(i));
                }

                //데이터 부분 저장.
                QByteArray body = buffer.mid(18);

                //데이터를 원래크기만큼 받아서 실제로 처리하는 부분
                if(rec_file_size.toHex().toInt(NULL,16) == body.size())
                {
                    qDebug() << "logic check2 : "<< rec_file_size.toHex().toInt(NULL,16);
                    //헤더열어보기
                    //파일 저장할 디렉토리 생성 - 기존에 있으면 무시, 없으면 생성(현재경로에 생성)
                    QDir dir;
                    //사용자의 데스크탑 디렉토리 반환(데스크탑개념이 없는 시스템에서 homelocation과 동일) + 생성할 폴더
                    QString temp_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/socket_file/";

                    //소켓기술자 별로 폴더를 구분하기 위해서
                    temp_path= temp_path.append(QString::number(c_socket->socketDescriptor())+"/");
                    dir.mkpath(temp_path);
                    qDebug() <<temp_path;

                    //파일확장자 가져오기
                    QString file_extension = extension_name.at(rec_evt.toHex().toInt(NULL,16));

                    qDebug()<<"extension name : "<<extension_name.at(rec_evt.toHex().toInt(NULL,16));

                    //파일명으로 쓸 현재시간 (초)가져옴
                    QString fileName = QString::number(time(NULL));

                    //확장자를 붙여서 완전한 파일이름 만들기
                    fileName = fileName.append("."+file_extension);
                    qDebug() << "file path cehck" << fileName;
                    qDebug() << "file path cehck2" << temp_path+fileName;

                    //위에서 만든 경로로 파일열기
                    QFile file(temp_path+fileName);

                    //파일을 열수 없는 경우
                    if(!file.open(QIODevice::WriteOnly))
                    {
                        return;
                    }
                    file.write(body);
                    ui->textBrowser->insertPlainText(QString("success file : %1\n").arg(temp_path+fileName));

                    qDebug() << "data size : " << body.size();

                    c_socket->write(QString("file upload successful!!").toUtf8());
                }
            }
            //11이면 센서 데이터
            else if(rec_sid.toHex() == "11")
            {
                QByteArray body = buffer.mid(13);
                /*sensor_data를 little endian으로
                QByteArray body;
                for(int i  = buffer.mid(13).size()-1 ; i >=0; i--)
                {
                    body.append(buffer.mid(13).at(i));
                }
                */
                //sensor data 출력
                float *value_temp;
                value_temp = reinterpret_cast<float*>(body.data());
                float sen_data = *value_temp;

                QString mac_add = rec_dev_id.toHex(':');

                //influxdb
                auto influxdb = influxdb::InfluxDBFactory::Get("http://129.254.87.109:8086?db=testDB");
                influxdb->write(influxdb::Point{"sensor"}.addField("value",sen_data).addTag("Mac-Address",mac_add.toStdString()));

                //kafka - producer
                try {
                    Configuration config = {{"bootstrap.servers","127.0.0.1:9092"}};

                    Producer producer(config);

                    string message = to_string(sen_data);
                    producer.produce(MessageBuilder("test_topic").partition(1).payload(message));

                    producer.flush();
                }  catch (exception e) {
                    qDebug() <<"error";
                }



                ui->textBrowser->insertPlainText("sensor data : ");
                ui->textBrowser->insertPlainText(QString::number(sen_data));
                ui->textBrowser->insertPlainText("\n");
                qDebug()<< "file_size : " <<sen_data;

            }

            //두개 중에 아무것도 아니면 에러- 버퍼 비우고 리턴
            else
            {
                buffer.clear();
                header.clear();
                ui->textBrowser->insertPlainText("Unidentifiable format(sid)!");
                return;
            }

        }
        //헤더 분석 불가
        else
        {
            qDebug() <<"header error";
            ui->textBrowser->insertPlainText(QString("[%1 socket] return Fail!\n").arg(c_socket->socketDescriptor()));
        }
        //하나의 데이터 요청에 대하여 처리가 끝나면 버퍼를 비워줌
        qDebug() << "finish clear check";
        buffer.clear();
        header.clear();

        //다음 데이터가 붙어있으면
        if(!next_buffer.isNull() && !next_header.isNull())
        {
            buffer = next_buffer;
            header = next_header;
            next_buffer.clear();
            next_header.clear();
        }

    }


}


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
        case QAbstractSocket::RemoteHostClosedError:
        break;
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

//listen 성공여부 ui 출력 - 성공시
void MainWindow::showListenSuccess(QString slog,quint16 listen_port)
{
    ui->textBrowser_server_state->insertPlainText(QString("%1(%2)\n").arg(slog).arg(listen_port));
}
//listen 성공여부 ui 출력 - 실패시
void MainWindow::showListenFail(QString flog)
{
    ui->textBrowser_server_state->setText(flog);
}



//broadcasting! 버튼을 눌렀을때 연결된 모든 클라이언트로 데이터를 보냄 - broadcast
void MainWindow::on_pushButton_clicked()
{
    //입력데이터를 ByteArray로 변환하여 보냄
    QByteArray broadcast_data = ui->textEdit_send_client->toPlainText().toUtf8();

    foreach(QTcpSocket* client_socket, connection_socket)
    {
        if(client_socket->isOpen())
        {
            client_socket->write(broadcast_data);

        }
    }

}

//스레드로 부터 발생한 신호를 처리하는 부분 - 클라이언트 정보를 표시하기 위한 값들을 받아서 테이블에 추가함
void MainWindow::showClientInfo(qintptr TsocketInfo,QString TconnectIp,quint16 TconnectPort,quint16 work_port,time_t TconnectTime)
{
    //struct tm t;
    //localtime_s(&t,&TconnectTime);
    struct tm *t;
    t = localtime(&TconnectTime);
    QString convert_time = QString("%1.%2.%3.%4:%5").arg(t->tm_year + 1900).arg(t->tm_mon+1).arg(t->tm_mday).arg(t->tm_hour).arg(t->tm_min);

    QStringList temp = TconnectIp.split(":");

    //값을 넣기위해 행을 하나 넣는다(빈 네모칸이 생김)
    ui->table_client_info->insertRow(ui->table_client_info->rowCount());
    //setItem(행,열,값) -> 위에서 만든 빈 행에 값을 넣는다. 행-1을 해주는 이유는 행을 하나 넣어서 카운트 값이 1이므로 0행에 넣기 위해 -1을 한다.
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,0,new QTableWidgetItem(QString::number(TsocketInfo)));
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,1,new QTableWidgetItem(temp[temp.length()-1]));
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,2,new QTableWidgetItem(QString::number(TconnectPort)));
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,3,new QTableWidgetItem(QString::number(work_port)));
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,4,new QTableWidgetItem(convert_time));

    //삭제시 빠른 조회를 위해 값 저장.
    //List를 이용하면 table의 row인덱스와 list의 인덱스를 동일하게 유지할수 있음(중간에 삭제가 일어나더라도 인덱스가 당겨짐)
    infoList.append(TsocketInfo);
}

