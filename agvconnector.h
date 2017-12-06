#ifndef AGVCONNECTOR_H
#define AGVCONNECTOR_H

#include <QObject>
#include "network/websocketclient.h"

class AgvConnector : public QObject
{
    Q_OBJECT
public:
    explicit AgvConnector(QObject *parent = nullptr);
    void init(const QUrl &_url);
    bool isconnect();
    bool isinit();

    bool sendTask(int line,int station);

    bool queryBussy();
signals:

public slots:
    void onRecv(QString str);
    void reconnect();
    void onConnect();
private:
    void processOneJson(QString json);
    WebsocketClient *client;
    QUrl url;
    bool hasInit;

    int status;
};

#endif // AGVCONNECTOR_H
