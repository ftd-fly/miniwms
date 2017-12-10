#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QList>
#include <QByteArray>

class QWebSocketServer;
class QWebSocket;

//这里简便起见，只接受和发送二进制消息
class WebsocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebsocketServer(quint16 port,QString name,QObject *parent = nullptr);

    virtual ~WebsocketServer();

    //发送给一个人
    bool sendToOne(QWebSocket *to,QByteArray data);

    //发送给所有人
    bool sendToAll(QByteArray data);

    //发送给所有人,except me
    bool sendToAllButMe(QWebSocket *from,QByteArray data);

    //继承请实现
    //不采用信号槽是只有这一个信号，还是用 虚函数效率高[又或者直接修改代码拿去实现]
    virtual void onRecv(QWebSocket *from,QByteArray msg) = 0;

signals:

public slots:
    void onNewConnection();
    void processMessage(const QByteArray &message);
    void socketDisconnected();
private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
};

#endif // WEBSOCKETSERVER_H
