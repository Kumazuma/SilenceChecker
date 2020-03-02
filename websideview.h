#ifndef WEBSIDEVIEW_H
#define WEBSIDEVIEW_H

#include <QObject>
#include "presenter.h"
#include <memory>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <map>
class WebSideView : public QObject
{
    Q_OBJECT
private:
    std::shared_ptr<Presenter> m_presenter;
    std::map<qint16, QWebSocket*> m_clients;
    QWebSocketServer m_server;
public:
    explicit WebSideView(std::shared_ptr<Presenter> presenter, QObject *parent = nullptr);
    ~WebSideView();
protected slots:
    void onNewConnection();
    void onDisconnect();
    void onStringMessage(QString);
    void onTick(int);
    void onChangeRunningState(bool);
    void onOverTimeout(int count);
    void onChangedTimeout(int min);
signals:

};

#endif // WEBSIDEVIEW_H
