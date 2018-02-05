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

        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("无法打开串口"),QMessageBox::Yes);
        mbox.setStyleSheet(
                    "QPushButton {"
                    "font:30px;"
                    "padding-left:100px;"
                    "padding-right:100px;"
                    "padding-top:40px;"
                    "padding-bottom:40px;"
                    "}"
                    "QLabel { font:30px;}"
                    );
        mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
        mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));
        mbox.exec();

        return false;
    }
    queryTimer.start();
    //成功
    return true;
}


void RadioFrequency::onRead()
{
    static QByteArray buffer;
    static int APushTime = 0;//计数 A按下的时间
    static int BPushTime = 0;//计数 A按下的时间
    buffer += serial->readAll();
    while(true)
    {
        int indexA = buffer.indexOf(RADOI_FREQUENCY_ADDRESS_A);
        int indexB = buffer.indexOf(RADOI_FREQUENCY_ADDRESS_B);
        if(indexA<0&&indexB<0)return ;
        int index = indexA;
        if(indexA<0)index = indexB;
        if(indexA>=0 && indexB>=0){
            if(indexB<indexA)
                index=indexB;
            else
                index = indexA;
        }
        if(index+7>=buffer.length())return ;
        //这里判定了要处理的是不是整个包
        if(0x03 == buffer.at(index+1))
        {
            //长度为7
            QByteArray statusMsg = buffer.mid(index,7);
            int status = statusMsg.at(3);
            if(statusMsg.at(0) == RADOI_FREQUENCY_ADDRESS_A ){
                if(status==0x03){

                    ++APushTime;
                    if(APushTime>=3)
                    {
                        qDebug()<<"A PUSH!";
                        //按钮被按下
                        //亮灯
                        lightOn(statusMsg.at(0));
                        qDebug()<<"A LIGHT ON!";
                        //发出信号
                        emit buttonClick(statusMsg.at(0));
                        APushTime = 0;
                    }
                }else{
                    APushTime = 0;
                }
            }else if(statusMsg.at(0) == RADOI_FREQUENCY_ADDRESS_B ){
                if(status==0x03){
                    ++BPushTime;
                    if(BPushTime>=3){
                        qDebug("B PUSH!");
                        //按钮被按下
                        //亮灯
                        lightOn(statusMsg.at(0));
                        qDebug()<<"B LIGHT ON!";
                        //发出信号
                        emit buttonClick(statusMsg.at(0));
                        BPushTime = 0;
                    }
                }else{
                    BPushTime = 0;
                }
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
    const unsigned char str[] = {(address&0xff),  0x10, 0x00, 0x02, 0x00, 0x01, 0x02,0x03,0xff,};
    QByteArray sendCmd(reinterpret_cast<const char*>(&str[0]),std::extent<decltype(str)>::value);

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
    //每次轮流查询A按钮和B按钮[原来是一起查询]
    static bool isQueryA = true;
    isQueryA = !isQueryA;

    QByteArray query;
    if(isQueryA){
        //A的地址是0x07
        const unsigned char str[] = {0x07, 0x03, 0x00, 0x08, 0x00, 0x01,};
        query = QByteArray(reinterpret_cast<const char*>(&str[0]),std::extent<decltype(str)>::value);
    }else{
        //B的地址是0x08
        const unsigned char str[] = {0x08, 0x03, 0x00, 0x08, 0x00, 0x01,};
        query = QByteArray(reinterpret_cast<const char*>(&str[0]),std::extent<decltype(str)>::value);
    }

    qint16 crc = checksum(query);

    query += (char )(crc>>8 & 0xff);
    query += (char )(crc & 0xff);

    if(serial->isOpen() && serial->isWritable())
    {
        serial->write(query);
    }
}
