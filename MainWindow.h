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

namespace Ui {    class MainWindow;     }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Define a maximum size for your data vectors
    const int MAX_VECTOR_SIZE = 25000; // Adjust this according to your needs
    QMutex mutex;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void handleError(QSerialPort::SerialPortError error);

signals:
    void dataProcessed(QStringList,QString);
    void emitWriteData(QByteArray data);
    void emitRemoveVectorData(QVector<QPointF> *);

private:
    Ui::MainWindow *ui;

    QVector<QPointF> ecgDataPoints, tempDataPoints, thresholdPoints, pressDataPoints;

    QLineSeries *ecgSeries, *tmpSeries, *thresholdSeries;
    QScatterSeries *thresholdMarkers;
    QChart *ecgChart, *tmpChart;
    QChartView *chartView, *tmpChartView;
    bool isThresholdPassed;

    qint64 startTimestamp, lastReceivedTimestamp;
    quint8 threshold;

    DataManagementThread *dataManagementThread;

    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();
    void closeEvent(QCloseEvent *event);

private slots:
    void simulation();
    void startReading();
    void updateData(QStringList, QString);
    void on_pushButton_data_save_clicked();
    void on_pushButton_threshold_save_clicked();
    void writeData(QByteArray data);
    void RemoveData(QVector<QPointF> *vector);
};

#endif // MAINWINDOW_H
