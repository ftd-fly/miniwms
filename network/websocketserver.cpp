#include "websocketserver.h"
#include <QWebSocketServer>
#include <QWebSocket>
#include <QDebug>

WebsocketServer::WebsocketServer(quint16 port, QString name, QObject *parent) : QObject(parent),
    m_pWebSocketServer(NULL)
{
    m_pWebSocketServer = new QWebSocketServer(name,
                                              QWebSocketServer::NonSecureMode,
                                              this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, port))
    {
        qDebug() << "websocket Server listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &WebsocketServer::onNewConnection);
    }
}
WebsocketServer::~WebsocketServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

//发送给一个人
bool WebsocketServer::sendToOne(QWebSocket *to,QByteArray data)
{
    return to->sendBinaryMessage(data) == data.length();
}

//发送给所有人
bool WebsocketServer::sendToAll(QByteArray data)
{
    for (QWebSocket *pClient : qAsConst(m_clients)) {
        pClient->sendBinaryMessage(data);
    }
}

//发送给所有人,except me
bool WebsocketServer::sendToAllButMe(QWebSocket *from,QByteArray data)
{
    for (QWebSocket *pClient : qAsConst(m_clients)) {
        if (pClient != from) //don't echo message back to sender
        {
            pClient->sendBinaryMessage(data);
        }
    }
}

void WebsocketServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebsocketServer::processMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WebsocketServer::socketDisconnected);

    m_clients << pSocket;
}

void WebsocketServer::processMessage(const QByteArray &message)
{
    QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());
    //不需要在判断是否在list当中。
    //TODO:
    onRecv(pSender,message);
}

void WebsocketServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
