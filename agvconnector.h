#ifndef AGVCONNECTOR_H
#define AGVCONNECTOR_H

#include <QObject>
#include <QTimer>
#include "network/websocketclient.h"

class AgvConnector : public QObject
{
    Q_OBJECT
public:

    enum{
        UNKNOWN = -99,
        ERROR = -1,//     错误
        IDLING = 0,//     空闲
        RUNNING = 1,//    正在运行
        PAUSED = 2, //     暂停
        COMPLETED = 3,  //   完成
        CANCELLED = 4,  //  取消
    };

    explicit AgvConnector(QObject *parent = nullptr);
    void init(int _id, QString _ip, int _port);
    bool isconnect();
    bool isinit();
    bool goOrigin();
    bool sendTask(int _taskId,int line,int station,bool _continue = false);
    bool cancelTask(int line, int station);
    bool isIdle();
    int getId(){return id;}
signals:
    void error();//发生错误
    void finish(int _taskId);//完成了一个任务
    void cancel(int _taskId);//取消了一个任务
public slots:
    void onRecv(QString str);
    void reconnect();
    void onConnect();
    bool queryStatus();
private:
    void processOneJson(QString json);
    WebsocketClient *client;
    QUrl url;
    bool hasInit;
    int status;
    int taskId;//正在执行的任务的类型

    int id;
    QString ip;
    int port;
};

#endif // AGVCONNECTOR_H
