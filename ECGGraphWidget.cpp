#include "ECGGraphWidget.h"

ECGGraphWidget::ECGGraphWidget(QWidget *parent) : QWidget(parent)
{
    setupGraph();
    setupSerialPort();

    qint64 unixTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/10;

    // Convert the Unix timestamp to qreal (float)
    startTime = static_cast<qreal>(unixTimestamp);

    // Simulate real ECG data here and add it to the series
    // Replace this with actual ECG data from your device
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ECGGraphWidget::updateData);
    timer->start(10);
}

void ECGGraphWidget::setupGraph()
{
    ecgSeries = new QLineSeries();

    ecgChart = new QChart();
    ecgChart->addSeries(ecgSeries);
    ecgChart->setTitle("ECG Chart");
    ecgChart->legend()->setVisible(false);

    QValueAxis *xAxis = new QValueAxis;
    xAxis->setTitleText("Time");
    ecgChart->addAxis(xAxis, Qt::AlignBottom);
    ecgSeries->attachAxis(xAxis);

    QValueAxis *yAxis = new QValueAxis;
    yAxis->setTitleText("Amplitude");
    ecgChart->addAxis(yAxis, Qt::AlignLeft);
    ecgSeries->attachAxis(yAxis);

    // Set the line width
    QPen pen = ecgSeries->pen();
    pen.setWidth(1); // Set the line width to 2 pixels
    pen.setColor(Qt::red);
    ecgSeries->setPen(pen);

    chartView = new QChartView(ecgChart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);

    this->setLayout(layout);
    this->resize(600,400);
}

void ECGGraphWidget::setupSerialPort()
{
    serialPort.setPortName("COM10");
    serialPort.setBaudRate(QSerialPort::Baud115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Error", "Failed To Open Serial Port!");
        return;
    }

    connect(&serialPort, &QSerialPort::readyRead, this, &ECGGraphWidget::readData);
}

void ECGGraphWidget::readData()
{
    readBuffer.append(serialPort.readAll());

    //qDebug()<<readBuffer;

    while (readBuffer.size() >= 3)
    {

       //qDebug()<<"readBuffer: "<<static_cast<qint8>(readBuffer.at(0));
        if ((readBuffer.at(0)) == '\xAA')
        {
            uchar lsb = static_cast<uchar>(readBuffer.at(1));
            uchar msb = static_cast<uchar>(readBuffer.at(2));

            int ecgValue = (msb << 8) | lsb;

           // qDebug()<<QString::number(ecgValue);
            ecgData.insert(QDateTime::currentDateTime(), ecgValue);
            qint64 unixTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/10;

            // Convert the Unix timestamp to qreal (float)
            qreal floatValueOfTime = static_cast<qreal>(unixTimestamp);
             timeGlobal = floatValueOfTime - startTime; // Assuming time is in seconds
             ampGlobal =ecgValue;
/*
             qDebug()<<"¨Time:"<<timeGlobal;
             qDebug()<<"Àmp:"<<ampGlobal;
             // Append the new data point to the chart series
             ecgSeries->append(timeGlobal, ampGlobal);

             // Increment the data counter
             ++dataCount;

             // Update the chart view
             ecgChart->axisX()->setRange(timeGlobal - 5, timeGlobal); // Show the last 5 seconds of data
             ecgChart->axisY()->setRange(0, 65536); // Adjust the Y-axis range based on your ECG data range
*/
            readBuffer.remove(0, 3);
        }
        else
        {
            readBuffer.remove(0, 1);
        }
    }
}

void ECGGraphWidget::updateData()
{
    // Simulate new data points (replace this with actual data from your device)
    //qreal time = dataCount * 0.03; // Assuming time is in seconds
   // qreal amplitude = qrand() % 200; // Random amplitude between 0 and 200 mV

    qDebug()<<"¨Time:"<<timeGlobal;
    qDebug()<<"Àmp:"<<ampGlobal;
    // Append the new data point to the chart series
    ecgSeries->append(timeGlobal, ampGlobal);

    // Increment the data counter
    ++dataCount;

    // Update the chart view
    ecgChart->axisX()->setRange(timeGlobal - 5, timeGlobal); // Show the last 5 seconds of data
    ecgChart->axisY()->setRange(0, 65536); // Adjust the Y-axis range based on your ECG data range
}
