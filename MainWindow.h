#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QtSerialPort>
#include <QVector>
#include <QString>
#include <QtWidgets>
#include <QtCharts>
#include <QtConcurrent>

#include "DataManagementThread.h"
#include "UpdateECGGraphThread.h"

namespace Ui {    class MainWindow;     }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QMutex ecgMutex, pressMutex;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void emitWriteSerailData(QByteArray data);

private:
    Ui::MainWindow *ui;

    QTimer *ecgChartUpdateTimer, *pressChartUpdateTimer;
    int ecgChartUpdateIndex, pressChartUpdateIndex;
    bool updateECGChart = false, updatePRESSChart = false;

    QList<QPointF> ecgDataPoints, tempDataPoints, thresholdPoints, pressDataPoints;
    QList<QPointF> oldEcgDataPoints, oldTempDataPoints, oldThresholdPoints, oldPressDataPoints;

    QLineSeries *ecgSeries, *pressSeries, *thresholdSeries;
    QScatterSeries *thresholdMarkerSeries;
    QChart *ecgChart, *pressChart;
    QChartView *chartView, *pressChartView;
    bool isThresholdPassed;

    qint64 startTimestamp, lastReceivedTimestamp;
    quint8 threshold;

    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();
    void closeEvent(QCloseEvent *event);

private slots:
    void simulateSerialData();
    void handleSerialPortError(QSerialPort::SerialPortError error);
    void readSerialData();
    void writeSerialData(QByteArray data);
    void updateEcgChart();
    void updatePressChart();
    void on_pushButton_threshold_save_clicked();
};

#endif // MAINWINDOW_H
