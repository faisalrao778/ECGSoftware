#include "MainWindow.h"
#include "ui_MainWindow.h"

#define CHART_WIDTH 1400
#define CHART_HEIGHT 400
#define CHART_X_MIN 0
#define CHART_X_MAX 250
#define CHART_Y_MIN 0
#define CHART_Y_MAX 250

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<QList<QPointF>>("QList<QPointF>");

    QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    this->move(availableGeometry.topLeft());

    startTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    lastReceivedTimestamp = 0;
    threshold = 200;
    isThresholdPassed = false;

    ui->lineEdit_threshold->setText(QString::number(threshold));

    setupGraph();
    //setupSerialPort();
    QtConcurrent::run(this, &MainWindow::simulation);

    //    dataManagementThread = new DataManagementThread(ecgMutex, pressMutex, ecgDataPoints, pressDataPoints);
    //    dataManagementThread->start();

    //    updateECGGraphThread = new UpdateECGGraphThread(ecgMutex, startTimestamp, ecgDataPoints, ecgSeries, thresholdMarkerSeries);
    //    updateECGGraphThread->start();

    //    connect(this, &MainWindow::dataProcessed, this, &MainWindow::updateData);
    connect(this, &MainWindow::emitWriteData, this, &MainWindow::writeData);
    connect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    //    connect(updateECGGraphThread, &UpdateECGGraphThread::emitCharDataPoint, this, &MainWindow::addChartPoint);

    ecgUpdateListIndex = CHART_X_MAX;

    chartUpdateTimer = new QTimer(this);
    connect(chartUpdateTimer, &QTimer::timeout, this, &MainWindow::updateData);
    chartUpdateTimer->start(1);
}

