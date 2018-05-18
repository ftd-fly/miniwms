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

    sendTimer.setInterval(100);
    connect(&sendTimer,&QTimer::timeout,this,&RadioFrequency::onSend);

    lightTimer.setInterval(200);
    connect(&lightTimer,&QTimer::timeout,this,&RadioFrequency::onLightTimer);

    //灯的address和 是否亮起
    address_on_off[RADOI_FREQUENCY_ADDRESS_A] = false;
    address_on_off[RADOI_FREQUENCY_ADDRESS_B] = false;
    address_on_off[RADOI_FREQUENCY_ADDRESS_C] = false;
    address_on_off[RADOI_FREQUENCY_ADDRESS_D] = false;
}

RadioFrequency::~RadioFrequency()
{
    serial->close();
    delete serial;
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
    sendTimer.start();
    queryTimer.start();
    lightTimer.start();
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
        if(index+1>=buffer.length())return ;

        //查询按钮状态返回，长度为7
        if(0x03 == buffer.at(index+1))
        {
            if(index+7>=buffer.length())return ;
            //长度为7
            QByteArray statusMsg = buffer.mid(index,7);
            int status = statusMsg.at(3);
            if(statusMsg.at(0) == RADOI_FREQUENCY_ADDRESS_A ){
//                queryOk  = true;
                if(status==0x03){

                    ++APushTime;
                    if(APushTime>=2)
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
//                queryOk  = true;
                if(status==0x03)
                {
                    ++BPushTime;
                    if(BPushTime>=2){
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
            if(index+8>=buffer.length())return ;
            //长度为8
            QByteArray setLightMsg = buffer.mid(index,8);
//            //这里其实分不清，是light的开还是关[并没有标志]
//            lightOk = true;
            buffer = buffer.right(buffer.length()-index-8);
        }
    }
}

void RadioFrequency::lightOn(int address)
{
    address_on_off[address] = true;
    address_on_off[RADOI_FREQUENCY_ADDRESS_C] = true;
    address_on_off[RADOI_FREQUENCY_ADDRESS_D] = true;
}

void RadioFrequency::lightOff(int address)
{
    address_on_off[address] = false;

    if(!address_on_off[RADOI_FREQUENCY_ADDRESS_A] && !address_on_off[RADOI_FREQUENCY_ADDRESS_B]){
        address_on_off[RADOI_FREQUENCY_ADDRESS_C] = false;
        address_on_off[RADOI_FREQUENCY_ADDRESS_D] = false;
    }
}

void RadioFrequency::onLightTimer()
{
    unsigned char light_on[] = {0xff,  0x10, 0x00, 0x02, 0x00, 0x01, 0x02,0x03,0xff,};
    unsigned char light_off[] = {0xff,  0x10, 0x00, 0x02, 0x00, 0x01, 0x02,0x00,0x00,};
    for(auto itr = address_on_off.begin();itr!=address_on_off.end();++itr){
        if(itr.value()){
            light_on[0] = itr.key() & 0xff;
            QByteArray sendCmd(reinterpret_cast<const char*>(&light_on[0]),std::extent<decltype(light_on)>::value);
            qint16 crc = checksum(sendCmd);
            sendCmd += (char )(crc>>8 & 0xff);
            sendCmd += (char )(crc & 0xff);
            sendQueue.enqueue(qMakePair(SEND_TYPE_LIGHT, sendCmd));
        }else{
            light_off[0] = itr.key() & 0xff;
            QByteArray sendCmd(reinterpret_cast<const char*>(&light_off[0]),std::extent<decltype(light_off)>::value);
            qint16 crc = checksum(sendCmd);
            sendCmd += (char )(crc>>8 & 0xff);
            sendCmd += (char )(crc & 0xff);
            sendQueue.enqueue(qMakePair(SEND_TYPE_LIGHT, sendCmd));
        }
    }

}

void RadioFrequency::onSend()
{
    if(sendQueue.length()>0)
    {
        QPair<SEND_TYPE,QByteArray> sendData = sendQueue.dequeue();
        if(serial->isOpen() && serial->isWritable())
        {
            if(sendData.first == SEND_TYPE_QUERY)
            {
                serial->write(sendData.second);
//                int sendTimes = 0;
//                while(true){
//                    queryOk = false;
//                    serial->write(sendData.second);
//                    ++sendTimes;
//                    int t = 0;
//                    while(true)
//                    {
//                        QyhSleep(20);
//                        ++t;
//                        if(queryOk)break;//发送OK了
//                        if(t>=10)break;//等待超时了[20ms*10 = 200ms]
//                    }
//                    if(queryOk)break;
//                    if(sendTimes>=4)break;//发送3次[200ms *3 = 600ms]都没有成功，那就算了
//                }
            }else if(sendData.first == SEND_TYPE_LIGHT)
            {
                serial->write(sendData.second);
//                int sendTimes = 0;
//                while(true){
//                    lightOk = false;
//                    serial->write(sendData.second);
//                    ++sendTimes;
//                    int t = 0;
//                    while(true)
//                    {
//                        QyhSleep(20);
//                        ++t;
//                        if(lightOk)break;//发送OK了
//                        if(t>=10)break;//等待超时了[20ms*10 = 200ms]
//                    }
//                    if(lightOk)break;
//                    if(sendTimes>=4)break;//发送3次都没有成功，那就算了
//                }
            }
        }
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
        const unsigned char str[] = {RADOI_FREQUENCY_ADDRESS_A, 0x03, 0x00, 0x08, 0x00, 0x01,};
        query = QByteArray(reinterpret_cast<const char*>(&str[0]),std::extent<decltype(str)>::value);
    }else{
        //B的地址是0x08
        const unsigned char str[] = {RADOI_FREQUENCY_ADDRESS_B, 0x03, 0x00, 0x08, 0x00, 0x01,};
        query = QByteArray(reinterpret_cast<const char*>(&str[0]),std::extent<decltype(str)>::value);
    }

    qint16 crc = checksum(query);

    query += (char )(crc>>8 & 0xff);
    query += (char )(crc & 0xff);

    sendQueue.enqueue(qMakePair(SEND_TYPE_QUERY,query));
}
