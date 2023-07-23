#ifndef DATAWORKER_H
#define DATAWORKER_H

#include <QObject>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QDateTime>

class DataWorker : public QObject
{
    Q_OBJECT

public:
    explicit DataWorker(QObject *parent = nullptr);

public slots:
    void startReading();

signals:
    void dataProcessed(qint64 timestamp, int ecgAmplitude);

private:
    qint64 lastReceivedTimestamp;
    QSerialPort serialPort;

};

#endif // DATAWORKER_H
