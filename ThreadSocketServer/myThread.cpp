 #include "myThread.h"
#include "mainwindow.h"



MyThread::MyThread(qintptr ID, QObject *parent):
    QThread(parent)
{
    this->socketDescriptor = ID;
}

//스레드의 start()함수를 사용했을때 실행하는 부분
void MyThread::run()
{
    //클라이언트의 요청 처리를 위해 소켓 생성
    client_socket = new QTcpSocket();


    //소켓 기술자를 설정,성공적으로 설정되면 true, 이를 이용해서 소켓이 제대로 생성되었는지 확인.
    if(!client_socket->setSocketDescriptor(this->socketDescriptor))
    {
        //emit을 이용하여 강제로 에러 시그널을 보냄
        emit error(client_socket->error());
        return;
    }
    qDebug() <<"buffer Size"<<client_socket->readBufferSize();

    /* ui에 표시할 값들 */
    // 생성된 소켓의 기술자
    TsocketInfo = client_socket->socketDescriptor();
    //클라이언트의 ip
    TconnectIp = client_socket->peerAddress().toString();
    //클라이언트의 port
    TconnectPort = client_socket->peerPort();
    //소켓 기술자가 셋팅된 시점을 접속시간으로 판단 -단위:sec
    TconnectTime = time(NULL);

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
    //데이터를 스트림으로 보내기 위해서 ByteArray를 사용한다.
    QByteArray data2;
    QByteArray temp;
    qDebug() << "Thread numeber : " << client_socket->socketDescriptor();

    //소켓으로 부터 읽을수 있는 바이트가 1개 이상이면 실행한다.
    if(client_socket->bytesAvailable()>0)
    {
        qDebug()<<"read data size1"<< client_socket->bytesAvailable();
        temp = client_socket->readLine();

        qDebug()<<"check data line"<< temp;

        qDebug()<<"read data size2"<< client_socket->bytesAvailable();
        while(client_socket->bytesAvailable()>0)
        {
            //if(!client_socket->waitForReadyRead(500))
            //{
                //소켓으로 부터 모든 데이터를 읽어서 저장한다.
                data2 = data2 + client_socket->readAll();
                qDebug() << "client to server : " << data2;
            //}

        }


        //RedayRead() 신호가 발생하고 읽을수 있는 새 데이터가 있으면 true반환.
        //오류또는 작업시간지나면 false 반환, 즉
        if(!client_socket->waitForReadyRead(100))
        {
            //gui 출력을 위해 받은 데이터를 전부 읽으면 시그널을 발생시킨다.
            qDebug() << "sig check::";
            emit sigReadData(client_socket->socketDescriptor(),QString(data2));
        //발생된 시그널을 처리할수 있도록 잠시 스레드를 멈춤
        //this->wait(100);

            //writeData함수의 결과가 true 이면 데이터를 전부 write하는데 성공 , false이면 실패 or 아직 전부 못함.
            send_flag=writeData(data2);

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
        }
    }


    //만약을 대비해 소켓을 비워줌(남은데이터 전부 write)
    //client_socket->flush();
}


bool MyThread::writeData(QByteArray mydata)
{
    QString returnData = QString("[%1 socket] return data => ").arg(client_socket->socketDescriptor());
    returnData.append(QString(mydata));
    qDebug()<< "server to client : " << returnData;
    client_socket->write(returnData.toStdString().c_str());

    return client_socket->waitForBytesWritten();
}


//gui로 부터 broadcast버튼이 눌렸을때.
void MyThread::broadcast_data_send(QByteArray bdata)
{
    //bdata.prepend()
    qDebug()<<"size"<< bdata.size();
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
