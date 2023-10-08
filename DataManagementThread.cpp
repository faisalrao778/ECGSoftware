#include "DataManagementThread.h"
#include <QDebug>

DataManagementThread::DataManagementThread(QMutex *ecgMutex,
                                           QVector<QPointF>& ecgData,
                                           QMutex *pressMutex,
                                           QVector<QPointF>& pressData)
    : ecgMutex_(ecgMutex),
      ecgData_(ecgData),
      pressMutex_(pressMutex),
      pressData_(pressData)
{

}

void DataManagementThread::run()
{
    while (!isInterruptionRequested())
    {
        removeOldECGData();
        removeOldPressData();

        sleep(5);
    }
}

void DataManagementThread::removeOldECGData()
{
    ecgMutex_->lock();

    if (ecgData_.size() > MAX_VECTOR_SIZE)
        ecgData_.remove(0, ecgData_.size() - MAX_VECTOR_SIZE);

    ecgMutex_->unlock();
}

void DataManagementThread::removeOldPressData()
{
    pressMutex_->lock();

    if (pressData_.size() > MAX_VECTOR_SIZE)
        pressData_.remove(0, pressData_.size() - MAX_VECTOR_SIZE);

    pressMutex_->unlock();
}
