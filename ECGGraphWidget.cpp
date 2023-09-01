#include "ECGGraphWidget.h"

ECGGraphWidget::ECGGraphWidget(QWidget *parent) : QWidget(parent)
{
    QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    this->setGeometry(availableGeometry.x(), availableGeometry.y(), availableGeometry.width(), availableGeometry.height()-100);
    this->move(availableGeometry.topLeft());

    startTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    lastReceivedTimestamp = 0;

    setupGraph();
    setupSerialPort();

    QtConcurrent::run(this, &ECGGraphWidget::startReading);

    connect(this, &ECGGraphWidget::dataProcessed, this, &ECGGraphWidget::updateData);
}

void ECGGraphWidget::setupGraph()
{
    //SETUP ECG CHART

    QMargins margins(0, 0, 0, 0);

    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Window, Qt::white);

    ecgSeries = new QLineSeries();
    tmpSeries = new QLineSeries();

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

    chartView = new QChartView(ecgChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(margins);
    chartView->setPalette(palette);
    chartView->setStyleSheet("QChartView { border: 1px solid black; }");
    chartView->setFrameShape(QFrame::Box);

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

    QPushButton *saveButton = new QPushButton("Save Data", this);
    saveButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QLabel *ecgLabel = new QLabel("ECG");
    ecgLabel->setAlignment(Qt::AlignCenter);

    QLabel *tmpLabel = new QLabel("TEMPRATURE");
    tmpLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QVBoxLayout *settingLayout = new QVBoxLayout(this);

    QVBoxLayout *thresholdSettingLayout = new QVBoxLayout(this);
    QLabel *thresholdLabel = new QLabel("Threshold");
    QLineEdit *thresholdInput = new QLineEdit();
    QPushButton *saveThreshold = new QPushButton("SAVE");
    thresholdSettingLayout->addWidget(thresholdLabel);
    thresholdSettingLayout->addWidget(thresholdInput);
    thresholdSettingLayout->addWidget(saveThreshold);

    settingLayout->addItem(thresholdSettingLayout);

    QVBoxLayout *chartLayout = new QVBoxLayout(this);
    chartLayout->addWidget(chartView);
    chartLayout->addWidget(ecgLabel);
    chartLayout->addWidget(tmpChartView);
    chartLayout->addWidget(tmpLabel);
    chartLayout->addWidget(saveButton);

    mainLayout->addItem(settingLayout);
    mainLayout->addItem(chartLayout);

    this->setLayout(mainLayout);
    //this->resize(1200,600);

    connect(saveButton, &QPushButton::clicked, this, &ECGGraphWidget::saveData);
}

void ECGGraphWidget::setupSerialPort()
{
    serialPort.setPortName("COM12");
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

                if(readBuffer.at(0) == '\xAA')
                    ecgDataPoints.append(QString::number(timestamp - startTimestamp) + "|" + QString::number(value));
                else if(readBuffer.at(0) == '\xBB')
                    tempDataPoints.append(QString::number(timestamp - startTimestamp) + "|" + QString::number(value));

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
            else
            {
                readBuffer.remove(0, 1);
            }
        }
    }
}

void ECGGraphWidget::updateData(QStringList list,QString type)
{
    if(list.isEmpty())
        return;

    if(type.compare("ECG")==0)
    {        
        for (int i = 0 ; i < list.size() ; i++)
        {
            qint64 timestamp = list.at(i).split("|").at(0).toLongLong();
            int ecgValue = list.at(i).split("|").at(1).toInt();
            ecgDataPoints.append(QPointF(timestamp, ecgValue));
        }

        ecgSeries->replace(ecgDataPoints);

        qint64 lastTimestamp = ecgDataPoints.last().x();

        ecgChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);
        ecgChart->axisY()->setRange(0, 32768); //changed from 65536
    }
    else if(type.compare("TEMP")==0)
    {
        for (int i = 0 ; i < list.size() ; i++)
        {
            qint64 timestamp = list.at(i).split("|").at(0).toLongLong();
            int tmpValue = list.at(i).split("|").at(1).toInt();
            tempDataPoints.append(QPointF(timestamp, tmpValue));
        }

        tmpSeries->replace(tempDataPoints);

        qint64 lastTimestamp = tempDataPoints.last().x();

        tmpChart->axisX()->setRange(lastTimestamp - 3000, lastTimestamp);
        tmpChart->axisY()->setRange(0, 65536);
    }
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
