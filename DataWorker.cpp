#include "DataWorker.h"

DataWorker::DataWorker(QObject *parent) : QObject(parent)
{
    serialPort.setPortName("COM10");
    serialPort.setBaudRate(QSerialPort::Baud115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed To Open Serial Port!";
        return;
    }
}

void DataWorker::startReading()
{
    if(!serialPort.isOpen())
        return;

    QByteArray readBuffer;

    while (true)
    {
        readBuffer.append(serialPort.readAll());

        while (readBuffer.size() >= 3)
        {
            // qDebug()<<"readBuffer: "<<static_cast<qint8>(readBuffer.at(0));
            if ((readBuffer.at(0)) == '\xAA')
            {
                uchar lsb = static_cast<uchar>(readBuffer.at(1));
                uchar msb = static_cast<uchar>(readBuffer.at(2));

                int ecgValue = (msb << 8) | lsb;
                qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

                if (timestamp - lastReceivedTimestamp >= 100)
                {
                    emit dataProcessed(timestamp, ecgValue);
                }

                lastReceivedTimestamp = timestamp;

                readBuffer.remove(0, 3);
            }
            else
            {
                readBuffer.remove(0, 1);
            }
        }
    }
}
