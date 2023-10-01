#ifndef UPDATEECGGRAPHTHREAD_H
#define UPDATEECGGRAPHTHREAD_H

#include <QThread>
#include <QVector>
#include <QPointF>
#include <QDateTime>
#include <QMutex>
#include <QtCharts>

class UpdateECGGraphThread : public QThread
{
    Q_OBJECT

public:
    UpdateECGGraphThread(QMutex& ecgMutex,
                         qint64 startTimestamp,
                         QVector<QPointF>& ecgDataPoints,
                         QLineSeries* ecgSeries,
                         QScatterSeries* thresholdMarkerSeries);

protected:
    void run() override;

private:
    QMutex& ecgMutex;
    QVector<QPointF>& ecgDataPoints;
    QLineSeries* ecgSeries;
    QScatterSeries* thresholdMarkerSeries;
    qint64 startTimestamp;

    void updateGraph();

signals:
    void emitCharDataPoint(QPointF);
};

#endif // UPDATEECGGRAPHTHREAD_H
