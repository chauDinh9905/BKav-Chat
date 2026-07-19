#include "SocketManager.h"
#include "appconfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>

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
void SocketManager::unregisterUser(qint64 userId)
{
    QJsonObject obj;
    obj["type"] = "unregister";
    obj["userId"] = userId;
    socket.sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    QTimer::singleShot(150, &socket, [this]() {socket.close();});
    socket.close();
}
void SocketManager::markSeen(qint64 userId, qint64 friendId){
    QJsonObject obj;
    obj["type"] = "mark_seen";
    obj["userId"] = userId;
    obj["friendId"] = friendId;
    socket.sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}