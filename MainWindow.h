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

signals:
    void dataProcessed(QStringList,QString);

private:
    Ui::MainWindow *ui;

    QVector<QPointF> ecgDataPoints, tempDataPoints;

    QLineSeries *ecgSeries, *tmpSeries, *thresholdSeries;
    QChart *ecgChart, *tmpChart;
    QChartView *chartView, *tmpChartView;

    qint64 lastReceivedTimestamp;
    quint8 threshold;

    QSerialPort serialPort;

    void setupGraph();
    void setupSerialPort();

private slots:
    void startReading();
    void updateData(QStringList, QString);
    void on_pushButton_data_save_clicked();
    void on_pushButton_threshold_save_clicked();
};

#endif // MAINWINDOW_H
