#include "ECGGraphWidget.h"
#include "DataWorker.h"

ECGGraphWidget::ECGGraphWidget(QWidget *parent) : QWidget(parent)
{
    setupGraph();

    dataThread = new QThread(this);
    dataWorker = new DataWorker();
    dataWorker->moveToThread(dataThread);

    connect(dataThread, &QThread::started, dataWorker, &DataWorker::startReading);
    connect(dataWorker, &DataWorker::dataProcessed, this, &ECGGraphWidget::updateData);

    dataThread->start();
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
    this->resize(600,400);

    connect(saveButton, &QPushButton::clicked, this, &ECGGraphWidget::saveData);
}

void ECGGraphWidget::updateData(qint64 timestamp, int ecgAmplitude)
{
    qDebug()<<"¨Time : " << timestamp;
    qDebug()<<" Àmp  : " << ecgAmplitude;

    ecgData.insert(timestamp, ecgAmplitude);

    ecgSeries->append(timestamp, ecgAmplitude);

    ecgChart->axisX()->setRange(timestamp - 5000, timestamp); // Show the last 5 seconds of data
    ecgChart->axisY()->setRange(0, 65536); // Adjust the Y-axis range based on your ECG data range
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
        for (auto it = ecgData.begin(); it != ecgData.end(); ++it)
        {
            qint64 timestamp = it.key();
            int ecgAmplitude = it.value();
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
