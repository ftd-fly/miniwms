#include "serialthread.h"

SerialThread::SerialThread(QString portName, QSerialPort::Parity _parity, int _baudrate, QSerialPort::DataBits _dataBits, QSerialPort::StopBits _stopBits, QObject *parent) :
    QThread(parent),
    portName(portName),
    parity(_parity),
    baudrate(_baudrate),
    dataBits(_dataBits),
    stopBits(_stopBits)
{
    this->moveToThread(this);
}

void SerialThread::run(){

    _serialPort = new QSerialPort(this);
    _serialPort->setPortName(portName);
    _serialPort->setBaudRate(baudrate);
    _serialPort->setDataBits(dataBits);
    _serialPort->setParity(parity);
    _serialPort->setStopBits(stopBits);

    connect(_serialPort, &QSerialPort::readyRead,
            this, &SerialThread::slot_dataArrived, Qt::QueuedConnection);

    connect(_serialPort, &QSerialPort::errorOccurred, this, &SerialThread::slot_error);

    _serialPort->open(QIODevice::ReadWrite);

    exec();

    _serialPort->close();
    _serialPort->deleteLater();
}

void SerialThread::slot_error(QSerialPort::SerialPortError error) {
    if (QSerialPort::NoError == error) {
        return;
    }
    this->quit();
}
void SerialThread::slot_sendData(const QByteArray &data) {
    _serialPort->write(data);
}
void SerialThread::slot_dataArrived() {
    emit sig_readData(_serialPort->readAll());
}