void MainWindow::simulation()
{
    QList<QPointF> ecgPoints, thresPoints;

    int minValue = 10000;
    int maxValue = 35000;

    int i = CHART_X_MIN;

    while(true)
    {
        int value = qrand() % (maxValue - minValue + 1) + minValue;

        qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        double dataValue = (static_cast<double>(value) / 65535.00) * 256.00;

        ecgPoints.insert(i, QPointF(i, dataValue));
        //        if(dataValue > static_cast<double>(threshold) && !isThresholdPassed)
        //        {
        //            thresPoints.append(QPointF((timestamp - startTimestamp), 16));
        //            isThresholdPassed = true;
        //        }
        //        else if(dataValue < static_cast<double>(threshold) && isThresholdPassed)
        //        {
        //            isThresholdPassed = false;
        //        }

        //if (timestamp - lastReceivedTimestamp >= 2000)
        if (i == CHART_X_MAX)
        {
            //            lastReceivedTimestamp = timestamp;
            i = CHART_X_MIN;

            //            if (timestamp - lastReceivedTimestamp >= 5000)
            //            {
            if(!updateECGData)
            {
                qDebug() << "Updating Chart";
                oldEcgDataPoints = ecgDataPoints;
                ecgDataPoints = ecgPoints;
                //                ecgUpdateListIndex = CHART_X_MAX;
                updateECGData = true;
            }


            //                lastReceivedTimestamp = timestamp;
            //            }

            //emit dataProcessed(ecgPoints,"ECG");
            //            emit dataProcessed(thresPoints,"THRES");

            ecgPoints.clear();
            //            thresPoints.clear();
        }
        else
        {
            i++;
        }

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
    tmpSeries = new QLineSeries();
    thresholdSeries = new QLineSeries();
    thresholdMarkerSeries = new QScatterSeries();

    QValueAxis *xAxisEcg = new QValueAxis;
    xAxisEcg->setTickCount(10);
    xAxisEcg->setReverse(true);
    xAxisEcg->setLabelsVisible(false);
    xAxisEcg->setRange(CHART_X_MIN, CHART_X_MAX);

    QValueAxis *yAxisEcg = new QValueAxis;
    yAxisEcg->setLabelsVisible(false);
    yAxisEcg->setRange(CHART_Y_MIN, CHART_Y_MAX);

    QValueAxis *xAxisTmp = new QValueAxis;
    xAxisTmp->setTickCount(10);
    xAxisTmp->setReverse(true);
    xAxisTmp->setLabelsVisible(false);
    xAxisTmp->setRange(CHART_X_MIN, CHART_X_MAX);

    QValueAxis *yAxisTmp = new QValueAxis;
    yAxisTmp->setLabelsVisible(false);
    yAxisTmp->setRange(CHART_Y_MIN, CHART_Y_MAX);

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
    QList<QPointF> ecgPoints, tempPoints, thresPoints, pressPoints;

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

                qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
                double dataValue = (static_cast<double>((msb << 8) | lsb) / 65535.00) * 256.00;

                if(readBuffer.at(0) == '\xAA')
                {
                    ecgPoints.append(QPointF((timestamp - startTimestamp), dataValue));

                    if(dataValue > static_cast<double>(threshold) && !isThresholdPassed)
                    {
                        thresPoints.append(QPointF((timestamp - startTimestamp), 16));
                        isThresholdPassed = true;
                    }
                    else if(dataValue < static_cast<double>(threshold) && isThresholdPassed)
                    {
                        isThresholdPassed = false;
                    }
                }
                else if(readBuffer.at(0) == '\xBB')
                {
                    pressPoints.append(QPointF((timestamp - startTimestamp), dataValue));
                }
                else if(readBuffer.at(0) == '\xDD')
                {
                    tempPoints.append(QPointF((timestamp - startTimestamp), dataValue));
                }

                if (timestamp - lastReceivedTimestamp >= 100)
                {
                    lastReceivedTimestamp = timestamp;

                    emit dataProcessed(ecgPoints,"ECG");
                    emit dataProcessed(thresPoints,"THRES");
                    emit dataProcessed(tempPoints,"TEMP");
                    emit dataProcessed(pressPoints,"PRESS");

                    ecgPoints.clear();
                    thresPoints.clear();
                    tempPoints.clear();
                    pressPoints.clear();
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

void MainWindow::updateData()
{
    if(ecgDataPoints.isEmpty())
        return;

    if(!updateECGData)
        return;

    qDebug() << "updateData listIndex " << ecgUpdateListIndex;

    if (ecgUpdateListIndex >= CHART_X_MIN)
    {
        if(oldEcgDataPoints.isEmpty())
            ecgSeries->append(ecgDataPoints.at(ecgUpdateListIndex));
        else
        {
//            if(spacerAdded)
//                ecgSeries->append(ecgDataPoints.at(ecgUpdateListIndex));
//            else
                ecgSeries->replace(oldEcgDataPoints.at(ecgUpdateListIndex), ecgDataPoints.at(ecgUpdateListIndex));
        }

        if(!oldEcgDataPoints.isEmpty())
        {
            if(ecgUpdateListIndex==CHART_X_MAX)
            {
                ecgSeries->replace(oldEcgDataPoints.at(ecgUpdateListIndex-1),QPointF(ecgUpdateListIndex-1, 0));
                ecgSeries->replace(oldEcgDataPoints.at(ecgUpdateListIndex-2),QPointF(ecgUpdateListIndex-2, 0));
                ecgSeries->replace(oldEcgDataPoints.at(ecgUpdateListIndex-3),QPointF(ecgUpdateListIndex-3, 0));
                ecgSeries->replace(oldEcgDataPoints.at(ecgUpdateListIndex-4),QPointF(ecgUpdateListIndex-4, 0));
                ecgSeries->replace(oldEcgDataPoints.at(ecgUpdateListIndex-5),QPointF(ecgUpdateListIndex-5, 0));

                oldEcgDataPoints.replace(ecgUpdateListIndex-1, QPointF(ecgUpdateListIndex-1, 0));
                oldEcgDataPoints.replace(ecgUpdateListIndex-2, QPointF(ecgUpdateListIndex-2, 0));
                oldEcgDataPoints.replace(ecgUpdateListIndex-3, QPointF(ecgUpdateListIndex-3, 0));
                oldEcgDataPoints.replace(ecgUpdateListIndex-4, QPointF(ecgUpdateListIndex-4, 0));
                oldEcgDataPoints.replace(ecgUpdateListIndex-5, QPointF(ecgUpdateListIndex-5, 0));
            }
            else
            {
                if(ecgUpdateListIndex-5 >= CHART_X_MIN)
                {
                    ecgSeries->replace(oldEcgDataPoints.at(ecgUpdateListIndex-5),QPointF(ecgUpdateListIndex-5, 0));
                    oldEcgDataPoints.replace(ecgUpdateListIndex-5, QPointF(ecgUpdateListIndex-5, 0));
                }
            }
        }

        ecgUpdateListIndex--;
    }
    else
    {
        ecgUpdateListIndex = CHART_X_MAX;
        updateECGData = false;
    }

    //    if(type.compare("ECG")==0)
    {


        //        QTimer* updateTimer = new QTimer(this);
        //        connect(updateTimer, &QTimer::timeout, this, [=]() {
        //            static int listIndex = CHART_X_MAX;
        //            if (listIndex >= CHART_X_MIN) {
        //                ecgSeries->append(list.at(listIndex));
        //                listIndex--;
        //            } else {
        //                updateTimer->stop(); // Stop the timer when all points are added
        //                listIndex = CHART_X_MIN;
        //                ecgSeries->clear();
        //            }
        //        });

        //        // Set the timer interval (adjust this value for the animation speed)
        //        updateTimer->start(2);

        //        for (const QPointF& point : list)
        //        {

        //        }
        //        ecgSeries->replace(list);
        //ecgChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);

        //        thresholdSeries->clear();
        //        thresholdSeries->append(lastTimestamp - 3000, threshold);
        //        thresholdSeries->append(lastTimestamp, threshold);
    }
    //    else if(type.compare("THRES")==0)
    {
        //        thresholdMarkerSeries->append(list);
    }
    //    else if(type.compare("TEMP")==0)
    {
        //        ui->label_temprature->setText(QString::number(list.last().y()));
    }
    //    else if(type.compare("PRESS")==0)
    {
        //        tmpSeries->append(list);
        //        tmpChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);
    }
}

void MainWindow::addChartPoint(QPointF point)
{
    qDebug() << "addChartPoint :: " << point.x() << " " << point.y();
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
