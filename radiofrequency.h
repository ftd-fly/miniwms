#ifndef RADIOFREQUENCY_H
#define RADIOFREQUENCY_H

#include <QObject>
#include <QTimer>
#include <QSerialPort>

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
    void queryStatus();
    void onRead();
    void lightOn(int address);
    void lightOff(int address);
private:
    QSerialPort *serial;
    QTimer queryTimer;
};

#endif // RADIOFREQUENCY_H
