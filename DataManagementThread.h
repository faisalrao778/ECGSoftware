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
    DataManagementThread(QMutex& ecgMutex,
                         QMutex& pressMutex,
                         QVector<QPointF>& ecgDataPoints,
                         QVector<QPointF>& pressDataPoints);

protected:
    void run() override;

private:
    const int MAX_VECTOR_SIZE = 15000;

    QMutex& ecgMutex;
    QMutex& pressMutex;
    QVector<QPointF>& ecgDataPoints;
    QVector<QPointF>& pressDataPoints;

    void removeOldData(QVector<QPointF>& data, QString type);
};

#endif // DATAMANAGEMENTTHREAD_H
