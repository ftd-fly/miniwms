#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QTimer>
#include <QQueue>
#include <QMap>

#define RADOI_FREQUENCY_ADDRESS_A  0x08
#define RADOI_FREQUENCY_ADDRESS_B  0x07
#define RADOI_FREQUENCY_ADDRESS_C  0x81
#define RADOI_FREQUENCY_ADDRESS_D  0x82

class SerialThread : public QThread
{
    Q_OBJECT
public:
    explicit SerialThread(QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void buttonClick(int address);
private slots:
    void slot_sendData(const QByteArray&);
    void onSendTimer();
    void onLightTimer();
    void onQueryTimer();
public slots:
    void lightOn(int address);
    void lightOff(int address);
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

    QTimer *queryTimer;
    QTimer *sendTimer;
    QTimer *lightTimer;
    typedef enum{
        SEND_TYPE_QUERY = 0,
        SEND_TYPE_LIGHT = 1,
    } SEND_TYPE;
    QQueue< QPair<SEND_TYPE,QByteArray> > sendQueue;
    QMap<int,bool> address_on_off;//灯的地址和灯的状态是否亮起

};

#endif // SERIALTHREAD_H
