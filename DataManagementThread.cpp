#include "DataManagementThread.h"
#include <QDebug>

DataManagementThread::DataManagementThread(QMutex *mutex, QVector<QPointF>& ecgDataPoints, QVector<QPointF>& tempDataPoints,
                                           QVector<QPointF>& thresholdPoints, QVector<QPointF>& pressDataPoints)
    : mutex_(mutex),
      ecgDataPoints_(ecgDataPoints),
      tempDataPoints_(tempDataPoints),
      thresholdPoints_(thresholdPoints),
      pressDataPoints_(pressDataPoints)
{}

void DataManagementThread::run() {
    while (!isInterruptionRequested())
    {
        // Remove older data from the vectors
        removeOldData(ecgDataPoints_);
        removeOldData(tempDataPoints_);
        removeOldData(thresholdPoints_);
        removeOldData(pressDataPoints_);

        sleep(5); // Adjust the sleep duration as needed
    }
}

void DataManagementThread::removeOldData(QVector<QPointF>& data)
{
    mutex_->lock();

    if (data.size() > MAX_VECTOR_SIZE)
        data.remove(0, data.size() - MAX_VECTOR_SIZE);

    mutex_->unlock();
}
