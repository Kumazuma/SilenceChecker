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
    obj.insert("type", QJsonValue("runningState"));
    obj.insert("state", QJsonValue(m_presenter->isRunning()));
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

void WebSideView::onTick(int sec)
{
    QJsonObject obj;
    obj.insert("type", QJsonValue("tick"));
    obj.insert("tick", QJsonValue(sec));
    obj.insert("outCount", QJsonValue(m_presenter->outCount()));
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
