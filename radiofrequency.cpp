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
    int p = configure.getValue("RadioFrequency/Parity").toInt();
    QSerialPort::Parity parity = QSerialPort::UnknownParity;
    if(p==0)parity = QSerialPort::NoParity;
    if(p==2)parity = QSerialPort::EvenParity;
    if(p==3)parity = QSerialPort::OddParity;
    if(p==4)parity = QSerialPort::SpaceParity;
    if(p==5)parity = QSerialPort::MarkParity;

    serial->setParity(parity);

    //波特率
    serial->setBaudRate(configure.getValue("RadioFrequency/BaudRate").toInt());

    //数据位
    int d = configure.getValue("RadioFrequency/DataBits").toInt();
    QSerialPort::DataBits dataBits = QSerialPort::UnknownDataBits;
    if(d==5)dataBits = QSerialPort::Data5;
    if(d==6)dataBits = QSerialPort::Data6;
    if(d==7)dataBits = QSerialPort::Data7;
    if(d==8)dataBits = QSerialPort::Data8;
    serial->setDataBits(dataBits);

    //停止位
    int s = configure.getValue("RadioFrequency/StopBits").toInt();
    QSerialPort::StopBits stopBits = QSerialPort::UnknownStopBits;
    if(s==1)stopBits = QSerialPort::OneStop;
    if(s==2)stopBits = QSerialPort::TwoStop;
    if(s==3)stopBits = QSerialPort::OneAndHalfStop;

    serial->setStopBits(stopBits);

    if(!serial->open(QIODevice::ReadWrite))
    {
        qDebug()<<"serial open fail!"<< serial->errorString();
        QMessageBox::warning(NULL,QStringLiteral("错误"),QStringLiteral("无法打开串口"),QMessageBox::Ok);
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
    while(true)
    {
        int index81 = buffer.indexOf(0x81);
        int index82 = buffer.indexOf(0x82);
        if(index81<0&&index82<0)return ;
        int index = index81;
        if(index81<0)index = index82;
        if(index81>=0 && index82>=0 && index82<index81)index = index82;
        if(index+7>=buffer.length())return ;
        //这里判定了要处理的是不是index81
        if(0x03 == buffer.at(index+1))
        {
            //长度为7
            QByteArray statusMsg = buffer.mid(index,7);
            int status = ((statusMsg.at(3))<<8 &0xff00)|((statusMsg.at(4)&0xff));
            if(status!=0){
                //按钮被按下
                //亮灯
                lightOn(statusMsg.at(0));
                //发出信号
                emit buttonClick(statusMsg.at(0));
            }

            buffer = buffer.right(buffer.length()-index-7);
        }else if(0x10 == buffer.at(index+1))
        {
            //长度为8
            QByteArray setLightMsg = buffer.mid(index,8);
            //TODO:设置灯成功？是否需要进行处理呢？不需要
            buffer = buffer.right(buffer.length()-index-8);
        }
    }
}

void RadioFrequency::lightOn(int address)
{
    const unsigned char str[] = {(address&0xff),  0x10, 0x00, 0x02, 0x00, 0x01, 0x02,0x00,0x01,};
    QByteArray sendCmd(reinterpret_cast<const char*>(&str[0]),std::extent<decltype(str)>::value);

//    QByteArray sendCmd =
    qint16 crc = checksum(sendCmd);
    sendCmd += (char )(crc>>8 & 0xff);
    sendCmd += (char )(crc & 0xff);
    if(serial->isOpen() && serial->isWritable())
    {
        serial->write(sendCmd);
    }
}

void RadioFrequency::lightOff(int address)
{
    const unsigned char str[] = {(address&0xff),  0x10, 0x00, 0x02, 0x00, 0x01, 0x02,0x00,0x00,};
    QByteArray sendCmd(reinterpret_cast<const char*>(&str[0]),std::extent<decltype(str)>::value);
    qint16 crc = checksum(sendCmd);
    sendCmd += (char )(crc>>8 & 0xff);
    sendCmd += (char )(crc & 0xff);
    if(serial->isOpen() && serial->isWritable())
    {
        serial->write(sendCmd);
    }
}

void RadioFrequency::queryStatus()
{
    const unsigned char str1[] = {0x81, 0x03, 0x00, 0x08, 0x00, 0x01,};
    QByteArray query81(reinterpret_cast<const char*>(&str1[0]),std::extent<decltype(str1)>::value);
    const unsigned char str2[] = {0x82, 0x03, 0x00, 0x08, 0x00, 0x01,};
    QByteArray query82(reinterpret_cast<const char*>(&str2[0]),std::extent<decltype(str2)>::value);

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
