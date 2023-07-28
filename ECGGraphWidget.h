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
    void dataProcessed(QStringList);

private slots:
    void startReading();
    void updateECGData(QStringList);
    void saveData();

private:
    QVector<QPointF> dataPoints;

    QLineSeries *ecgSeries;
    QChart *ecgChart;
    QChartView *chartView;

    qint64 lastReceivedTimestamp;
    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();
};

#endif // ECGGRAPHWIDGET_H
