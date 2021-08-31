#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGridLayout>
#include <QNetworkRequest>

#include <QFileDialog>
#include <QStandardPaths>
#include <QBuffer>
#include <QAbstractButton>
#include <QPushButton>
#include <QString>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // Alterando o titulo do QMainWindow
    setWindowTitle("RTSP e Streaming Video com Qt5 Widgets");

    //QString temp_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/socket_file/";
    //QFile file(temp_path+"check3.mp4");

    if(!file.open(QIODevice::WriteOnly))
    {
       qDebug() << "file open fail";
    }

    // Criando 4 QVideoWidget
    _vw1 = new QVideoWidget;


    // Criando 4 QMediaPlayer

    /*
    videoset = new QVideoEncoderSettings;
    videoset->setResolution(1280,720);
    videoset->setQuality(QMultimedia::VeryHighQuality);
    videoset->setFrameRate(0);
    videoset->setCodec("H.264Video");
    */



    // Criando um QGridLayout de 4x4
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(_vw1,0,0,1,2);
    layout->addWidget(ui->widget_1,0,2,1,1);



    // Cria um QWudget que irá receber o layout com os VideoWidgets

    QWidget *win = new QWidget();
    win->setLayout(layout);
    setCentralWidget(win);

    // QMediaPlayer -> QVideoWidget

    // Links de RTSP e Videos

    //const QUrl url1 = QUrl("rtsp://129.254.90.157:8554/ds-test");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mediaStateChanged(QMediaPlayer::MediaStatus status)
{
    qDebug()<<"State check : " <<status;

    if(status == QMediaPlayer::BufferedMedia)
    {
        qDebug()<< "if test" << status;
        //영상저장을 위한 레코더객체

        qDebug() << "check : "<< test->outputLocation();
        /*
        test = new QMediaRecorder(_player1);

        qDebug() <<"record_format"<<test->containerFormat();
        qDebug() <<"record_error"<<test->error();
        test->setVideoSettings(*videoset);



        test->setContainerFormat("mp4");
        test->setOutputLocation(QUrl::fromLocalFile("C:/Users/etri/Desktop/socket_file/check3.mp4"));


        test->record();
        */

    }
}

void MainWindow::bufferlog(int percentFilled)
{
    qDebug()<< "state buffer : "<<percentFilled;
}


void MainWindow::metaChange(bool available)
{
    qDebug() <<"meta Change check : " <<available;
}

void MainWindow::checkPosition(qint64 position)
{
    qDebug() <<"check position : "<< position;
    ui->time_output->setText(QString::number(position));

}

//시작버튼
void MainWindow::on_start_btn_clicked()
{
    try {
        _player1 = new QMediaPlayer;

        _player1->setVideoOutput(_vw1);

        QString ip_num = ui->input_ip->text();
        QString port_num = ui->input_port->text();
        QString session_val = ui->input_session->text();
        QString rtsp_url = QString("rtsp://%1:%2/%3").arg(ip_num).arg(port_num).arg(session_val);
        qDebug()<<"url check : " << rtsp_url;

        //connect(_player1,&QMediaPlayer::bufferStatusChanged,this,&MainWindow::bufferlog);

        //메타데이터 이용가능확인
        //connect(_player1,&QMediaObject::metaDataAvailableChanged,this,&MainWindow::metaChange);

        //connect(_player1,&QMediaPlayer::mediaStatusChanged,this,&MainWindow::mediaStateChanged);
        //position 확인


        const QUrl url1 = QUrl(rtsp_url);

        const QNetworkRequest requestRtsp1(url1);

        connect(_player1,&QMediaPlayer::positionChanged,this,&MainWindow::checkPosition);
        _player1->setMedia(requestRtsp1);


        _player1->play();
    }  catch (_exception e) {
        qDebug()<< "error";
    }

}

