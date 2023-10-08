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
    DataManagementThread(QMutex *ecgMutex,
                         QVector<QPointF>& ecgData,
                         QMutex *pressMutex,
                         QVector<QPointF>& pressData);

protected:
    void run() override;

private:
    const int MAX_VECTOR_SIZE = 15000;

    QMutex *ecgMutex_;
    QMutex *pressMutex_;

    QVector<QPointF>& ecgData_;
    QVector<QPointF>& pressData_;

    void removeOldECGData();
    void removeOldPressData();
};

#endif // DATAMANAGEMENTTHREAD_H
