#include "mainwindow.h"
#include "serverworker.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //w.show();
    ServerWorker s;
    return a.exec();
}
