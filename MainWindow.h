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

    QMutex ecgMutex;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void handleError(QSerialPort::SerialPortError error);

signals:
    void emitWriteData(QByteArray data);

private:
    Ui::MainWindow *ui;

    bool stopReading = false;

    QVector<QPointF> ecgDataPoints, thresholdPoints;

    QFile *ecgLog, *tempLog;
    QTextStream *ecgStream, *tempStream;

    QLineSeries *ecgSeries, *thresholdSeries;
    QScatterSeries *thresholdMarkerSeries;
    QChart *ecgChart;
    QChartView *chartView;

    bool isThresholdPassed;
    quint8 threshold;

    QTimer *chartUpdateTimer;

    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort(QString portNum);
    void closeEvent(QCloseEvent *event);

private slots:
    void simulation();
    void startReading();
    void updateCharts();
    void writeData(QByteArray data);
    void on_pushButton_data_save_clicked();
    void on_pushButton_threshold_save_clicked();
    void on_pushButton_save_port_clicked();
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
