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

public slots:
    void handleError(QSerialPort::SerialPortError error);
    void addChartPoint(QPointF);

signals:
    void dataProcessed(QList<QPointF>, QString);
    void emitWriteData(QByteArray data);
    void emitRemoveVectorData(QVector<QPointF> *);

private:
    Ui::MainWindow *ui;

    QTimer* chartUpdateTimer;
    int ecgUpdateListIndex;

    bool updateECGData = false;
    bool spacerAdded = false;

    QList<QPointF> ecgDataPoints, tempDataPoints, thresholdPoints, pressDataPoints;
    QList<QPointF> oldEcgDataPoints, oldTempDataPoints, oldThresholdPoints, oldPressDataPoints;

    QLineSeries *ecgSeries, *tmpSeries, *thresholdSeries;
    QScatterSeries *thresholdMarkerSeries;
    QChart *ecgChart, *tmpChart;
    QChartView *chartView, *tmpChartView;
    bool isThresholdPassed;

    qint64 startTimestamp, lastReceivedTimestamp;
    quint8 threshold;

    DataManagementThread *dataManagementThread;
    UpdateECGGraphThread *updateECGGraphThread;

    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();
    void closeEvent(QCloseEvent *event);

private slots:
    void simulation();
    void startReading();
    void updateData();
    void on_pushButton_data_save_clicked();
    void on_pushButton_threshold_save_clicked();
    void writeData(QByteArray data);
};

#endif // MAINWINDOW_H
