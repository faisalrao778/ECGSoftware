#include "ECGGraphWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ECGGraphWidget ecgGraphWidget;
    ecgGraphWidget.show();
    return a.exec();
}
