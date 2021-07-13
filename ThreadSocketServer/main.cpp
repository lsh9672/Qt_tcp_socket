#include "mainwindow.h"

#include <QApplication>

#include "myserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MyServer server1;
    //server1.startServer();

    MainWindow w;
    w.show();
    return a.exec();
}
