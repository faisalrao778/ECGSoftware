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

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void handleError(QSerialPort::SerialPortError error);
signals:
    void dataProcessed(QStringList,QString);
    void emitWriteData(QByteArray data);

private:
    Ui::MainWindow *ui;

    QVector<QPointF> ecgDataPoints, tempDataPoints, thresholdPoints;

    QLineSeries *ecgSeries, *tmpSeries, *thresholdSeries;
    QScatterSeries *thresholdMarkers;
    QChart *ecgChart, *tmpChart;
    QChartView *chartView, *tmpChartView;

    qint64 startTimestamp, lastReceivedTimestamp;
    quint8 threshold;

    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();
    void closeEvent(QCloseEvent *event);

private slots:
    void startReading();
    void updateData(QStringList, QString);
    void on_pushButton_data_save_clicked();
    void on_pushButton_threshold_save_clicked();
    void writeData(QByteArray data);
};

#endif // MAINWINDOW_H
