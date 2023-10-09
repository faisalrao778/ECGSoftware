#include "MainWindow.h"
#include "ui_MainWindow.h"

#define CHART_WIDTH 1400
#define CHART_HEIGHT 400

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    this->move(availableGeometry.topLeft());

    threshold = 150;
    isThresholdPassed = false;

    threshold2 = 150;

    ui->lineEdit_threshold->setText(QString::number(threshold));
    ui->lineEdit_threshold2->setText(QString::number(threshold2));

    QString datetime = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

    ecgLog = new QFile("ecgdata_" + datetime + ".txt");
    ecgLog->open(QIODevice::WriteOnly | QIODevice::Text);
    ecgStream = new QTextStream(ecgLog);

    tempLog = new QFile("tempdata_" + datetime + ".txt");
    tempLog->open(QIODevice::WriteOnly | QIODevice::Text);
    tempStream = new QTextStream(tempLog);

    pressLog = new QFile("pressdata_" + datetime + ".txt");
    pressLog->open(QIODevice::WriteOnly | QIODevice::Text);
    pressStream = new QTextStream(pressLog);

    setupGraph();
    setupSerialPort();
//    QtConcurrent::run(this, &MainWindow::simulation);

    connect(this, &MainWindow::emitWriteData, this, &MainWindow::writeData);
    connect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    chartUpdateTimer = new QTimer(this);
    connect(chartUpdateTimer, &QTimer::timeout, this, &MainWindow::updateCharts);
    chartUpdateTimer->start(10);
}

