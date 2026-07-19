#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>

class DatabaseManager {
public:
    // Hàm này giúp lấy instance duy nhất của DatabaseManager
    static DatabaseManager& instance() {
        static DatabaseManager instance;
        return instance;
    }

    void init(qint64 myId); // Hàm khởi tạo database và tạo bảng
    void insertMessage(const QString& id, qint64 senderId, qint64 friendId,
                    const QString& content, const QJsonArray& files,
                        const QJsonArray& images, const QString& createdAt, int isSend = 1);
    QVector<QVariantMap> getMessages(qint64 myId, qint64 friendId);
     void updateMessageStatus(const QString& id, int isSend);
    void markAllSeenFromFriend(qint64 friendId, qint64 myId);
    void setEncryptionKey(QString key){
        if (key.isEmpty()) {
            qWarning() << "Cảnh báo: Khóa mã hóa đang bị trống!";
            return;
        }
        m_currentKey = key;
    }
private:
    DatabaseManager() {} // Private để đảm bảo singleton
    QSqlDatabase db;
    QString m_currentKey;
    QMutex mutex;
};

#endif // DATABASEMANAGER_H