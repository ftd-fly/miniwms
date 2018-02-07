#ifndef RADIOFREQUENCY_H
#define RADIOFREQUENCY_H

#include <QObject>
#include <QTimer>
#include <QSerialPort>
#include <QQueue>

#define RADOI_FREQUENCY_ADDRESS_A  0x08
#define RADOI_FREQUENCY_ADDRESS_B  0x07

class RadioFrequency : public QObject
{
    Q_OBJECT
public:
    explicit RadioFrequency(QObject *parent = nullptr);
    ~RadioFrequency();
    bool init();
signals:
    void buttonClick(int address);
public slots:
    //做一个定时器发送，防止黏包现象。可能对方并没有处理黏包
    void onSend();
    void queryStatus();
    void onRead();
    void lightOn(int address);
    void lightOff(int address);
private:
    QSerialPort *serial;
    QTimer queryTimer;
    QTimer sendTimer;
    typedef enum{
        SEND_TYPE_QUERY = 0,
        SEND_TYPE_LIGHT = 1,
    } SEND_TYPE;
    QQueue< QPair<SEND_TYPE,QByteArray> > sendQueue;
    volatile bool queryOk;
    volatile bool lightOk;
};

#endif // RADIOFREQUENCY_H
