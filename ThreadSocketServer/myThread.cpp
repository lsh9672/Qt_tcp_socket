 #include "myThread.h"
#include "mainwindow.h"



MyThread::MyThread(qintptr ID, QObject *parent):
    QThread(parent)
{
    this->socketDescriptor = ID;
}
MyThread::~MyThread()
{
    if(client_socket->isOpen())
    {
        client_socket->close();
        client_socket->deleteLater();
    }
}

//스레드의 start()함수를 사용했을때 실행하는 부분
void MyThread::run()
{

    //클라이언트의 요청 처리를 위해 소켓 생성
    client_socket = new QTcpSocket();
    //client_socket->setReadBufferSize(30);


    //소켓 기술자를 설정,성공적으로 설정되면 true, 이를 이용해서 소켓이 제대로 생성되었는지 확인.
    if(!client_socket->setSocketDescriptor(this->socketDescriptor))
    {
        //emit을 이용하여 강제로 에러 시그널을 보냄
        emit error(client_socket->error());
        return;
    }
    qDebug() << QString("%1 socket port %2").arg(client_socket->socketDescriptor()).arg(client_socket->localPort());
    qDebug() <<"buffer Size"<<client_socket->readBufferSize();




    //소켓기술자 셋팅이 되면 ui에 표시할 정보들을 넘길 시그널 발생
    emit sigClientInfo(TsocketInfo,TconnectIp,TconnectPort,TconnectTime);

    //connect를 이용하여 처리할 시그널과 소켓을 연결

    connect(client_socket,SIGNAL(readyRead()),this,SLOT(readData()));

    connect(client_socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

    qDebug() << client_socket->socketDescriptor() <<  "Client connected";

    exec();
}

void MyThread::disconnected()
{

   /*deleteLater() => 이벤트 루프가 실행되고 있을때 끊기더라도
    이 함수에 의해서 이벤트 루프 실행후 객체가 삭제됨(중간에 블락될 걱정을 안해도 된다.)*/

    //연결이 끊어졌음을 gui에 표시하기 위한 시그널 발생.
    emit sigDisconnected(TsocketInfo);
    this->wait(100);
    emit  sigClientDel(TsocketInfo);
    client_socket->deleteLater();

    qDebug() << "disconnected client";

   exit(0);
}

//readyRead 시그널 처리
void MyThread::readData()
{
    //qDebug() << "read signal check :" << check;

    //데이터를 스트림으로 보내기 위해서 ByteArray를 사용한다.
    //QByteArray data2,temp_save_buffer;
    //QString header;
    QByteArray buffer;

    //데이터스트림을 이용해서 소켓 데이터를 받는다.
    if(client_socket->bytesAvailable()>0)
    {
        //QTcpSocket *test = reinterpret_cast<QTcpSocket*>(sender());

        QDataStream socketStream(client_socket);
        socketStream.setVersion(QDataStream::Qt_5_12);

        //a++;
        //qDebug()<<"signal call count" <<a;

        socketStream.startTransaction();

        socketStream >> buffer;


        if(!socketStream.commitTransaction())
        {
            qDebug()<<"commit check : "<< a;
            return;
        }

        qDebug() << "recieve data size check : " << buffer.size();
        //이부분이 실행되면 보낸 데이터를 buffer에 다 썼다는 이야기
        //헤더열어보기
        QString header = buffer.mid(0,128);

        //데이터부분 저장
        QByteArray body = buffer.mid(128);

        //헤더에서 메시지인지 파일인지 확인을 위해
        QString type1 = header.split(",")[0].split(":")[1];

        qDebug() << "header check : " << QString(header);

        //데이터 사이즈 체크를 위해
        int check_length=0;

        if(type1=="ms")
        {
            //데이터 사이즈 저장
            check_length = header.split(",")[2].split(":")[1].toInt();

            //length 필드와 받아서 저장한 데이터 크기가 같다면 실행
            if(check_length == body.size())
            {
                QString data2 = QString(body);
                emit sigReadData(client_socket->socketDescriptor(),data2);

                qDebug() << "data size : " << body.size();


                send_flag=writeData(body);

                if(send_flag)
                {
                    qDebug() << "Data Return Success!";
                    //write 성공여부를 위한 Signal
                    emit sigWriteSuccess(client_socket->socketDescriptor());
                    //this->wait(100);
                }
                else
                {
                    qDebug() << "Data Return Fail";
                    //실패시 화면출력을 위한 signal
                    emit sigWriteFail(client_socket->socketDescriptor());
                    //this->wait(100);
                }
                buffer.clear();
            }
            //데이터가 중간에 유실되거나 함-> 다시 보내야됨.
            else if(check_length>0)
            {
                qDebug() << "data send fail";

            }

        }

        // 파일인 경우
        else if(type1=="file")
        {

            //데이터 사이즈 저장
            check_length = header.split(",")[2].split(":")[1].toInt();

            //데이터를 원래크기만큼 받아서 실제로 처리하는 부분
            if(check_length == body.size())
            {
                //파일 저장할 디렉토리 생성 - 기존에 있으면 무시, 없으면 생성(현재경로에 생성)
                QDir dir;
                //사용자의 데스크탑 디렉토리 반환(데스크탑개념이 없는 시스템에서 homelocation과 동일) + 생성할 폴더
                QString temp_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/socket_file/";

                //소켓기술자 별로 폴더를 구분하기 위해서
                temp_path= temp_path.append(QString::number(client_socket->socketDescriptor())+"/");
                dir.mkpath(temp_path);
                qDebug() <<temp_path;

                //파일확장자 가져오기
                QString file_extension = header.split(",")[1].split(":")[1];

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
                //gui 출력을 위해서
                emit sigFileSave(temp_path+fileName);

                qDebug() << "data size : " << body.size();
                buffer.clear();
            }
        }
        //헤더 분석 불가
        else
        {
            qDebug() <<"header error";
        }


    /* 원래코드
    //헤더가 붙어있는 데이터인지 아닌지 확인
    if(header.split(",")[0].split(":")[1] =="ms" || header.split(",")[0].split(":")[1] =="file")
    {
        //헤더에서 메시지인지 파일인지 확인을 위해
        type1 = header.split(",")[0].split(":")[1];

        //헤더를 빼고 다시 버퍼에 넣어둠
        save_buffer = buffer.mid(128);
    }
    else
    {
        temp_save_buffer = buffer;
    }


    //헤더가 메시지인 경우
    if(type1=="ms")
    {
        //데이터 사이즈 저장
        check_length = header.split(",")[2].split(":")[1].toInt();

        //체크 크기에서 받은 크기를 뺐을때 0이 되면 데이터가 전부온것
        check_length = check_length - save_buffer.size();

        //length 필드와 받아서 저장한 데이터 크기가 같다면 실행
        if(check_length == 0)
        {
            QString data2 = QString(save_buffer);
            emit sigReadData(client_socket->socketDescriptor(),data2);

            qDebug() << "data size : " << save_buffer.size();


            send_flag=writeData(save_buffer);

            if(send_flag)
            {
                qDebug() << "Data Return Success!";
                //write 성공여부를 위한 Signal
                emit sigWriteSuccess(client_socket->socketDescriptor());
                //this->wait(100);
            }
            else
            {
                qDebug() << "Data Return Fail";
                //실패시 화면출력을 위한 signal
                emit sigWriteFail(client_socket->socketDescriptor());
                //this->wait(100);
            }
            save_buffer.clear();
            temp_save_buffer.clear();
            buffer.clear();
        }
        //length 필드와 받아서 저장한 데이터 크기가 다르면 아직 안온것
        else if(check_length>0)
        {
            save_buffer = save_buffer + temp_save_buffer;

        }


    }
    // 파일인 경우
    else if(type1=="file")
    {

        //데이터 사이즈 저장
        check_length = header.split(",")[2].split(":")[1].toInt();

        //체크 크기에서 받은 크기를 뺐을때 0이 되면 데이터가 전부온것
        check_length = check_length - save_buffer.size();

        //데이터를 원래크기만큼 받아서 실제로 처리하는 부분
        if(check_length == 0)
        {

            //파일 저장할 디렉토리 생성 - 기존에 있으면 무시, 없으면 생성(현재경로에 생성)
            QDir dir;
            //사용자의 데스크탑 디렉토리 반환(데스크탑개념이 없는 시스템에서 homelocation과 동일) + 생성할 폴더
            QString temp_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/socket_file/";

            //소켓기술자 별로 폴더를 구분하기 위해서
            temp_path= temp_path.append(QString::number(client_socket->socketDescriptor())+"/");
            dir.mkpath(temp_path);
            qDebug() <<temp_path;

            //파일확장자 가져오기
            QString file_extension = header.split(",")[1].split(":")[1];

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
            file.write(save_buffer);
            //gui 출력을 위해서
            emit sigFileSave(temp_path+fileName);



            qDebug() << "data size : " << save_buffer.size();

            save_buffer.clear();
            temp_save_buffer.clear();
            buffer.clear();
        }

        //length 필드와 받아서 저장한 데이터 크기가 다르면 아직 안온것
        else if(check_length>0)
        {
            save_buffer = save_buffer + temp_save_buffer;

        }



    }

    //헤더가 아니라 데이터 부분이 나눠져서 왔을 경우
    else
    {
        qDebug() << "data chunk test1 : "<< send_data.size();
        //받은 데이터를 버퍼에 이어붙이기
        send_data = send_data+buffer;
        qDebug() << "data chunk test2 : "<< send_data.size();
    }
    */

    }

}


//받은데이터를 클라이언트로 다시 쓰는 부분
bool MyThread::writeData(QByteArray mydata)
{
    //
    QString returnData = QString("[%1 socket] return data => ").arg(client_socket->socketDescriptor());
    returnData.append(QString(mydata));
    //qDebug()<< "server to client : " << returnData;
    client_socket->write(returnData.toStdString().c_str());

    return client_socket->waitForBytesWritten();
}


//gui로 부터 broadcast버튼이 눌렸을때.
void MyThread::broadcast_data_send(QByteArray bdata)
{
    client_socket->write(bdata);
    if(!client_socket->waitForBytesWritten())
    {
        this->wait(100);
    }
    else
    {
        qDebug() << "data success";
    }


}
