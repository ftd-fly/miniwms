#include "agvconnector.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
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
    qDebug() << "recv:"<<str;
    processOneJson(str);
}

void AgvConnector::reconnect()
{
    hasInit = false;
    client->setUrl(url);
    client->open();
}

void AgvConnector::onConnect()
{
    //连上以后    //发出初始化指令
    QVariantMap initcmd;
    initcmd.insert("op", "advertise");
    initcmd.insert("topic", "/nav_ctrl");
    initcmd.insert("type", "yocs_msgs/NavigationControl");

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(initcmd);
    if (!jsonDocument.isNull()) {
        qDebug() << jsonDocument.toJson();
        client->sendToServer(QString(jsonDocument.toJson()));
    }
    QyhSleep(1000);

    //发出状态订阅
    queryStatus();
}

void AgvConnector::processOneJson(QString json)
{
    static int lastStatus = UNKNOWN;
    if(json.startsWith("\""))json = json.right(json.length()-1);
    if(json.endsWith("\""))json = json.left(json.length()-1);
    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toLocal8Bit(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug()<<"parse json error:" << parseError.errorString();
        return ;
    }

    QVariantMap result = jsonDocument.toVariant().toMap();
    if(result["op"].toString() == "unadvertise" && result["topic"].toString() == "/nav_ctrl")
    {
        hasInit = true;
    }

    QMap<QString, QVariant> mm = result["msg"].toMap();

    if(result["msg"].toMap()["status"].toString().length()>0){
        status = result["msg"].toMap()["status"].toInt();
        if(status == COMPLETED && lastStatus != status){
            emit finish(taskId);
        }else if(status == ERROR && lastStatus != status){
            emit error();
        }else if(status == IDLING && lastStatus != status){
            emit cancel(taskId);
        }
        lastStatus = status;
    }
}

//从那条线上，运送到那个站点
bool AgvConnector::sendTask(int _taskId, int line, int station)
{
    QString goal_name;
    if(line== Task::LineA){
        goal_name = QString("cache_A_%1_%2").arg(1+station/column).arg(1+station%column);
    }else{
        goal_name = QString("cache_B_%1_%2").arg(1+station/column-rowA).arg(1+station%column);
    }
    //QString goal_name = QString("Line_%1ToCache_%2").arg(line).arg(station);

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
    taskId = _taskId;
    //等待返回值？//协议中未给出返回值！直接认为是成功的
    return true;
}

//从那条线上，运送到那个站点
bool AgvConnector::cancelTask(int line, int station)
{
    QString goal_name;
    if(line== Task::LineA){
        goal_name = QString("cache_A_%1_%2").arg(1+station/column).arg(1+station%column);
    }else{
        goal_name = QString("cache_B_%1_%2").arg(1+station/column-rowA).arg(1+station%column);
    }
    //QString goal_name = QString("Line_%1ToCache_%2").arg(line).arg(station);

    QVariantMap msg;
    msg.insert("goal_name", goal_name);
    msg.insert("control", 0);
    QJsonDocument msgJson = QJsonDocument::fromVariant(msg);
    qDebug() << msgJson.toJson();

    QVariantMap pub;
    pub.insert("op", "publish");
    pub.insert("topic", "/nav_ctrl");
    pub.insert("msg", msgJson.toVariant());
    QJsonDocument pubJson = QJsonDocument::fromVariant(pub);
    qDebug() << pubJson.toJson();
    if(!client->sendToServer(QString(pubJson.toJson())))return false;
    //等待返回值？//协议中未给出返回值！直接认为是成功的
    return true;
}

bool AgvConnector::queryStatus()
{
    QVariantMap query;
    query.insert("op", "subscribe");
    query.insert("topic", "/nav_ctrl_status");
    query.insert("type", "yocs_msgs/NavigationControlStatus");
    QJsonDocument queryJson = QJsonDocument::fromVariant(query);
    status = UNKNOWN;
    qDebug() << "send:"<<QString(queryJson.toJson());
    if(!client->sendToServer(QString(queryJson.toJson())))return false;

    return true;
}

bool AgvConnector::isIdle()
{
    return status == IDLING;
}
