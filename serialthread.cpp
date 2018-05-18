#include "serialthread.h"
#include "global.h"
#include "crc16.h"
SerialThread::SerialThread(QObject *parent) :
    QThread(parent)
{
    this->moveToThread(this);
}

void SerialThread::run(){

    portName = configure.getValue("RadioFrequency/COM").toString();

    //奇偶校验
    int p = configure.getValue("RadioFrequency/Parity").toInt();
    parity = QSerialPort::UnknownParity;
    if(p==0)parity = QSerialPort::NoParity;
    if(p==2)parity = QSerialPort::EvenParity;
    if(p==3)parity = QSerialPort::OddParity;
    if(p==4)parity = QSerialPort::SpaceParity;
    if(p==5)parity = QSerialPort::MarkParity;

    baudrate = configure.getValue("RadioFrequency/BaudRate").toInt();

    //数据位
    int d = configure.getValue("RadioFrequency/DataBits").toInt();
    dataBits = QSerialPort::UnknownDataBits;
    if(d==5)dataBits = QSerialPort::Data5;
    if(d==6)dataBits = QSerialPort::Data6;
    if(d==7)dataBits = QSerialPort::Data7;
    if(d==8)dataBits = QSerialPort::Data8;

    int s = configure.getValue("RadioFrequency/StopBits").toInt();
    stopBits = QSerialPort::UnknownStopBits;
    if(s==1)stopBits = QSerialPort::OneStop;
    if(s==2)stopBits = QSerialPort::TwoStop;
    if(s==3)stopBits = QSerialPort::OneAndHalfStop;

    address_on_off[RADOI_FREQUENCY_ADDRESS_A] = false;
    address_on_off[RADOI_FREQUENCY_ADDRESS_B] = false;
    address_on_off[RADOI_FREQUENCY_ADDRESS_C] = false;
    address_on_off[RADOI_FREQUENCY_ADDRESS_D] = false;

    queryTimer = new QTimer();
    queryTimer->setInterval(150);
    connect(queryTimer,&QTimer::timeout,this,&SerialThread::onQueryTimer);

    sendTimer = new QTimer();
    sendTimer->setInterval(100);
    connect(sendTimer,&QTimer::timeout,this,&SerialThread::onSendTimer);

    lightTimer = new QTimer();
    lightTimer->setInterval(200);
    connect(lightTimer,&QTimer::timeout,this,&SerialThread::onLightTimer);


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

    sendTimer->start();
    queryTimer->start();
    lightTimer->start();
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
    static QByteArray buffer;
    static int APushTime = 0;//计数 A按下的时间
    static int BPushTime = 0;//计数 A按下的时间
    buffer += _serialPort->readAll();
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

void SerialThread::lightOn(int address)
{
    address_on_off[address] = true;
//    qDebug()<<"address="<<address<<" light on";
    address_on_off[RADOI_FREQUENCY_ADDRESS_C] = true;
    address_on_off[RADOI_FREQUENCY_ADDRESS_D] = true;
//    qDebug()<<"a="<<address_on_off[RADOI_FREQUENCY_ADDRESS_A]
//              <<"b="<<address_on_off[RADOI_FREQUENCY_ADDRESS_B]
//                <<"c="<<address_on_off[RADOI_FREQUENCY_ADDRESS_C]
//                  <<"d="<<address_on_off[RADOI_FREQUENCY_ADDRESS_D];
}

void SerialThread::lightOff(int address)
{
    address_on_off[address] = false;
//    qDebug()<<"address="<<address<<" light off";

    if(!address_on_off[RADOI_FREQUENCY_ADDRESS_A] && !address_on_off[RADOI_FREQUENCY_ADDRESS_B]){
        address_on_off[RADOI_FREQUENCY_ADDRESS_C] = false;
        address_on_off[RADOI_FREQUENCY_ADDRESS_D] = false;
    }
//    qDebug()<<"a="<<address_on_off[RADOI_FREQUENCY_ADDRESS_A]
//              <<"b="<<address_on_off[RADOI_FREQUENCY_ADDRESS_B]
//                <<"c="<<address_on_off[RADOI_FREQUENCY_ADDRESS_C]
//                  <<"d="<<address_on_off[RADOI_FREQUENCY_ADDRESS_D];
}

void SerialThread::onLightTimer()
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
            if(sendQueue.length()>20)sendQueue.clear();
            sendQueue.enqueue(qMakePair(SEND_TYPE_LIGHT, sendCmd));
//            qDebug()<<" lignth "<<itr.key()<<" on enqueue";
        }else{
            light_off[0] = itr.key() & 0xff;
            QByteArray sendCmd(reinterpret_cast<const char*>(&light_off[0]),std::extent<decltype(light_off)>::value);
            qint16 crc = checksum(sendCmd);
            sendCmd += (char )(crc>>8 & 0xff);
            sendCmd += (char )(crc & 0xff);
            if(sendQueue.length()>20)sendQueue.clear();
            sendQueue.enqueue(qMakePair(SEND_TYPE_LIGHT, sendCmd));
//            qDebug()<<" lignth "<<itr.key()<<" off enqueue";
        }
    }

}

void SerialThread::onSendTimer()
{
    if(sendQueue.length()>0)
    {
        QPair<SEND_TYPE,QByteArray> sendData = sendQueue.dequeue();

        if(sendData.first == SEND_TYPE_QUERY)
        {
//            qDebug()<<" query dequeue";
            slot_sendData(sendData.second);
            //                serial->write(sendData.second);
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
//            qDebug()<<" lignth "<<sendData.second[0]<<" dequeue";
            slot_sendData(sendData.second);
            //serial->write(sendData.second);
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

void SerialThread::onQueryTimer()
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
    if(sendQueue.length()>20)sendQueue.clear();
    sendQueue.enqueue(qMakePair(SEND_TYPE_QUERY,query));
//    qDebug()<<" query status enqueue";
}
