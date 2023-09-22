#include "MainWindow.h"
#include "ui_MainWindow.h"

#define CHART_WIDTH 1400
#define CHART_HEIGHT 400

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    this->move(availableGeometry.topLeft());

    startTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    lastReceivedTimestamp = 0;
    threshold = 200;
    isThresholdPassed = false;

    ui->lineEdit_threshold->setText(QString::number(threshold));

    setupGraph();
    setupSerialPort();

    connect(this, &MainWindow::dataProcessed, this, &MainWindow::updateData);
    connect(this, &MainWindow::emitWriteData, this, &MainWindow::writeData);
    connect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(this, &MainWindow::emitRemoveVectorData, this, &MainWindow::RemoveData);

    //dataManagementThread = new DataManagementThread(ecgDataPoints, tempDataPoints, thresholdPoints, pressDataPoints);
    //dataManagementThread->start();
}

void MainWindow::setupGraph()
{
    //SETUP ECG CHART
    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::red);

    QPen pen_threshold;
    pen_threshold.setStyle(Qt::SolidLine);
    pen_threshold.setWidth(2);
    pen_threshold.setColor(Qt::darkRed);

    QPalette palette;
    palette.setColor(QPalette::Window, Qt::white);

    ecgSeries = new QLineSeries();
    tmpSeries = new QLineSeries();
    thresholdSeries = new QLineSeries();
    thresholdMarkers = new QScatterSeries();

    QValueAxis *xAxisEcg = new QValueAxis;
    xAxisEcg->setTickCount(10);
    xAxisEcg->setReverse(true);
    xAxisEcg->setLabelsVisible(false);

    QValueAxis *yAxisEcg = new QValueAxis;
    yAxisEcg->setLabelsVisible(false);

    QValueAxis *xAxisTmp = new QValueAxis;
    xAxisTmp->setTickCount(10);
    xAxisTmp->setReverse(true);
    xAxisTmp->setLabelsVisible(false);

    QValueAxis *yAxisTmp = new QValueAxis;
    yAxisTmp->setLabelsVisible(false);

    ecgChart = new QChart();
    ecgChart->addSeries(ecgSeries);
    ecgChart->addSeries(thresholdSeries);
    ecgChart->addSeries(thresholdMarkers);
    ecgChart->legend()->setVisible(false);
    ecgChart->addAxis(xAxisEcg, Qt::AlignBottom);
    ecgChart->addAxis(yAxisEcg, Qt::AlignLeft);
    ecgChart->setPlotArea(QRect(0, 0, CHART_WIDTH, CHART_HEIGHT));

    ecgSeries->attachAxis(xAxisEcg);
    ecgSeries->attachAxis(yAxisEcg);
    ecgSeries->setUseOpenGL(true);
    ecgSeries->setPointsVisible(true);
    ecgSeries->setPointLabelsVisible(true);
    ecgSeries->setPointLabelsFormat("(@xPoint, @yPoint)");
    ecgSeries->setPen(pen);

    thresholdSeries->attachAxis(xAxisEcg);
    thresholdSeries->attachAxis(yAxisEcg);
    thresholdSeries->setPen(pen_threshold);

    thresholdMarkers->attachAxis(xAxisEcg);
    thresholdMarkers->attachAxis(yAxisEcg);
    thresholdMarkers->setMarkerSize(10);
    thresholdMarkers->setColor(Qt::red);
    thresholdMarkers->setMarkerShape(QScatterSeries::MarkerShapeRectangle);

    chartView = new QChartView(ecgChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setPalette(palette);
    chartView->setFrameShape(QFrame::Box);
    chartView->setFixedSize(QSize(CHART_WIDTH, CHART_HEIGHT));

    //SETUP TEMPRATURE CHART

    tmpChart = new QChart();
    tmpChart->addSeries(tmpSeries);
    tmpChart->legend()->setVisible(false);
    tmpChart->addAxis(xAxisTmp, Qt::AlignBottom);
    tmpChart->addAxis(yAxisTmp, Qt::AlignLeft);
    tmpChart->setPlotArea(QRect(0, 0, CHART_WIDTH, CHART_HEIGHT));

    tmpSeries->attachAxis(xAxisTmp);
    tmpSeries->attachAxis(yAxisTmp);
    tmpSeries->setUseOpenGL(true);
    tmpSeries->setPointsVisible(true);
    tmpSeries->setPointLabelsVisible(true);
    tmpSeries->setPointLabelsFormat("(@xPoint, @yPoint)");
    tmpSeries->setPen(pen);

    tmpChartView = new QChartView(tmpChart);
    tmpChartView->setRenderHint(QPainter::Antialiasing);
    tmpChartView->setPalette(palette);
    tmpChartView->setFrameShape(QFrame::Box);
    tmpChartView->setFixedSize(QSize(CHART_WIDTH, CHART_HEIGHT));

    ui->gridLayout_chart1->addWidget(chartView);
    ui->gridLayout_chart2->addWidget(tmpChartView);

    ecgChart->axisY()->setRange(0, 256);
    tmpChart->axisY()->setRange(0, 256);

    thresholdSeries->append(0,threshold);
    thresholdSeries->append(1000,threshold);
}

