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
    void updateData();

private:
    int dataCount = 0; // Counter to simulate continuous data

    QMap<QDateTime, int> ecgData;

    qreal timeGlobal= 0, ampGlobal = 0;
    qreal startTime;
    QSerialPort serialPort;
    QByteArray readBuffer;

    QLineSeries *ecgSeries;
    QChart *ecgChart;
    QChartView *chartView;

    void setupSerialPort();
    void setupGraph();
    void parseData(QByteArray data);
};

#endif // ECGGRAPHWIDGET_H
