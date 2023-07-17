#include "ECGGraphWidget.h"

//QT_CHARTS_USE_NAMESPACE

ECGGraphWidget::ECGGraphWidget(QWidget *parent)
    : QWidget(parent), dataBuffer(512), dataCounter(0)
{
    setupSerialPort();
    setupGraph();

    connect(&serialPort, &QSerialPort::readyRead, this, &ECGGraphWidget::readData);
}

void ECGGraphWidget::readData()
{
    QByteArray data = serialPort.readAll();
    qInfo() << "data received----1";
    parseData(data);
    //    for (int i = 0; i < data.size(); ++i)
    //    {
    //        parseData(data);
    //        qInfo() << "data received----2";
    //       // processByte(static_cast<quint8>(data.at(i)));
    //    }
}

void ECGGraphWidget::setupSerialPort()
{
    serialPort.setPortName("COM4");
    serialPort.setBaudRate(QSerialPort::Baud9600);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Error", "Failed to open serial port!");
        return;
    }
}

void ECGGraphWidget::setupGraph()
{
    series = new QLineSeries();
    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("ECG Graph");

    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    setLayout(layout);
}

void ECGGraphWidget::parseData(QByteArray data)
{
    qInfo() << "data received----4";
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    quint8 header, msb,lsb;

    stream >> header;
    stream >> msb;
    stream >> lsb;
    qInfo() << "STREAM ==> " << header << msb << lsb;
    if(header){
        qInfo() << "data received----5";
        // Update the graph with the new value
        series->append(msb, msb);

        // Adjust the chart view to show the updated data
        chart->axisX()->setRange(0, msb + 1);
        chart->axisY()->setRange(0, 200);

        // Redraw the chart
        chart->update();

        // Clear the data counter for the next set of values
        dataCounter = 0;

    }
}

//void ECGGraphWidget::processByte(quint8 byte)
//{
//    dataBuffer[dataCounter] = byte;
//    dataCounter++;
//qInfo() << "data received----3";
//    if (dataCounter == 3)
//    {
//        // Extract the header, MSB, and LSB values
//        quint8 header = dataBuffer[0];
//        quint8 msb = dataBuffer[1];
//        quint8 lsb = dataBuffer[2];
//        qInfo() << "data received----4" << header;
//        // Check if the received data is valid
//        if (header == 0xAA)
//        {
//    qInfo() << "data received----5";

//            // Combine MSB and LSB to get the heartbeats per minute value
//            int value = (msb << 8) | lsb;

//            // Update the graph with the new value
//            series->append(dataCounter, value);

//            // Adjust the chart view to show the updated data
//            chart->axisX()->setRange(0, dataCounter + 1);
//            chart->axisY()->setRange(0, 200);

//            // Redraw the chart
//            chart->update();

//            // Clear the data counter for the next set of values
//            dataCounter = 0;
//        }
//        else
//        {
//            // Invalid data, reset the data counter
//            dataCounter = 0;
//        }
//    }
//}
