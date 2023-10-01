#include "DataManagementThread.h"
#include <QDebug>

DataManagementThread::DataManagementThread(QMutex& ecgMutex,
                                           QMutex& pressMutex,
                                           QVector<QPointF>& ecgDataPoints,
                                           QVector<QPointF>& pressDataPoints)
    : ecgMutex(ecgMutex),
      pressMutex(pressMutex),
      ecgDataPoints(ecgDataPoints),
      pressDataPoints(pressDataPoints)
{}

void DataManagementThread::run()
{
    while (!isInterruptionRequested())
    {
        removeOldData(ecgDataPoints, "ECG");
        removeOldData(pressDataPoints, "PRESS");
        sleep(5);
    }
}

void DataManagementThread::removeOldData(QVector<QPointF>& data, QString type)
{
    if(type.compare("ECG")==0)          ecgMutex.lock();
    else if(type.compare("PRESS")==0)   pressMutex.lock();

    if (data.size() > MAX_VECTOR_SIZE)  data.remove(0, data.size() - MAX_VECTOR_SIZE);

    if(type.compare("ECG")==0)          ecgMutex.unlock();
    else if(type.compare("PRESS")==0)   pressMutex.unlock();
}
