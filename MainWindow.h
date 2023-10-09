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
#include <QDateTime>

namespace Ui {    class MainWindow;     }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    const int MAX_VECTOR_SIZE = 20000;

    QMutex ecgMutex, pressMutex;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void handleError(QSerialPort::SerialPortError error);

signals:
    void emitWriteData(QByteArray data);

private:
    Ui::MainWindow *ui;

    QVector<QPointF> ecgDataPoints, thresholdPoints, pressDataPoints, threshold2Points;

    QFile *ecgLog, *tempLog, *pressLog;
    QTextStream *ecgStream, *tempStream, *pressStream;

    QLineSeries *ecgSeries, *pressSeries, *thresholdSeries, *threshold2Series;
    QScatterSeries *thresholdMarkerSeries, *threshold2MarkerSeries;
    QChart *ecgChart, *pressChart;
    QChartView *chartView, *pressChartView;

    bool isThresholdPassed, isThreshold2Passed;
    quint8 threshold, threshold2;

    QTimer *chartUpdateTimer;

    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();
    void closeEvent(QCloseEvent *event);

private slots:
    void simulation();
    void startReading();
    void updateCharts();
    void writeData(QByteArray data);
    void on_pushButton_data_save_clicked();
    void on_pushButton_threshold_save_clicked();
    void on_pushButton_threshold2_save_clicked();
};

#endif // MAINWINDOW_H
