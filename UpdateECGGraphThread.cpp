#include "UpdateECGGraphThread.h"
#include <QDebug>

UpdateECGGraphThread::UpdateECGGraphThread(QMutex& ecgMutex,
                                           qint64 startTimestamp,
                                           QVector<QPointF>& ecgDataPoints,
                                           QLineSeries* ecgSeries,
                                           QScatterSeries* thresholdMarkerSeries)
    : ecgMutex(ecgMutex),
      startTimestamp(startTimestamp),
      ecgDataPoints(ecgDataPoints),
      ecgSeries(ecgSeries),
      thresholdMarkerSeries(thresholdMarkerSeries)
{

}

void UpdateECGGraphThread::run()
{
    while (!isInterruptionRequested())
    {
        sleep(2);
        updateGraph();
    }
}

void UpdateECGGraphThread::updateGraph()
{
    qDebug() << "UpdateECGGraphThread::updateGraph()";

    if(ecgDataPoints.isEmpty())
        return;

    ecgMutex.lock();

    qint64 lastTimestamp = ecgDataPoints.last().x() + startTimestamp;
    qint64 fromTimestamp = lastTimestamp - 3000;

    for (const QPointF& point : ecgDataPoints)
    {
        if(point.x()+startTimestamp<fromTimestamp)
            continue;

        emit emitCharDataPoint(point);
    }

    ecgMutex.unlock();
}
