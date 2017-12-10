#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QByteArray>
#include <QAbstractSocket>


//这里简便起见，只接受和发送二进制消息
class WebsocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebsocketClient(const QUrl &url,QObject *parent = nullptr);

    //发送数据到服务器
    bool sendToServer(QString qba);

    //是否已连接服务器
    bool isConnect();

    //主动关闭连接
    void close();

    //主动关闭连接后，主动连接
    void open();

    //重置连接地址
    void setUrl(const QUrl &url);
signals:
    //断开连接
    void sig_disconnect();

    //已连接
    void sig_connect();

    //收到消息
    void sig_recv(QString);
public slots:

private:
    QWebSocket m_webSocket;
    QUrl m_url;
};

#endif // WEBSOCKETCLIENT_H
