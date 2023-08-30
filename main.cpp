#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    ECGGraphWidget *ecgGraphWidget = new ECGGraphWidget();
//    ecgGraphWidget->show();

    MainWindow *mainWindow = new MainWindow();
    mainWindow->show();

    return a.exec();
}
