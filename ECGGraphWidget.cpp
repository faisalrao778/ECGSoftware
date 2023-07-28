#include "ECGGraphWidget.h"
#include "DataWorker.h"

ECGGraphWidget::ECGGraphWidget(QWidget *parent) : QWidget(parent)
{
    lastReceivedTimestamp = 0;

    setupGraph();
    setupSerialPort();

    QtConcurrent::run(this, &ECGGraphWidget::startReading);

    connect(this, &ECGGraphWidget::dataProcessed, this, &ECGGraphWidget::updateECGData);
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
    xAxis->setTickCount(10);
    ecgChart->addAxis(xAxis, Qt::AlignBottom);

    QValueAxis *yAxis = new QValueAxis;
    yAxis->setTitleText("Amplitude");
    ecgChart->addAxis(yAxis, Qt::AlignLeft);

    ecgSeries->attachAxis(xAxis);
    ecgSeries->attachAxis(yAxis);
    ecgSeries->setUseOpenGL(true);

    QPen pen = ecgSeries->pen();
    pen.setWidth(1);
    pen.setColor(Qt::red);
    ecgSeries->setPen(pen);

    chartView = new QChartView(ecgChart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QPushButton *saveButton = new QPushButton("Save Data", this);
    saveButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(chartView);
    mainLayout->addWidget(saveButton);

    this->setLayout(mainLayout);
    this->resize(1500,900);

    connect(saveButton, &QPushButton::clicked, this, &ECGGraphWidget::saveData);
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
        qDebug() << "Failed To Open Serial Port!";
        return;
    }

    qDebug() << "Opened Serial Port!";
}

void ECGGraphWidget::startReading()
{
    QStringList dataPoints;
    QByteArray readBuffer;

    while (true)
    {
        QCoreApplication::processEvents();

        readBuffer.append(serialPort.readAll());

        while (readBuffer.size() >= 3)
        {
            if ((readBuffer.at(0)) == '\xAA')
            {
                uchar lsb = static_cast<uchar>(readBuffer.at(1));
                uchar msb = static_cast<uchar>(readBuffer.at(2));

                int ecgValue = (msb << 8) | lsb;
                qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

                dataPoints.append(QString::number(timestamp)+"|"+QString::number(ecgValue));

                if (timestamp - lastReceivedTimestamp >= 50)
                {
                    qDebug() << "Update To Chart";
                    emit dataProcessed(dataPoints);
                    dataPoints.clear();
                    lastReceivedTimestamp = timestamp;
                }
                readBuffer.remove(0, 3);
            }
            else
            {
                readBuffer.remove(0, 1);
            }
        }

        QThread::msleep(10);
    }
}

void ECGGraphWidget::updateECGData(QStringList ecgList)
{
    qDebug() << "updateECGData : " << ecgList.size();

    for (int i = 0 ; i < ecgList.size() ; i++)
    {
        qint64 timestamp = ecgList.at(i).split("|").at(0).toLongLong();
        int ecgValue = ecgList.at(i).split("|").at(1).toInt();
        dataPoints.append(QPointF(timestamp, ecgValue));
    }

    ecgSeries->replace(dataPoints);

    qint64 firstTimestamp = dataPoints.first().x();
    qint64 lastTimestamp = dataPoints.last().x();

    ecgChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);
    ecgChart->axisY()->setRange(0, 65536);
}

void ECGGraphWidget::saveData()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Data", QDir::homePath(), "Data Files (*.dat);;All Files (*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        // Save the data from the ecgData map to the file in the desired format
        for (const QPointF& point : dataPoints)
        {
            qint64 timestamp = point.x();
            int ecgAmplitude = point.y();
            out << timestamp << "," << ecgAmplitude << "\n";
        }

        file.close();
    }
    else
    {
        // Handle file open error if needed
        QMessageBox::critical(this, "Error", "Failed to save data to the file.");
    }
}
