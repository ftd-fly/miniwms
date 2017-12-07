#include "agvconnector.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include "global.h"

AgvConnector::AgvConnector(QObject *parent) : QObject(parent),client(NULL),hasInit(false)
{
}

void AgvConnector::init(int _id,QString _ip,int _port)
{
    id =_id;
    ip = _ip;
    port = _port;
    QString _url = QString("ws://%1:%2").arg(ip).arg(port);
    url = QUrl(_url);
    if(client)delete client;
    client = new WebsocketClient(url);
    connect(client,SIGNAL(sig_recv(QString)),this,SLOT(onRecv(QString)));
    connect(client,SIGNAL(sig_disconnect()),this,SLOT(reconnect()));
    connect(client,SIGNAL(sig_connect()),this,SLOT(onConnect()));
}

bool AgvConnector::isconnect()
{
    if(!client)return false;
    return client->isConnect();
}
bool AgvConnector::isinit()
{
    if(!client)return false;
    if(!client->isConnect())return false;
    return hasInit;
}
void AgvConnector::onRecv(QString str)
{
    static QString buff;
    buff+=str;
    int start,end;
    start = buff.indexOf("{");
    end=buff.indexOf("}",start);
    if(start==-1||end==-1)return;
    QString oneJson = buff.mid(start,end-start+1);
    buff = buff.right(buff.length()-end-1);
    //对其进行解析
    processOneJson(oneJson);
}

void AgvConnector::reconnect()
{
    hasInit = false;
    client->setUrl(url);
    client->open();
}

void AgvConnector::onConnect()
{
    //连上以后
    //发出初始化指令
    QVariantMap initcmd;
    initcmd.insert("op", "advertise");
    initcmd.insert("topic", "/nav_ctrl");
    initcmd.insert("type", "yocs_msgs/NavigationControl");

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(initcmd);
    if (!jsonDocument.isNull()) {
        qDebug() << jsonDocument.toJson();
        client->sendToServer(QString(jsonDocument.toJson()));
    }
    queryStatusTimer.setInterval(2000);
    connect(&queryStatusTimer,SIGNAL(timeout()),this,SLOT(queryStatus()));
}

void AgvConnector::processOneJson(QString json)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toLatin1(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug()<<"parse json error:" << error.errorString();
        return ;
    }

    QVariantMap result = jsonDocument.toVariant().toMap();
    if(result["op"].toString() == "unadvertise" && result["topic"].toString() == "/nav_ctrl")
    {
        hasInit = true;
    }
    if(result["waypoint_name"].toString().length()>0&&result["status_desc"].toString().length()>0){
        status = result["status_desc"].toInt();
    }
}

//从那条线上，运送到那个站点
bool AgvConnector::sendTask(int line,int station)
{
    QString goal_name = QString("Line_%1ToCache_%2").arg(line).arg(station);

    QVariantMap msg;
    msg.insert("goal_name", goal_name);
    msg.insert("control", 1);
    QJsonDocument msgJson = QJsonDocument::fromVariant(msg);
    qDebug() << msgJson.toJson();

    QVariantMap pub;
    pub.insert("op", "publish");
    pub.insert("topic", "/nav_ctrl");
    pub.insert("msg", msgJson.toVariant());
    QJsonDocument pubJson = QJsonDocument::fromVariant(pub);
    qDebug() << pubJson.toJson();
    if(!client->sendToServer(QString(pubJson.toJson())))return false;
    taskType = line;
    //等待返回值？//协议中未给出返回值！
    return true;
}

int AgvConnector::queryStatus()
{
    static int lastStatus = UNKNOWN;
    QVariantMap query;
    query.insert("op", "subscribe");
    query.insert("topic", "/nav_ctrl_status");
    query.insert("type", "yocs_msgs/NavigationControlStatus");
    QJsonDocument queryJson = QJsonDocument::fromVariant(query);
    status = UNKNOWN;
    if(!client->sendToServer(QString(queryJson.toJson())))return false;
    //等待返回值
    int kk = 10;//超时
    while(status == UNKNOWN && --kk > 0)QyhSleep(100);

    if(status == COMPLETED && lastStatus != status){
        emit finish(taskType);
    }else if(status == ERROR && lastStatus != status){
        emit error();
    }

    lastStatus = status;
    return status;
}

bool AgvConnector::isIdle()
{
    return queryStatus() == IDLING;
}
