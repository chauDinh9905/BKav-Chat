#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include <QVariantMap>
#include <QJsonArray>

class DatabaseWorker : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseWorker(QObject *parent = nullptr);
    ~DatabaseWorker() override;

    void init(qint64 myId);
    void setEncryptionKey(const QString &key);
    void insertMessage(const QString& id, qint64 senderId, qint64 friendId,
                       const QString& content, const QJsonArray& files,
                       const QJsonArray& images, const QString& createdAt, int isSend);
    void getMessages(qint64 myId, qint64 friendId, quint64 requestId);
    void updateMessageStatus(const QString& id, int isSend);
    void markAllSeenFromFriend(qint64 friendId, qint64 myId);

signals:
    void messagesReady(quint64 requestId, QVector<QVariantMap> messages);

private:
    QSqlDatabase db;
    QString m_currentKey;
    QString m_connectionName;
};

#endif // DATABASEWORKER_H