void MainWindow::simulation()
{
    int minValue = 10000;
    int maxValue = 35000;

    while(true)
    {
        int value = qrand() % (maxValue - minValue + 1) + minValue;

        qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        double dataValue = (static_cast<double>(value) / 65535.00) * 256.00;

        ecgDataPoints.append(QPointF(timestamp,dataValue));
        pressDataPoints.append(QPointF(timestamp,dataValue));

        if(dataValue > static_cast<double>(threshold) && !isThresholdPassed)
        {
            thresholdPoints.append(QPointF(timestamp, 16));
            isThresholdPassed = true;
        }
        else if(dataValue < static_cast<double>(threshold) && isThresholdPassed)
        {
            isThresholdPassed = false;
        }

        if(ecgDataPoints.size() > MAX_VECTOR_SIZE)
            ecgDataPoints.removeFirst();

        if(thresholdPoints.size() > MAX_VECTOR_SIZE)
            thresholdPoints.removeFirst();

        if(pressDataPoints.size() > MAX_VECTOR_SIZE)
            pressDataPoints.removeFirst();

        Sleep(1);
    }
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
    pressSeries = new QLineSeries();
    thresholdSeries = new QLineSeries();
    threshold2Series = new QLineSeries();
    thresholdMarkerSeries = new QScatterSeries();

    QValueAxis *xAxisEcg = new QValueAxis;
    xAxisEcg->setTickCount(10);
    xAxisEcg->setReverse(true);
    xAxisEcg->setLabelsVisible(false);

    QValueAxis *yAxisEcg = new QValueAxis;
    yAxisEcg->setLabelsVisible(false);

    QValueAxis *xAxisPress = new QValueAxis;
    xAxisPress->setTickCount(10);
    xAxisPress->setReverse(true);
    xAxisPress->setLabelsVisible(false);

    QValueAxis *yAxisPress = new QValueAxis;
    yAxisPress->setLabelsVisible(false);

    ecgChart = new QChart();
    ecgChart->addSeries(ecgSeries);
    ecgChart->addSeries(thresholdSeries);
    ecgChart->addSeries(thresholdMarkerSeries);
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

    thresholdMarkerSeries->attachAxis(xAxisEcg);
    thresholdMarkerSeries->attachAxis(yAxisEcg);
    thresholdMarkerSeries->setMarkerSize(10);
    thresholdMarkerSeries->setColor(Qt::red);
    thresholdMarkerSeries->setMarkerShape(QScatterSeries::MarkerShapeRectangle);

    chartView = new QChartView(ecgChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setPalette(palette);
    chartView->setFrameShape(QFrame::Box);
    chartView->setFixedSize(QSize(CHART_WIDTH, CHART_HEIGHT));

    //SETUP TEMPRATURE CHART

    pressChart = new QChart();
    pressChart->addSeries(pressSeries);
    pressChart->addSeries(threshold2Series);
    pressChart->legend()->setVisible(false);
    pressChart->addAxis(xAxisPress, Qt::AlignBottom);
    pressChart->addAxis(yAxisPress, Qt::AlignLeft);
    pressChart->setPlotArea(QRect(0, 0, CHART_WIDTH, CHART_HEIGHT));

    pressSeries->attachAxis(xAxisPress);
    pressSeries->attachAxis(yAxisPress);
    pressSeries->setUseOpenGL(true);
    pressSeries->setPointsVisible(true);
    pressSeries->setPointLabelsVisible(true);
    pressSeries->setPointLabelsFormat("(@xPoint, @yPoint)");
    pressSeries->setPen(pen);

    threshold2Series->attachAxis(xAxisPress);
    threshold2Series->attachAxis(yAxisPress);
    threshold2Series->setPen(pen_threshold);

    pressChartView = new QChartView(pressChart);
    pressChartView->setRenderHint(QPainter::Antialiasing);
    pressChartView->setPalette(palette);
    pressChartView->setFrameShape(QFrame::Box);
    pressChartView->setFixedSize(QSize(CHART_WIDTH, CHART_HEIGHT));

    ui->gridLayout_chart1->addWidget(chartView);
    ui->gridLayout_chart2->addWidget(pressChartView);

    ecgChart->axisY()->setRange(0, 256);
    pressChart->axisY()->setRange(0, 256);

    thresholdSeries->append(0,threshold);
    thresholdSeries->append(1000,threshold);

    threshold2Series->append(0,threshold2);
    threshold2Series->append(1000,threshold2);
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

                if(readBuffer.at(0) == '\xAA')
                {
                    ecgMutex.lock();

                    ecgDataPoints.append(QPointF(timestamp,dataValue));
                    *ecgStream << timestamp << "," << dataValue << "\n";

                    if(dataValue > static_cast<double>(threshold) && !isThresholdPassed)
                    {
                        thresholdPoints.append(QPointF(timestamp, 16));
                        isThresholdPassed = true;
                    }
                    else if(dataValue < static_cast<double>(threshold) && isThresholdPassed)
                    {
                        isThresholdPassed = false;
                    }

                    if(ecgDataPoints.size() > MAX_VECTOR_SIZE)
                        ecgDataPoints.removeFirst();

                    if(thresholdPoints.size() > MAX_VECTOR_SIZE)
                        thresholdPoints.removeFirst();

                    ecgMutex.unlock();
                }
                else if(readBuffer.at(0) == '\xBB')
                {
					pressMutex.lock();

                    pressDataPoints.append(QPointF(timestamp,dataValue));
                    *pressStream << timestamp << "," << dataValue << "\n";

                    if(pressDataPoints.size() > MAX_VECTOR_SIZE)
                        pressDataPoints.removeFirst();

                    pressMutex.unlock();
                }
                else if(readBuffer.at(0) == '\xDD')
                {
                    ui->label_temprature->setText(QString::number(dataValue));
                    *tempStream << timestamp << "," << dataValue << "\n";
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

void MainWindow::updateCharts()
{
    if(ecgDataPoints.size()>0)
    {
        qint64 ecgLastTimestamp = ecgDataPoints.last().x();

        ecgMutex.lock();
        ecgSeries->replace(ecgDataPoints);
        thresholdMarkerSeries->replace(thresholdPoints);
        ecgMutex.unlock();

        ecgChart->axisX()->setRange(ecgLastTimestamp - 3000, ecgLastTimestamp);

        thresholdSeries->clear();
        thresholdSeries->append(ecgLastTimestamp - 3000, threshold);
        thresholdSeries->append(ecgLastTimestamp, threshold);
    }

    if(pressDataPoints.size()>0)
    {
        qint64 pressLastTimestamp = pressDataPoints.last().x();

        pressMutex.lock();
        pressSeries->replace(pressDataPoints);
        pressMutex.unlock();

        pressChart->axisX()->setRange(pressLastTimestamp - 3000, pressLastTimestamp);

        threshold2Series->clear();
        threshold2Series->append(pressLastTimestamp - 3000, threshold2);
        threshold2Series->append(pressLastTimestamp, threshold2);
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
    return;

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
}

void MainWindow::on_pushButton_threshold2_save_clicked()
{
    if(ui->lineEdit_threshold2->text().toUInt() < 0 || ui->lineEdit_threshold2->text().toUInt() > 255)
    {
        QMessageBox::critical(this, "Error", "Threshould2 should be in Range: [0-255]");
        return;
    }

    threshold2 = ui->lineEdit_threshold2->text().toUInt();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "Closing...";
    dataManagementThread->requestInterruption();
    dataManagementThread->wait(); // Wait for the thread to finish

    delete dataManagementThread;

    ecgLog->close();
    pressLog->close();
    tempLog->close();
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
