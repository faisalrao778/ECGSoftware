#include "DataManagementThread.h"
#include <QDebug>

DataManagementThread::DataManagementThread(qint64& startTimestamp,
                                           QVector<QPointF>& ecgDataPoints, QVector<QPointF>& tempDataPoints,
                                           QVector<QPointF>& thresholdPoints, QVector<QPointF>& pressDataPoints)
    : startTimestamp_(startTimestamp),
      ecgDataPoints_(ecgDataPoints),
      tempDataPoints_(tempDataPoints),
      thresholdPoints_(thresholdPoints),
      pressDataPoints_(pressDataPoints)
{}

void DataManagementThread::run() {
    const qint64 maxDataAge = 5000; // Maximum age of data to keep (5 seconds in milliseconds)

    while (!isInterruptionRequested())
    {
        // Remove older data from the vectors
        removeOldData(ecgDataPoints_, maxDataAge, "ECG Data Points");
        removeOldData(tempDataPoints_, maxDataAge, "TEMP Data Points");
        removeOldData(thresholdPoints_, maxDataAge, "Threadhold Data Points");
        removeOldData(pressDataPoints_, maxDataAge, "PRESS Data Points");

        sleep(5); // Adjust the sleep duration as needed
    }
}

void DataManagementThread::removeOldData(QVector<QPointF>& data, qint64 maxDataAge, QString remarks)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    int removedCount = 0;

    while (!data.isEmpty() && (data.first().x() + startTimestamp_) < currentTime - maxDataAge)
    {
        data.removeFirst();
        removedCount++;
    }

    qDebug() << "Removed" << removedCount << "Old Data Points From" << remarks;
}
