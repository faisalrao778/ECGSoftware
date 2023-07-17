#ifndef ECGGRAPHWIDGET_H
#define ECGGRAPHWIDGET_H

#include <QWidget>
#include <QtSerialPort>
#include <QVector>
#include <QtWidgets>
#include <QtCharts>

class ECGGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ECGGraphWidget(QWidget *parent = nullptr);

private slots:
    void readData();

private:
    void setupSerialPort();
    void setupGraph();
    //void processByte(quint8 byte);
    void parseData(QByteArray data);

    QSerialPort serialPort;
    QVector<quint8> dataBuffer;
    int dataCounter;
    QLineSeries *series;
    QChart *chart;
    QChartView *chartView;
};

#endif // ECGGRAPHWIDGET_H
