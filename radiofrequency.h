#ifndef RADIOFREQUENCY_H
#define RADIOFREQUENCY_H

#include <QObject>
#include <QTimer>
#include <QQueue>
#include <QMap>
#include "serialthread.h"

#define RADOI_FREQUENCY_ADDRESS_A  0x08
#define RADOI_FREQUENCY_ADDRESS_B  0x07

#define RADOI_FREQUENCY_ADDRESS_C  0x81
#define RADOI_FREQUENCY_ADDRESS_D  0x82

class RadioFrequency : public QObject
{
    Q_OBJECT
public:
    explicit RadioFrequency(QObject *parent = nullptr);
    ~RadioFrequency();
    bool init();
signals:
    void buttonClick(int address);
    void sig_wirteSerial(const QByteArray &b);
public slots:
    //做一个定时器发送，防止黏包现象。可能对方并没有处理黏包
    void onSend();
    void onLightTimer();
    void queryStatus();
    void onRead(const QByteArray &qba);
    void lightOn(int address);
    void lightOff(int address);
private:
    SerialThread *serial;
    QTimer queryTimer;
    QTimer sendTimer;
    QTimer lightTimer;
    typedef enum{
        SEND_TYPE_QUERY = 0,
        SEND_TYPE_LIGHT = 1,
    } SEND_TYPE;
    QQueue< QPair<SEND_TYPE,QByteArray> > sendQueue;
    QMap<int,bool> address_on_off;//灯的地址和灯的状态是否亮起
//    volatile bool queryOk;
//    volatile bool lightOk;
};

#endif // RADIOFREQUENCY_H
