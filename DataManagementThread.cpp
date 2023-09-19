#include "DataManagementThread.h"
#include <QDebug>

DataManagementThread::DataManagementThread(QVector<QPointF>& ecgDataPoints, QVector<QPointF>& tempDataPoints,
                                           QVector<QPointF>& thresholdPoints, QVector<QPointF>& pressDataPoints)
    : ecgDataPoints_(ecgDataPoints),
      tempDataPoints_(tempDataPoints),
      thresholdPoints_(thresholdPoints),
      pressDataPoints_(pressDataPoints)
{}

void DataManagementThread::run() {
    const qint64 maxDataAge = 10000; // Maximum age of data to keep (5 seconds in milliseconds)

    while (!isInterruptionRequested())
    {
        // Remove older data from the vectors
        removeOldData(ecgDataPoints_, maxDataAge);
        removeOldData(tempDataPoints_, maxDataAge);
        removeOldData(thresholdPoints_, maxDataAge);
        removeOldData(pressDataPoints_, maxDataAge);

        sleep(5); // Adjust the sleep duration as needed
    }
}

void DataManagementThread::removeOldData(QVector<QPointF>& data, qint64 maxDataAge)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    int removedCount = 0;

    while (!data.isEmpty() && data.first().x() < currentTime - maxDataAge)
    {
        data.removeFirst();
        removedCount++;
    }

    if (removedCount > 0)
    {
        qDebug() << "Removed" << removedCount << "old data points from" << data;
    }
}
