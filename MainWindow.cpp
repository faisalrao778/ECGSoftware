#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    this->setGeometry(availableGeometry.x(), availableGeometry.y(), availableGeometry.width(), availableGeometry.height()-100);
    this->move(availableGeometry.topLeft());

    startTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    lastReceivedTimestamp = 0;
    threshold = 200;

    ui->lineEdit_threshold->setText(QString::number(threshold));

    setupGraph();
    setupSerialPort();

    connect(this, &MainWindow::dataProcessed, this, &MainWindow::updateData);
}

void MainWindow::setupGraph()
{
    //SETUP ECG CHART

    QMargins margins(0, 0, 0, 0);

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

    QValueAxis *xAxis = new QValueAxis;
    //xAxis->setTitleText("Time");
    xAxis->setTickCount(10);
    xAxis->setReverse(true);
    xAxis->setLabelsVisible(true);

    QValueAxis *yAxis = new QValueAxis;
    //yAxis->setTitleText("Amplitude");
    yAxis->setLabelsVisible(true);

    QValueAxis *xAxisTmp = new QValueAxis;
    //xAxis->setTitleText("Time");
    xAxisTmp->setTickCount(10);
    xAxisTmp->setLabelsVisible(true);
    xAxisTmp->setReverse(true);

    QValueAxis *yAxisTmp = new QValueAxis;
    //yAxisTmp->setTitleText("Amplitude");
    yAxisTmp->setLabelsVisible(true);

    ecgChart = new QChart();
    ecgChart->addSeries(ecgSeries);
    ecgChart->addSeries(thresholdSeries);
    ecgChart->addSeries(thresholdMarkers);
    //ecgChart->setTitle("ECG");
    ecgChart->legend()->setVisible(false);
    ecgChart->addAxis(xAxis, Qt::AlignBottom);
    ecgChart->addAxis(yAxis, Qt::AlignLeft);
    ecgChart->setMargins(margins);

    ecgSeries->attachAxis(xAxis);
    ecgSeries->attachAxis(yAxis);
    ecgSeries->setUseOpenGL(true);
    ecgSeries->setPointsVisible(true);
    ecgSeries->setPointLabelsVisible(true);
    ecgSeries->setPointLabelsFormat("(@xPoint, @yPoint)");
    ecgSeries->setPen(pen);

    thresholdSeries->attachAxis(xAxis);
    thresholdSeries->attachAxis(yAxis);
    thresholdSeries->setPen(pen_threshold);

    thresholdMarkers->attachAxis(xAxis);
    thresholdMarkers->attachAxis(yAxis);
    thresholdMarkers->setMarkerSize(10);
    thresholdMarkers->setColor(Qt::red);
    thresholdMarkers->setMarkerShape(QScatterSeries::MarkerShapeRectangle);

    chartView = new QChartView(ecgChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(margins);
    chartView->setPalette(palette);
    chartView->setStyleSheet("QChartView { border: 1px solid black; }");
    chartView->setFrameShape(QFrame::Box);
    chartView->setMinimumWidth(600);

    //SETUP TEMPRATURE CHART

    tmpChart = new QChart();
    tmpChart->addSeries(tmpSeries);
    //tmpChart->setTitle("Temprature");
    tmpChart->legend()->setVisible(false);
    tmpChart->addAxis(xAxisTmp, Qt::AlignBottom);
    tmpChart->addAxis(yAxisTmp, Qt::AlignLeft);
    tmpChart->setMargins(margins);

    tmpSeries->attachAxis(xAxisTmp);
    tmpSeries->attachAxis(yAxisTmp);
    tmpSeries->setUseOpenGL(true);
    tmpSeries->setPointsVisible(true);
    tmpSeries->setPointLabelsVisible(true);
    tmpSeries->setPointLabelsFormat("(@xPoint, @yPoint)");
    tmpSeries->setPen(pen);

    tmpChartView = new QChartView(tmpChart);
    tmpChartView->setRenderHint(QPainter::Antialiasing);
    tmpChartView->setContentsMargins(margins);
    tmpChartView->setPalette(palette);
    tmpChartView->setStyleSheet("QChartView { border: 1px solid black; }");
    tmpChartView->setFrameShape(QFrame::Box);
    tmpChartView->setMinimumWidth(600);

    ui->gridLayout->addWidget(chartView);
    ui->gridLayout_2->addWidget(tmpChartView);

    ecgChart->axisY()->setRange(0, 256);
    tmpChart->axisY()->setRange(0, 256);

    thresholdSeries->append(0,threshold);
    thresholdSeries->append(1000,threshold);
}

void MainWindow::setupSerialPort()
{
    serialPort.setPortName("COM12");
    serialPort.setBaudRate(QSerialPort::Baud115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);

    connect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    if (!serialPort.open(QIODevice::ReadWrite))
    {
        qDebug() << "Failed To Open Serial Port!";
        return;
    }

    qDebug() << "Opened Serial Port!";
    QtConcurrent::run(this, &MainWindow::startReading);
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

void MainWindow::startReading()
{
    QStringList ecgDataPoints,tempDataPoints;
    QByteArray readBuffer;

    while (true)
    {
        QCoreApplication::processEvents();

        readBuffer.append(serialPort.readAll());

        while (readBuffer.size() >= 3)
        {
            if (readBuffer.at(0) == '\xAA' || readBuffer.at(0) == '\xBB')
            {
                uchar lsb = static_cast<uchar>(readBuffer.at(1));
                uchar msb = static_cast<uchar>(readBuffer.at(2));

                int value = (msb << 8) | lsb;
                qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

                //int dataValue = static_cast<int>((static_cast<double>(value) / 65535.00) * 256);
                double dataValue = (static_cast<double>(value) / 65535.00) * 256.00;
                QString val = QString::number(timestamp - startTimestamp) + "|" + QString::number(dataValue);

                if(readBuffer.at(0) == '\xAA')
                    ecgDataPoints.append(val);
                else if(readBuffer.at(0) == '\xBB')
                    tempDataPoints.append(val);

                if (timestamp - lastReceivedTimestamp >= 50)
                {
                    emit dataProcessed(ecgDataPoints,"ECG");
                    emit dataProcessed(tempDataPoints,"TEMP");
                    ecgDataPoints.clear();
                    tempDataPoints.clear();
                    lastReceivedTimestamp = timestamp;
                }
                readBuffer.remove(0, 3);
            }
            else if(readBuffer.at(0) == '\xCC')
            {
                qDebug() << "Receieved 0xcc";
                QByteArray data;
                data.append(static_cast<char>(threshold));
                int len = serialPort.write(data);
                qDebug() << len << " Byte Written";
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
            ecgDataPoints.append(QPointF(timestamp, ecgValue));
            if(ecgValue > static_cast<double>(threshold))
                thresholdPoints.append(QPointF(timestamp, 16));
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

        tmpSeries->replace(tempDataPoints);

        qint64 lastTimestamp = tempDataPoints.last().x();

        tmpChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);
        tmpChart->axisY()->setRange(0, 256);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::closeEvent(QCloseEvent *event) {
    // Close the serial port here
    serialPort.close(); // Replace serialPort with your QSerialPort instance
    QMainWindow::closeEvent(event);
}
