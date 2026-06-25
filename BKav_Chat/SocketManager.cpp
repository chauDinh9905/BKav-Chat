#include "SocketManager.h"
#include "appconfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

SocketManager::SocketManager(QObject *parent)
    : QObject(parent)
{
    connect(
        &socket,
        &QWebSocket::textMessageReceived,
        this,
        &SocketManager::onTextMessageReceived);
    connect(&socket, &QWebSocket::connected,
            this, &SocketManager::onConnected);
}

SocketManager&
SocketManager::instance()
{
    static SocketManager manager;
    return manager;
}

void SocketManager::connectToServer()
{
    QString ip = AppConfig::instance().getServerIp();
    QString port = AppConfig::instance().getServerPort();
    QUrl socketUrl(QString("ws://%1:%2/").arg(ip, port));
    connect(&socket, &QWebSocket::errorOccurred, this, &SocketManager::onErrorOccurred, Qt::UniqueConnection);
    socket.open(socketUrl);
}
void SocketManager::onErrorOccurred(QAbstractSocket::SocketError error)
{
    qDebug() << " [Socket] WS error:" << error;
}
void SocketManager::sendMessage(
    qint64 from,
     qint64 to,
    const QString &content)
{
    QJsonObject obj;

    obj["from"] = from;
    obj["to"] = to;
    obj["content"] = content;

    socket.sendTextMessage(
        QJsonDocument(obj)
            .toJson(
                QJsonDocument::Compact));
}

void SocketManager::onTextMessageReceived(
    const QString &message)
{
    emit messageReceived(message);
}
void SocketManager::onConnected()
{
    emit connected();
}

void SocketManager::registerUser(qint64 userId)
{
    QJsonObject obj;
    obj["type"] = "register";
    obj["userId"] = userId;
    socket.sendTextMessage(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));
}