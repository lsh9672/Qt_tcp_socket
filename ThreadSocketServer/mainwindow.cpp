#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "myThread.h"

//클라이언트 정보 출력을 위한 구조체

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
    delete ui;
}

void MainWindow::showConnect(qintptr socketDescriptor)
{

    ui->textBrowser->insertPlainText(QString("[%1 socket] client connected\n").arg(socketDescriptor));

    MyThread *thread = new MyThread(socketDescriptor,this);

    //qRegisterMetaType()에 qintptr타입이 등록되어있지 않아서 에러가 발생한다. 따라서 이를 추가해준다.
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<time_t>("time_t");


    //클라이언트 정보 출력
    connect(thread,SIGNAL(sigClientInfo(qintptr,QString,quint16,time_t)),this,SLOT(showClientInfo(qintptr,QString,quint16,time_t)));

    //클라이언트 정보 삭제
    connect(thread,SIGNAL(sigClientDel(qintptr)),this,SLOT(delClientInfo(qintptr)));

    //읽은 데이터 ui출력
    connect(thread,SIGNAL(sigReadData(qintptr,QString)),this,SLOT(showReadData(qintptr,QString)));

    //write 성공여부 ui 출력 - 성공
    connect(thread,SIGNAL(sigWriteSuccess(qintptr)),this,SLOT(showWriteSuccess(qintptr)));

    //write 성공여부 ui 출력 - 실패
    connect(thread,SIGNAL(sigWriteFail(qintptr)),this,SLOT(showWriteFail(qintptr)));

    //연결해제 ui 출력
    connect(thread,SIGNAL(sigDisconnected(qintptr)),this,SLOT(showDisconnected(qintptr)));

    //소켓설정중에 에러 발생시 처리
    connect(thread,SIGNAL(error(QTcpSocket::SocketError)),this,SLOT(showSocketError(QTcpSocket::SocketError)));

    //서버->클라이언트 메시지
    connect(this,SIGNAL(broadcast_data(QByteArray)),thread,SLOT(broadcast_data_send(QByteArray)));

    //이미지 다운로드 성공
    connect(thread,SIGNAL(sigFileSave(QString)),this,SLOT(showFileSave(QString)));

    //스레드에서 작업이 끝나면 발생
    connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
    thread->start();

}


//연결해제 ui 출력을 위한 slot함수
void MainWindow::showDisconnected(qintptr socketDescriptor)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] disconnected client\n").arg(socketDescriptor));
}

//읽은 데이터 ui 출력
void MainWindow::showReadData(qintptr socketDescriptor,QString rData)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] Client to Server : %2\n").arg(socketDescriptor).arg(QString(rData)));
}

//write 성공여부 ui 출력 - 성공시
void MainWindow::showWriteSuccess(qintptr socketDescriptor)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] return success!\n").arg(socketDescriptor));
}

//write 성공여부 ui 출력 - 실패시
void MainWindow::showWriteFail(qintptr socketDescriptor)
{
    ui->textBrowser->insertPlainText(QString("[%1 socket] return Fail!\n").arg(socketDescriptor));
}

void MainWindow::showSocketError(QTcpSocket::SocketError socketerror)
{
    ui->textBrowser->insertPlainText(QString("Error! : %1").arg(socketerror));
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

//서버 실행하는 버튼
void MainWindow::on_pushButton_start_clicked()
{
    //포트번호를 입력했을때만 실행하도록 함.
    if(!ui->lineEdit_port->text().isEmpty())
    {
        //서버 실행 - 실행시 입력한 포트번호를 넘겨줌.
        port = (ui->lineEdit_port->text()).QString::toUInt();

        //MyServer의 함수를 실행하기 위해 객체 생성
        MyServer *mserver = new MyServer(this);
        //listen 실행성공시
        connect(mserver,SIGNAL(listenSuccess(QString,quint16)),this,SLOT(showListenSuccess(QString,quint16)));
        //listen 실패시
        connect(mserver,SIGNAL(listenFail(QString)),this,SLOT(showListenFail(QString)));
        //listen하는 startServer
        mserver->startServer(port);
        //연결정보 ui출력
        connect(mserver,SIGNAL(connectClient(qintptr)),this,SLOT(showConnect(qintptr)));
    }
}
//broadcasting! 버튼을 눌렀을때 연결된 모든 클라이언트로 데이터를 보냄 - broadcast
void MainWindow::on_pushButton_clicked()
{
    //입력데이터를 ByteArray로 변환하여 보냄
    emit broadcast_data((ui->textEdit_send_client->toPlainText()).toStdString().c_str());

}

//스레드로 부터 발생한 신호를 처리하는 부분 - 클라이언트 정보를 표시하기 위한 값들을 받아서 테이블에 추가함
void MainWindow::showClientInfo(qintptr TsocketInfo,QString TconnectIp,quint16 TconnectPort,time_t TconnectTime)
{
    struct tm t;
    localtime_s(&t,&TconnectTime);
    QString convert_time = QString("%1.%2.%3.%4:%5").arg(t.tm_year + 1900).arg(t.tm_mon+1).arg(t.tm_mday).arg(t.tm_hour).arg(t.tm_min);

    QStringList temp = TconnectIp.split(":");

    //값을 넣기위해 행을 하나 넣는다(빈 네모칸이 생김)
    ui->table_client_info->insertRow(ui->table_client_info->rowCount());
    //setItem(행,열,값) -> 위에서 만든 빈 행에 값을 넣는다. 행-1을 해주는 이유는 행을 하나 넣어서 카운트 값이 1이므로 0행에 넣기 위해 -1을 한다.
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,0,new QTableWidgetItem(QString::number(TsocketInfo)));
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,1,new QTableWidgetItem(temp[temp.length()-1]));
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,2,new QTableWidgetItem(QString::number(TconnectPort)));
    ui->table_client_info->setItem(ui->table_client_info->rowCount()-1,3,new QTableWidgetItem(convert_time));

    //삭제시 빠른 조회를 위해 값 저장.
    //List를 이용하면 table의 row인덱스와 list의 인덱스를 동일하게 유지할수 있음(중간에 삭제가 일어나더라도 인덱스가 당겨짐)
    infoList.append(TsocketInfo);
}


//연결이 끊어졌을때 삭제 - 작업중
void MainWindow::delClientInfo(qintptr TsocketInfo)
{
    qint32 temp2 = infoList.indexOf(TsocketInfo);
    ui->table_client_info->removeRow(temp2);

    //다음 입력,삭제를 위해 삭제한 값을 리스트에도 삭제
    infoList.removeAt(temp2);
}

void MainWindow::showFileSave(QString filePath)
{
    ui->textBrowser->insertPlainText(QString("success file : %1\n").arg(filePath));
}
