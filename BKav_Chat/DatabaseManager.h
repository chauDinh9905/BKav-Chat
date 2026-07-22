#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QVariantMap>
#include <QJsonArray>
#include <QAtomicInteger>

class DatabaseWorker;

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    void init(qint64 myId);
    void setEncryptionKey(const QString &key);

    void insertMessage(const QString& id, qint64 senderId, qint64 friendId,
                       const QString& content, const QJsonArray& files,
                       const QJsonArray& images, const QString& createdAt, int isSend = 1);
    void updateMessageStatus(const QString& id, int isSend);
    void markAllSeenFromFriend(qint64 friendId, qint64 myId);

    quint64 requestMessages(qint64 myId, qint64 friendId);

signals:
    void messagesReady(quint64 requestId, QVector<QVariantMap> messages);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QThread m_workerThread;
    DatabaseWorker *m_worker;
    QAtomicInteger<quint64> m_requestCounter{0};
};

#endif // DATABASEMANAGER_H