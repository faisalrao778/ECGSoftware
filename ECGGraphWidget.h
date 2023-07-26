#ifndef ECGGRAPHWIDGET_H
#define ECGGRAPHWIDGET_H

#include <QWidget>
#include <QtSerialPort>
#include <QVector>
#include <QtWidgets>
#include <QtCharts>

class DataWorker;

class ECGGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ECGGraphWidget(QWidget *parent = nullptr);

private slots:
    void readData();
    void updateData(qint64 timestamp, int ecgAmplitude);
    void saveData();

private:
    QByteArray readBuffer;
    QMap<qint64, int> ecgData;

    QLineSeries *ecgSeries;
    QChart *ecgChart;
    QChartView *chartView;

    qint64 lastReceivedTimestamp;
    QSerialPort serialPort;

    QThread *dataThread;
    DataWorker *dataWorker;

    void setupGraph();
    void setupSerialPort();

};

#endif // ECGGRAPHWIDGET_H
