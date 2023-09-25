#ifndef DATAMANAGEMENTTHREAD_H
#define DATAMANAGEMENTTHREAD_H

#include <QThread>
#include <QVector>
#include <QPointF>
#include <QDateTime>
#include <QMutex>

class DataManagementThread : public QThread
{
    Q_OBJECT

public:
    DataManagementThread(QMutex *sharedMutex,
                         qint64& startTimestamp,
                         QVector<QPointF>& ecgDataPoints,
                         QVector<QPointF>& tempDataPoints,
                         QVector<QPointF>& thresholdPoints,
                         QVector<QPointF>& pressDataPoints);

    QMutex *sharedMutex_;

protected:
    void run() override;

private:
    qint64& startTimestamp_;
    QVector<QPointF>& ecgDataPoints_;
    QVector<QPointF>& tempDataPoints_;
    QVector<QPointF>& thresholdPoints_;
    QVector<QPointF>& pressDataPoints_;

    void removeOldData(QVector<QPointF>& data, qint64 maxDataAge, QString remarks);
};

#endif // DATAMANAGEMENTTHREAD_H