void MainWindow::setupSerialPort()
{
    serialPort.setPortName("COM12");//COM12
    serialPort.setBaudRate(QSerialPort::Baud115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);

    if (!serialPort.open(QIODevice::ReadWrite))
    {
        qDebug() << "Failed To Open Serial Port!";
        return;
    }

    qDebug() << "Opened Serial Port!";

    QByteArray data;
    data.append(static_cast<char>(threshold));
    emit emitWriteData(data);

    QtConcurrent::run(this, &MainWindow::startReading);
}

void MainWindow::startReading()
{
    QStringList ecgDataPoints,pressDataPoints,tempDataPoints;
    QByteArray readBuffer;

    while (true)
    {
        QCoreApplication::processEvents();

        readBuffer.append(serialPort.readAll());

        while (readBuffer.size() >= 3)
        {
            if (readBuffer.at(0) == '\xAA' || readBuffer.at(0) == '\xBB' || readBuffer.at(0) == '\xDD')
            {
                uchar lsb = static_cast<uchar>(readBuffer.at(1));
                uchar msb = static_cast<uchar>(readBuffer.at(2));

                int value = (msb << 8) | lsb;
                qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

                double dataValue = (static_cast<double>(value) / 65535.00) * 256.00;
                QString val = QString::number(timestamp - startTimestamp) + "|" + QString::number(dataValue);

                if(readBuffer.at(0) == '\xAA')
                    ecgDataPoints.append(val);
                else if(readBuffer.at(0) == '\xBB')
                    pressDataPoints.append(val);
                else if(readBuffer.at(0) == '\xDD')
                    tempDataPoints.append(val);

                if (timestamp - lastReceivedTimestamp >= 100)
                {
                    lastReceivedTimestamp = timestamp;

                    emit dataProcessed(ecgDataPoints,"ECG");
                    emit dataProcessed(tempDataPoints,"TEMP");
                    emit dataProcessed(pressDataPoints,"PRESS");
                    ecgDataPoints.clear();
                    tempDataPoints.clear();
                    pressDataPoints.clear();
                }

                readBuffer.remove(0, 3);
            }
            else if(readBuffer.at(0) == '\xCC')
            {
                QByteArray data;
                data.append(static_cast<char>(threshold));
                emit emitWriteData(data);
                readBuffer.remove(0, 1);
            }
            else
            {
                readBuffer.remove(0, 1);
            }
        }
    }
}

void MainWindow::updateData(QStringList list,QString type)
{
    if(list.isEmpty())
        return;

    if(type.compare("ECG")==0)
    {
        for (int i = 0 ; i < list.size() ; i++)
        {
            qint64 timestamp = list.at(i).split("|").at(0).toLongLong();
            double ecgValue = list.at(i).split("|").at(1).toDouble();

            if(ecgValue > static_cast<double>(threshold) && !isThresholdPassed)
            {
                thresholdPoints.append(QPointF(timestamp, 16));
                isThresholdPassed = true;
            }
            else if(ecgValue < static_cast<double>(threshold) && isThresholdPassed)
            {
                isThresholdPassed = false;
            }

            ecgDataPoints.append(QPointF(timestamp, ecgValue));
        }

        ecgSeries->replace(ecgDataPoints);
        thresholdMarkers->replace(thresholdPoints);

        qint64 lastTimestamp = ecgDataPoints.last().x();

        ecgChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);
        ecgChart->axisY()->setRange(0, 256);

        thresholdSeries->clear();
        thresholdSeries->append(lastTimestamp - 3000, threshold);
        thresholdSeries->append(lastTimestamp, threshold);
    }
    else if(type.compare("TEMP")==0)
    {
        for (int i = 0 ; i < list.size() ; i++)
        {
            qint64 timestamp = list.at(i).split("|").at(0).toLongLong();
            double tmpValue = list.at(i).split("|").at(1).toDouble();
            tempDataPoints.append(QPointF(timestamp, tmpValue));
        }

        ui->label_temprature->setText(QString::number(tempDataPoints.last().y()));
    }
    else if(type.compare("PRESS")==0)
    {
        for (int i = 0 ; i < list.size() ; i++)
        {
            qint64 timestamp = list.at(i).split("|").at(0).toLongLong();
            double tmpValue = list.at(i).split("|").at(1).toDouble();
            pressDataPoints.append(QPointF(timestamp, tmpValue));
        }

        tmpSeries->replace(pressDataPoints);

        qint64 lastTimestamp = pressDataPoints.last().x();

        tmpChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);
        tmpChart->axisY()->setRange(0, 256);
    }

