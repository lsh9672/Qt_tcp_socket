 #include "myThread.h"
#include "mainwindow.h"



MyThread::MyThread(qintptr ID, QObject *parent):
    QThread(parent)
{
    this->socketDescriptor = ID;
    //this->client_socket = sock;
}

//스레드의 start()함수를 사용했을때 실행하는 부분
void MyThread::run()
{

    //thread로 실행하는 구문
    qDebug() << "T Start!!";

    qDebug() << "Thread ID" << QThread::currentThreadId();

    //클라이언트의 요청 처리를 위해 소켓 생성
    client_socket = new QTcpSocket();


    //setSocketdescriptor는 소켓의 설명자가 성공적으로 설정되면 true, 이를 이용해서 소켓이 제대로 생성되었는지 확인.
    if(!client_socket->setSocketDescriptor(this->socketDescriptor))
    {
        //emit을 이용하여 강제로 에러 시그널을 보냄
        emit error(client_socket->error());
        return;
    }

    /* ui에 표시할 값들 */
    // 생성된 소켓의 기술자
    TsocketInfo = client_socket->socketDescriptor();
    //클라이언트의 ip
    TconnectIp = client_socket->peerAddress().toString();
    //클라이언트의 port
    TconnectPort = client_socket->peerPort();
    //소켓 기술자가 셋팅된 시점을 접속시간으로 판단 - 온전한게 연결이 되었을때를 접속시간으로 생각함 - sec로 보내서 ui쪽에서 locatime()와 tm구조체로 보기좋게 정제.
    TconnectTime = time(NULL);

    //소켓기술자 셋팅이 되면 ui에 표시할 정보들을 넘길 시그널 발생
    emit sigClientInfo(TsocketInfo,TconnectIp,TconnectPort,TconnectTime);
    qDebug() << "ip : " << client_socket->peerAddress() ;
    qDebug() << "port : " << client_socket->peerPort() ;

    //connect를 이용하여 처리할 시그널과 소켓을 연결
    connect(client_socket,SIGNAL(readyRead()),this,SLOT(readData()));


    connect(client_socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

    qDebug() << client_socket->socketDescriptor() <<  "Client connected";

    //peerxxx를 이용하면 클라이언트 소켓에서 원격으로 연결된 host의 정보를 빼낼수 있다
    //name의 경우에는 connectToHost로 name을 설정해서 접속했다면 알수 있다.
    /*

    */


    exec();

}

void MyThread::disconnected()
{


   //deleteLater() => 이벤트 루프가 실행되고 있을때 끊기더라도 이 함수에 의해서 이벤트 루프 실행후 객체가 삭제됨(중간에 블락될 걱정을 안해도 된다.)
    emit sigDisconnected(TsocketInfo);
    this->wait(100);
    emit  sigClientDel(TsocketInfo);
    client_socket->deleteLater();

    qDebug() << "disconnected client";

   //연결이 끊어졌음을 gui에 표시하기 위한 시그널 발생.


   exit(0);
}

void MyThread::readData()
{
    //데이터를 스트림으로 보내기 위해서 Bytearray를 사용한다.
    QByteArray data2;
    //client_socket = (QTcpSocket*)sender();

    qDebug() << "Thread numeber : " << client_socket->socketDescriptor();

    //소켓으로 부터 읽을수 있는 바이트가 1개 이상이면 실행한다.
    if(client_socket->bytesAvailable()>0)
    {

        //소켓으로 부터 모든 데이터를 읽어서 저장한다.
        data2 = client_socket->readAll();
        //ui->textBrowser->insertPlainText(QString("Client to Server : %1\n").arg(QString(data2)));
        qDebug() << "client to server : " << QString(data2);
        //gui 출력을 위해 받은 데이터를 전부 읽으면 시그널을 발생시킨다.
        emit sigReadData(client_socket->socketDescriptor(),QString(data2));
        //발생된 시그널을 처리할수 있도록 잠시 스레드를 멈춤
        this->wait(100);
    }

    //writeData함수의 결과가 true 이면 데이터를 전부 write하는데 성공 , false이면 데이터를 write하는데 실패함(또는 아직 전부다 write 못함)
    send_flag=writeData(data2);

    if(send_flag)
    {

        //ui->textBrowser->insertPlainText("Data Return Success!\n");
        qDebug() << "Data Return Success!";
        //write 성공여부를 위한 Signal
        emit sigWriteSuccess(client_socket->socketDescriptor());
        this->wait(100);
    }
    else
    {
        //ui->textBrowser->insertPlainText("Data Return Fail\n");
        qDebug() << "Data Return Fail";
        emit sigWriteFail(client_socket->socketDescriptor());
        this->wait(100);
    }
    client_socket->flush();
}
bool MyThread::writeData(QByteArray mydata)
{
    //QTcpSocket *client = (QTcpSocket*)sender();
    QString returnData = QString("[%1 socket] return data => ").arg(client_socket->socketDescriptor());
    returnData.append(QString(mydata));
    qDebug()<< "server to client : " << returnData;
    client_socket->write(returnData.toStdString().c_str());
    /*
    if(client->waitForBytesWritten())
    {
        ui->textBrowser->insertPlainText(QString("Server to Client : %1\n").arg(returnData));
    }
    */
    return client_socket->waitForBytesWritten();
}


//gui로 부터 broadcast버튼이 눌렸을때.
void MyThread::broadcast_data_send(QByteArray bdata)
{
    client_socket->write(bdata);
    if(!client_socket->waitForBytesWritten())
    {
        this->wait();
    }
    else
    {
        qDebug() << "data success";
    }

}
