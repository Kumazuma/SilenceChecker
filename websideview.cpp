#include "websideview.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrent>
WebSideView::WebSideView(std::shared_ptr<Presenter> presenter, QObject *parent) :
    QObject(parent),
    m_presenter(std::move(presenter)),
    m_server("websideview", QWebSocketServer::SslMode::NonSecureMode)
{
    if(m_server.listen(QHostAddress::LocalHost, 3010))
    {
        connect(&m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    }
    connect(m_presenter.get(), &Presenter::runningState, this, &WebSideView::onChangeRunningState);
    connect(m_presenter.get(), &Presenter::tickTime,this, &WebSideView::onTick);
    connect(m_presenter.get(), &Presenter::overTimeout,this, &WebSideView::onOverTimeout);
    connect(m_presenter.get(), &Presenter::changeTimeout,this, &WebSideView::onChangedTimeout);
}

WebSideView::~WebSideView()
{
    for(auto& item: m_clients)
    {
        QWebSocket* socket = item.second;
        socket->close();
        socket->deleteLater();
    }
    m_clients.clear();
    m_server.close();
}

void WebSideView::onNewConnection()
{
    auto* client = m_server.nextPendingConnection();
    connect(client, SIGNAL(textMessageReceived(QString)), this, SLOT(onStringMessage(QString)));
    connect(client, SIGNAL(disconnected()), this, SLOT(onDisconnect()));

    auto peerPort=  client->peerPort();
    m_clients.insert(std::pair(peerPort,client));
    QJsonObject obj;
    obj.insert("type", QJsonValue("init"));
    obj.insert("state", QJsonValue(m_presenter->isRunning()));
    obj.insert("timeout", QJsonValue(m_presenter->timeout()/(60* 1000)));
    obj.insert("timeoutCount", QJsonValue(m_presenter->outCount()));
    QJsonDocument document(obj);
    QString jsonText = document.toJson();
    client->sendTextMessage(jsonText);

}

void WebSideView::onDisconnect()
{
    auto socket = qobject_cast<QWebSocket*>(sender());
    if(socket == nullptr)
        return;
    auto node = m_clients.find(socket->peerPort());
    if(node != m_clients.end())
    {
        m_clients.erase(node);
    }
    disconnect(socket);
    socket->deleteLater();
}

void WebSideView::onStringMessage(QString)
{

}

void WebSideView::onTick(int ms)
{
    QJsonObject obj;
    obj.insert("type", QJsonValue("tick"));
    obj.insert("tick", QJsonValue(ms));
    QJsonDocument document(obj);
    QString jsonText = document.toJson();
    for(auto& item: m_clients)
    {
        QWebSocket* socket = item.second;
        socket->sendTextMessage(jsonText);
    }
}

void WebSideView::onChangeRunningState(bool state)
{
    QJsonObject obj;
    obj.insert("type", QJsonValue("runningState"));
    obj.insert("state", QJsonValue(state));
    QJsonDocument document(obj);
    QString jsonText = document.toJson();
    for(auto& item: m_clients)
    {
        QWebSocket* socket = item.second;
        socket->sendTextMessage(jsonText);
    }
}

void WebSideView::onOverTimeout(int count)
{
    QJsonObject obj;
    obj.insert("type", QJsonValue("timeout"));
    obj.insert("count", QJsonValue(count));
    QJsonDocument document(obj);
    QString jsonText = document.toJson();
    for(auto& item: m_clients)
    {
        QWebSocket* socket = item.second;
        socket->sendTextMessage(jsonText);
    }
}

void WebSideView::onChangedTimeout(int ms)
{
    QJsonObject obj;
    obj.insert("type", QJsonValue("timeoutChanged"));
    obj.insert("time", QJsonValue(ms /(60* 1000)));
    QJsonDocument document(obj);
    QString jsonText = document.toJson();
    for(auto& item: m_clients)
    {
        QWebSocket* socket = item.second;
        socket->sendTextMessage(jsonText);
    }
}
