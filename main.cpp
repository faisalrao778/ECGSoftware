#include <QApplication>

#include "ECGGraphWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ECGGraphWidget *ecgGraphWidget = new ECGGraphWidget();
    ecgGraphWidget->show();

    return a.exec();
}
