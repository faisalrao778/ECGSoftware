#ifndef DATAMANAGEMENTTHREAD_H
#define DATAMANAGEMENTTHREAD_H

#include <QThread>
#include <QVector>
#include <QPointF>
#include <QDateTime>

class DataManagementThread : public QThread
{
    Q_OBJECT

public:
    DataManagementThread(QVector<QPointF>& ecgDataPoints,
                         QVector<QPointF>& tempDataPoints,
                         QVector<QPointF>& thresholdPoints,
                         QVector<QPointF>& pressDataPoints);

protected:
    void run() override;

private:
    QVector<QPointF>& ecgDataPoints_;
    QVector<QPointF>& tempDataPoints_;
    QVector<QPointF>& thresholdPoints_;
    QVector<QPointF>& pressDataPoints_;

    void removeOldData(QVector<QPointF>& data, qint64 maxDataAge);
};

#endif // DATAMANAGEMENTTHREAD_H
