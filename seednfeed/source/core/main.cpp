#include "ui/mainwindow.h"
#include "core/utilities.h"
#include <QApplication>


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setOrganizationName("Girgis Clan");
    a.setOrganizationDomain("elysianshadows.com");
    a.setApplicationName("Seed ~N~ Feed");
    a.setApplicationVersion(QString("v")+QString(VERSION_STRING));

    MainWindow w;
    w.setWindowTitle("Seed ~N~ Feed v"+QString(VERSION_STRING));
    w.show();

    return a.exec();
}