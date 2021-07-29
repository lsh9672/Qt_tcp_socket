#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qint32 port_number = 9999;

    //서버 소켓 만듦
    while(port_number<10005)
    {
        QTcpServer *server_socket = new QTcpServer(this);

        port_number++;


        //listen 상태에서 클라이언트가 연결할 때마다 newConnection Signal이 발생
        if(!server_socket->listen(QHostAddress::Any,port_number))
        {
            ui->textBrowser->insertPlainText("Server could not Start\n");
        }
        else
        {
            ui->textBrowser->insertPlainText(QString("Qt Version : %1\n").arg(QT_VERSION_STR));
            ui->textBrowser->insertPlainText(QString("listen port : %1\n").arg(port_number));
            ui->textBrowser->insertPlainText("Sever Started!\n");
            ui->textBrowser->insertPlainText("-------------------------------logs-------------------------------\n");
        }
         connect(server_socket,SIGNAL(newConnection()),this,SLOT(newConnection()));
    }

        //연결이 들어오면(newConnection Signal이 발생하면) 해당 시그널을 처리하도록 connect 함수로 시그널과 슬롯을 연결



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
    QTcpServer *server_socket = (QTcpServer*)sender();
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
    /*sender()=> Signal에 의해 활성화된 Slot에서 호출하면 신호를 보낸 객체
    (클라이언트의 소켓정보를 넘겨준다.)*/
   QTcpSocket *client = (QTcpSocket*)sender();

   //소켓을 닫는다.
   client->close();
   //닫힘정보 gui화면에 표시
   ui->textBrowser->insertPlainText("disconnected client\n");
}

void MainWindow::readData()
{
    QByteArray buffer;

    QTcpSocket *client_socket = (QTcpSocket*)sender();

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
                //emit sigReadData(client_socket->socketDescriptor(),data2);

                qDebug() << "data size : " << body.size();


                send_flag=writeData(body);

                if(send_flag)
                {
                    qDebug() << "Data Return Success!";
                    //write 성공여부를 위한 Signal
                    //emit sigWriteSuccess(client_socket->socketDescriptor());
                    //this->wait(100);
                }
                else
                {
                    qDebug() << "Data Return Fail";
                    //실패시 화면출력을 위한 signal
                    //emit sigWriteFail(client_socket->socketDescriptor());
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
                //emit sigFileSave(temp_path+fileName);

                qDebug() << "data size : " << body.size();
                buffer.clear();
            }
        }
        //헤더 분석 불가
        else
        {
            qDebug() <<"header error";
        }

     }
}

//클라이언트 쪽으로 데이터를 보내는 함수, bool 타입으로 쓰기 작업이 완전히 끝났는지 확인을 한다.
bool MainWindow::writeData(QByteArray mydata)
{
    QTcpSocket *client = (QTcpSocket*)sender();
    //받은 데이터에 붙여줄 문자열
    QString returnData = "server_send=>";
    returnData.append(QString(mydata));

    //ByteArray로 변환하여 씀
    client->write(returnData.toStdString().c_str());

    //데이터를 온전히 다 write하면 true 발생
    return client->waitForBytesWritten();
}



void MainWindow::on_pushButton_clicked()
{

}

