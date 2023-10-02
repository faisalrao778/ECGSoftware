#include "MainWindow.h"
#include "ui_MainWindow.h"

#define CHART_WIDTH 1400
#define CHART_HEIGHT 400
#define CHART_X_MIN 0
#define CHART_X_MAX 250
#define CHART_Y_MIN 0
#define CHART_Y_MAX 256

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
//    QtConcurrent::run(this, &MainWindow::simulateSerialData);

    connect(this, &MainWindow::emitWriteSerailData, this, &MainWindow::writeSerialData);
    connect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleSerialPortError);

    ecgChartUpdateIndex = CHART_X_MAX;
    pressChartUpdateIndex = CHART_X_MAX;

    ecgChartUpdateTimer = new QTimer(this);
    connect(ecgChartUpdateTimer, &QTimer::timeout, this, &MainWindow::updateEcgChart);
    ecgChartUpdateTimer->start(1);

    pressChartUpdateTimer = new QTimer(this);
    connect(pressChartUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePressChart);
    pressChartUpdateTimer->start(1);
}

void MainWindow::simulateSerialData()
{
    QList<QPointF> ecgPoints, thresPoints;

    int minValue = 10000;
    int maxValue = 35000;

    int i = CHART_X_MIN;

    while(true)
    {
        int value = qrand() % (maxValue - minValue + 1) + minValue;

        double dataValue = (static_cast<double>(value) / 65535.00) * 256.00;

        ecgPoints.insert(i, QPointF(i, dataValue));

        if(dataValue > static_cast<double>(threshold) && !isThresholdPassed)
        {
            thresPoints.insert(i, QPointF(i, 16));
            isThresholdPassed = true;
        }
        else if(dataValue < static_cast<double>(threshold) && isThresholdPassed)
        {
            thresPoints.insert(i, QPointF(i, 0));
            isThresholdPassed = false;
        }
        else
        {
            thresPoints.insert(i, QPointF(i, 0));
        }

        if (i == CHART_X_MAX)
        {
            i = CHART_X_MIN;

            if(!updateECGChart)
            {
                oldEcgDataPoints = ecgDataPoints;
                ecgDataPoints = ecgPoints;

                oldThresholdPoints = thresholdPoints;
                thresholdPoints = thresPoints;

                updateECGChart = true;
            }

            ecgPoints.clear();
            thresPoints.clear();
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
    pressSeries = new QLineSeries();
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

    QValueAxis *xAxisPress = new QValueAxis;
    xAxisPress->setTickCount(10);
    xAxisPress->setReverse(true);
    xAxisPress->setLabelsVisible(false);
    xAxisPress->setRange(CHART_X_MIN, CHART_X_MAX);

    QValueAxis *yAxisPress = new QValueAxis;
    yAxisPress->setLabelsVisible(false);
    yAxisPress->setRange(CHART_Y_MIN, CHART_Y_MAX);

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
    emit emitWriteSerailData(data);

    QtConcurrent::run(this, &MainWindow::readSerialData);
}

void MainWindow::readSerialData()
{
    QList<QPointF> ecgPoints, tempPoints, thresPoints, pressPoints;

    QByteArray readBuffer;

    int i = CHART_X_MIN;
    int j = CHART_X_MIN;

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
                    ecgPoints.insert(i, QPointF(i, dataValue));

                    if(dataValue > static_cast<double>(threshold) && !isThresholdPassed)
                    {
                        thresPoints.insert(i, QPointF(i, 16));
                        isThresholdPassed = true;
                    }
                    else if(dataValue < static_cast<double>(threshold) && isThresholdPassed)
                    {
                        thresPoints.insert(i, QPointF(i, 0));
                        isThresholdPassed = false;
                    }
                    else
                    {
                        thresPoints.insert(i, QPointF(i, 0));
                    }

                    if (i == CHART_X_MAX)
                    {
                        i = CHART_X_MIN;

                        if(!updateECGChart)
                        {
                            oldEcgDataPoints = ecgDataPoints;
                            ecgDataPoints = ecgPoints;

                            oldThresholdPoints = thresholdPoints;
                            thresholdPoints = thresPoints;

                            updateECGChart = true;
                        }

                        ecgPoints.clear();
                        thresPoints.clear();
                    }
                    else
                    {
                        i++;
                    }
                }
                else if(readBuffer.at(0) == '\xBB')
                {
                    pressPoints.insert(i, QPointF(i, dataValue));

                    if (j == CHART_X_MAX)
                    {
                        j = CHART_X_MIN;

                        if(!updatePRESSChart)
                        {
                            oldPressDataPoints = pressDataPoints;
                            pressDataPoints = pressPoints;

                            updatePRESSChart = true;
                        }

                        pressPoints.clear();
                    }
                    else
                    {
                        j++;
                    }
                }
                else if(readBuffer.at(0) == '\xDD')
                {
                    ui->label_temprature->setText(QString::number(dataValue));
                }

                readBuffer.remove(0, 3);
            }
            else if(readBuffer.at(0) == '\xCC')
            {
                QByteArray data;
                data.append(static_cast<char>(threshold));
                emit emitWriteSerailData(data);
                readBuffer.remove(0, 1);
            }
            else
            {
                readBuffer.remove(0, 1);
            }
        }
    }
}

