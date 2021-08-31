#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaRecorder>
#include <QVideoWidget>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void mediaStateChanged(QMediaPlayer::MediaStatus status);
    void bufferlog(int percentFilled);

    //위치확인
    //void checkLocation(const QUrl);
    //사용가능 여부 확인
    //void checkAval(QMultimedia::AvailabilityStatus);

    //meta 데이터 변경여부
    void metaChange(bool);

    //position 확인
    void checkPosition(qint64);


private slots:
    void on_start_btn_clicked();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *_player1;
    QMediaRecorder *test;
    QVideoEncoderSettings *videoset;
    QFile file;

    QVideoWidget *_vw1;
};



#endif // MAINWINDOW_H
