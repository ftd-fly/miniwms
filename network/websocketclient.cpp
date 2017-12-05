#include "websocketclient.h"

WebsocketClient::WebsocketClient(const QUrl &url, QObject *parent) : QObject(parent),
    m_url(url)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &WebsocketClient::sig_connect);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WebsocketClient::sig_disconnect);
    connect(&m_webSocket, &QWebSocket::error, this, &WebsocketClient::sig_error);
    connect(&m_webSocket,&QWebSocket::binaryMessageReceived,this,&WebsocketClient::sig_recv);
    m_webSocket.open(QUrl(url));
}

//发送数据到服务器
bool WebsocketClient::sendToServer(QByteArray qba)
{
    if(isConnect()){
        return m_webSocket.sendBinaryMessage(qba) == qba.length();
    }
    return false;
}

//是否已连接服务器
bool WebsocketClient::isConnect()
{
    if(!m_webSocket.isValid())return false;
    return m_webSocket.state() == QAbstractSocket::ConnectedState;
}

//主动关闭连接
void WebsocketClient::close()
{
    m_webSocket.close();
}

//主动关闭连接后，主动连接
void WebsocketClient::open()
{
    m_webSocket.open(m_url);
}

//重置连接地址
void WebsocketClient::setUrl(const QUrl &url)
{
    m_url = url;
}