void MainWindow::updateEcgChart()
{
    if(ecgDataPoints.isEmpty())
        return;

    if(!updateECGChart)
        return;

    if (ecgChartUpdateIndex >= CHART_X_MIN)
    {
        if(oldEcgDataPoints.isEmpty())
            ecgSeries->append(ecgDataPoints.at(ecgChartUpdateIndex));
        else
            ecgSeries->replace(oldEcgDataPoints.at(ecgChartUpdateIndex), ecgDataPoints.at(ecgChartUpdateIndex));

        if(oldThresholdPoints.isEmpty())
        {
            if(thresholdPoints.at(ecgChartUpdateIndex).y() > 0)
            {
                thresholdMarkerSeries->append(thresholdPoints.at(ecgChartUpdateIndex));
            }
        }
        else
        {
            if(oldThresholdPoints.at(ecgChartUpdateIndex).y() > 0 && thresholdPoints.at(ecgChartUpdateIndex).y() == 0)
            {
                thresholdMarkerSeries->remove(oldThresholdPoints.at(ecgChartUpdateIndex));
            }
            else if(oldThresholdPoints.at(ecgChartUpdateIndex).y() == 0 && thresholdPoints.at(ecgChartUpdateIndex).y() > 0)
            {
                thresholdMarkerSeries->append(thresholdPoints.at(ecgChartUpdateIndex));
            }
        }

        ecgChartUpdateIndex--;
    }
    else
    {
        ecgChartUpdateIndex = CHART_X_MAX;
        updateECGChart = false;
    }
}

void MainWindow::updatePressChart()
{
    if(pressDataPoints.isEmpty())
        return;

    if(!updatePRESSChart)
        return;

    if (pressChartUpdateIndex >= CHART_X_MIN)
    {
        if(oldPressDataPoints.isEmpty())
            pressSeries->append(pressDataPoints.at(pressChartUpdateIndex));
        else
            pressSeries->replace(oldPressDataPoints.at(pressChartUpdateIndex), pressDataPoints.at(pressChartUpdateIndex));

        pressChartUpdateIndex--;
    }
    else
    {
        pressChartUpdateIndex = CHART_X_MAX;
        updatePRESSChart = false;
    }
}

void MainWindow::writeSerialData(QByteArray data)
{
    if (serialPort.write(data) == -1)
        qDebug() << "Error writing to serial port: " << serialPort.errorString();
}

void MainWindow::handleSerialPortError(QSerialPort::SerialPortError error)
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

//void MainWindow::on_pushButton_data_save_clicked()
//{
//    QString fileName = QFileDialog::getSaveFileName(this, "Save Data", QDir::homePath(), "Data Files (*.dat);;All Files (*)");

//    if (fileName.isEmpty())
//        return;

//    QFile file(fileName);
//    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        QTextStream out(&file);
//        // Save the data from the ecgData map to the file in the desired format
//        for (const QPointF& point : ecgDataPoints)
//        {
//            qint64 timestamp = point.x();
//            int ecgAmplitude = point.y();
//            out << timestamp << "," << ecgAmplitude << "\n";
//        }

//        file.close();
//    }
//    else
//    {
//        // Handle file open error if needed
//        QMessageBox::critical(this, "Error", "Failed to save data to the file.");
//    }
//}

void MainWindow::on_pushButton_threshold_save_clicked()
{
    if(ui->lineEdit_threshold->text().toUInt() < 0 || ui->lineEdit_threshold->text().toUInt() > 255)
    {
        QMessageBox::critical(this, "Error", "Threshould should be in Range: [0-255]");
        return;
    }

    threshold = ui->lineEdit_threshold->text().toUInt();
    thresholdSeries->clear();
    thresholdSeries->append(CHART_X_MIN,threshold);
    thresholdSeries->append(CHART_X_MAX,threshold);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "Closing...";

    //    dataManagementThread->requestInterruption();
    //    dataManagementThread->wait(); // Wait for the thread to finish
    //    delete dataManagementThread;

    disconnect(ecgChartUpdateTimer, &QTimer::timeout, this, &MainWindow::updateEcgChart);
    disconnect(this, &MainWindow::emitWriteSerailData, this, &MainWindow::writeSerialData);
    disconnect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleSerialPortError);

    serialPort.close();

    delete ui;
    QMainWindow::closeEvent(event);
    QCoreApplication::quit();
}

MainWindow::~MainWindow()
{

}
