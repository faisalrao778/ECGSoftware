#ifndef ECGGRAPHWIDGET_H
#define ECGGRAPHWIDGET_H

#include <QWidget>
#include <QtSerialPort>
#include <QVector>
#include <QtWidgets>
#include <QtCharts>
#include <QtConcurrent>

class DataWorker;

class ECGGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ECGGraphWidget(QWidget *parent = nullptr);

signals:
    void dataProcessed(QStringList,QString);

private slots:
    void startReading();
    void updateData(QStringList, QString);
    void saveData();

private:
    QVector<QPointF> ecgDataPoints, tempDataPoints;

    QLineSeries *ecgSeries, *tmpSeries;
    QChart *ecgChart, *tmpChart;
    QChartView *chartView, *tmpChartView;

    qint64 startTimestamp, lastReceivedTimestamp;
    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();
};

#endif // ECGGRAPHWIDGET_H
