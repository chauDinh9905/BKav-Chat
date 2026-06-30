#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QObject>
#include <QWebSocket>

class SocketManager : public QObject
{
    Q_OBJECT

public:
    static SocketManager& instance();

    void connectToServer();

    void sendMessage(qint64 from,qint64 to,const QString &content);
    void registerUser(qint64 userId);
    void unregisterUser(qint64 userId);

signals:
    void messageReceived(const QString &message);
    void connected();
private slots:
    void onTextMessageReceived(const QString &message);
    void onConnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
private:
    explicit SocketManager(QObject *parent = nullptr);
    QWebSocket socket;
};

#endif