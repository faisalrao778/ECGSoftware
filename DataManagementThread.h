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
    DataManagementThread(QMutex *mutex,
                         QVector<QPointF>& ecgDataPoints,
                         QVector<QPointF>& tempDataPoints,
                         QVector<QPointF>& thresholdPoints,
                         QVector<QPointF>& pressDataPoints);

protected:
    void run() override;

private:
    const int MAX_VECTOR_SIZE = 15000;

    QMutex *mutex_;
    QVector<QPointF>& ecgDataPoints_;
    QVector<QPointF>& tempDataPoints_;
    QVector<QPointF>& thresholdPoints_;
    QVector<QPointF>& pressDataPoints_;

    void removeOldData(QVector<QPointF>& data);
};

#endif // DATAMANAGEMENTTHREAD_H