/*
    qDebug()<<"ecgDataPoints.first.x"<<ecgDataPoints.count();
    qDebug()<<"lastReceivedTimestamp"<<lastReceivedTimestamp;
    qDebug()<<"tempDataPoints.first.x"<<tempDataPoints.count();
    qDebug()<<"lastReceivedTimestamp"<<lastReceivedTimestamp;
    qDebug()<<"thresholdPoints.first.x"<<thresholdPoints.count();
    qDebug()<<"lastReceivedTimestamp"<<lastReceivedTimestamp;
    qDebug()<<"pressDataPoints.first.x"<<pressDataPoints.count();
    qDebug()<<"lastReceivedTimestamp"<<lastReceivedTimestamp;
    */

    emitRemoveVectorData(&ecgDataPoints);
    emitRemoveVectorData(&tempDataPoints);
    emitRemoveVectorData(&thresholdPoints);
    emitRemoveVectorData(&pressDataPoints);

    /*
    while (ecgDataPoints.first().x() < lastReceivedTimestamp - 4000)
        ecgDataPoints.removeFirst();

    while (tempDataPoints.first().x() < lastReceivedTimestamp - 4000)
        tempDataPoints.removeFirst();

    while (thresholdPoints.first().x() < lastReceivedTimestamp - 4000)
        thresholdPoints.removeFirst();

    while (pressDataPoints.first().x() < lastReceivedTimestamp - 4000)
        pressDataPoints.removeFirst();
        */
}
void MainWindow::RemoveData(QVector<QPointF> *vector)
{
    if (vector->size() > MAX_VECTOR_SIZE) {
           // Calculate the number of elements to remove
           int elementsToRemove = vector->size() - MAX_VECTOR_SIZE/2;
           // Remove the oldest elements from the beginning of the vector
           vector->remove(0, elementsToRemove);
       }
}

void MainWindow::writeData(QByteArray data)
{
    if (serialPort.write(data) == -1)
        qDebug() << "Error writing to serial port: " << serialPort.errorString();
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        qDebug() << "Serial port closed or encountered a resource error. Reopening...";
        if (!serialPort.open(QIODevice::ReadWrite))
        {
            qDebug() << "Failed To Open Serial Port!";
            return;
        }

        qDebug() << "Opened Serial Port!";
    }
}

void MainWindow::on_pushButton_data_save_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Data", QDir::homePath(), "Data Files (*.dat);;All Files (*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        // Save the data from the ecgData map to the file in the desired format
        for (const QPointF& point : ecgDataPoints)
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

void MainWindow::on_pushButton_threshold_save_clicked()
{
    if(ui->lineEdit_threshold->text().toUInt() < 0 || ui->lineEdit_threshold->text().toUInt() > 255)
    {
        QMessageBox::critical(this, "Error", "Threshould should be in Range: [0-255]");
        return;
    }

    threshold = ui->lineEdit_threshold->text().toUInt();
    thresholdSeries->clear();
    thresholdSeries->append(0,threshold);
    thresholdSeries->append(100,threshold);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "Closing...";
    dataManagementThread->requestInterruption();
    dataManagementThread->wait(); // Wait for the thread to finish

    delete dataManagementThread;

    disconnect(this, &MainWindow::dataProcessed, this, &MainWindow::updateData);
    disconnect(this, &MainWindow::emitWriteData, this, &MainWindow::writeData);
    disconnect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    serialPort.close();

    delete ui;
    QMainWindow::closeEvent(event);
    QCoreApplication::quit();
}

MainWindow::~MainWindow()
{

}
