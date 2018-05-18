#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QSerialPort>

class SerialThread : public QThread
{
    Q_OBJECT
public:
    explicit SerialThread(QString portName,
                          QSerialPort::Parity _parity,
                          int _baudrate,
                          QSerialPort::DataBits _dataBits,
                          QSerialPort::StopBits _stopBits,
                          QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void sig_readData(const QByteArray&);
public slots:
    void slot_sendData(const QByteArray&);
private slots:
    void slot_dataArrived();
    void slot_error(QSerialPort::SerialPortError);
private:
    QSerialPort *_serialPort;
    QString portName;
    QSerialPort::Parity parity;
    int baudrate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;

};

#endif // SERIALTHREAD_H
