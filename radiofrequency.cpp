#include "radiofrequency.h"
#include <QSerialPort>
#include "global.h"
#include "crc16.h"

RadioFrequency::RadioFrequency(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);
    connect(serial, &QSerialPort::readyRead,this, &RadioFrequency::onRead);

    queryTimer.setInterval(150);
    connect(&queryTimer,&QTimer::timeout,this,&RadioFrequency::queryStatus);
}

RadioFrequency::~RadioFrequency()
{
    serial->close();
}

bool RadioFrequency::init()
{
    //从配置文件中读取配置

    //COM口
    serial->setPortName(configure.getValue("RadioFrequency/COM").toString());

    //奇偶校验
    serial->setParity(configure.getValue("RadioFrequency/Parity").toInt());

    //波特率
    serial->setBaudRate(configure.getValue("RadioFrequency/BaudRate").toInt());

    //数据位
    serial->setDataBits(configure.getValue("RadioFrequency/DataBits").toInt());

    //停止位
    serial->setStopBits(configure.getValue("RadioFrequency/StopBits").toInt());

    if(!serial->open(QIODevice::ReadWrite))
    {
        qDebug<<"serial open fail!"<< serial->errorString();
        QMessageBox::warning(null,QStringLiteral("错误"),QStringLiteral("无法打开串口"),QMessageBox::Ok);
        return false;
    }
    queryTimer.start();
    //成功
    return true;
}


void RadioFrequency::onRead()
{
    static QByteArray buffer;
    buffer += serial->readAll();
    while(true){
        int index = buffer.indexOf(0x81);
        if(index<0)return ;
        if(index +7>=buffer.length())return ;

        QByteArray msg = buffer.mid(index,7);
        //TODO:等待他修改协议或者给一个更清楚的协议
        buffer = buffer.right(buffer.length()-index-7);
    }
}

void RadioFrequency::lightOn(int address)
{
    QByteArray sendCmd = {(char )(address&0xff),  0x10, 0x00, 0x02, 0x00, 0x01, 0x02,0x00,0x01,};
    qint16 crc = checksum(sendCmd);
    sendCmd += (char )(crc>>8 & 0xff);
    sendCmd += (char )(crc & 0xff);
    if(serial->isOpen() && serial->isWritable())
    {
        serial->write(query81);
        serial->write(query82);
    }
}

void RadioFrequency::lightOff(int address)
{
    QByteArray sendCmd = {(char )(address&0xff),  0x10, 0x00, 0x02, 0x00, 0x01, 0x02,0x00,0x00,};
    qint16 crc = checksum(sendCmd);
    sendCmd += (char )(crc>>8 & 0xff);
    sendCmd += (char )(crc & 0xff);
    if(serial->isOpen() && serial->isWritable())
    {
        serial->write(query81);
        serial->write(query82);
    }
}

void RadioFrequency::queryStatus()
{
    QByteArray query81 = {0x81, 0x03, 0x00, 0x08, 0x00, 0x01,};
    QByteArray query82 = {0x82, 0x03, 0x00, 0x08, 0x00, 0x01,};
    qint16 crc81 = checksum(query81);
    qint16 crc82 = checksum(query82);

    query81 += (char )(crc81>>8 & 0xff);
    query81 += (char )(crc81 & 0xff);

    query82 += (char )(crc82>>8 & 0xff);
    query82 += (char )(crc82 & 0xff);

    if(serial->isOpen() && serial->isWritable())
    {
        serial->write(query81);
        serial->write(query82);
    }
}
