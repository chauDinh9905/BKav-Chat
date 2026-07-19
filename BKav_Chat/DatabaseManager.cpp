#include "DatabaseManager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include "SecurityUtils.h"
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>

void DatabaseManager::init(qint64 userId) {
    QMutexLocker locker(&mutex);

    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(basePath + "/cache");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString dbFileName = QString("cache_%1.db").arg(userId);
    QString dbPath = dir.filePath(dbFileName);
    qDebug() << "DB path:" << dbPath;
    QString connectionName = QString("conn_%1").arg(userId);

    //  Kết nối tới SQLite riêng cho từng userId (nếu chưa kết nối)
    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(dbPath);
    }

    //  Mở database
    if (!db.open()) {
        qDebug() << "Không thể mở database:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);                                  //  truyền db, không để rơi về default connection
    query.exec("PRAGMA journal_mode=WAL");                 // tiện áp WAL luôn, khớp hướng active_sessions

    // Tạo bảng tin nhắn (Chỉ tạo nếu chưa tồn tại)
    bool success = query.exec("CREATE TABLE IF NOT EXISTS messages ("
                              "id TEXT PRIMARY KEY, "
                              "sender_id INTEGER, "
                              "friend_id INTEGER, "
                              "content TEXT, "
                              "files_json TEXT, "
                              "images_json TEXT, "
                              "created_at TEXT, "
                              "is_send INTEGER DEFAULT 1)");
    if (!success) {
        qDebug() << "Lỗi tạo bảng:" << query.lastError().text();
    }
    query.exec("CREATE INDEX IF NOT EXISTS idx_participants ON messages(sender_id, friend_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_created_at ON messages(created_at)");

    // Dọn cache cũ quá 3 ngày, tránh phình vô hạn
    query.exec("DELETE FROM messages WHERE created_at < datetime('now', '-3 days')");
}

void DatabaseManager::insertMessage(const QString& id, qint64 senderId, qint64 friendId,
                                    const QString& content, const QJsonArray& files,
                                    const QJsonArray& images, const QString& createdAt, int isSend) {
    QMutexLocker locker(&mutex);
    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO messages (id, sender_id, friend_id, content, files_json, images_json, created_at, is_send) "
                  "VALUES (:id, :s_id, :f_id, :content, :files, :images, :time, :is_send)");

    // Chuyển QJsonArray sang chuỗi JSON string
    QString filesStr = QJsonDocument(files).toJson(QJsonDocument::Compact);
    QString imagesStr = QJsonDocument(images).toJson(QJsonDocument::Compact);
    QString contentStr = content;

    QString encContent = SecurityUtils::process(contentStr, m_currentKey);
    QString encFiles   = SecurityUtils::process(filesStr, m_currentKey);
    QString encImages  = SecurityUtils::process(imagesStr, m_currentKey);

    query.bindValue(":content", encContent);
    query.bindValue(":id", id);
    query.bindValue(":s_id", senderId);
    query.bindValue(":f_id", friendId);
    query.bindValue(":files", encFiles);
    query.bindValue(":images", encImages);
    query.bindValue(":time", createdAt);
    query.bindValue(":is_send", isSend);

    if (!query.exec()) {
        qDebug() << "Lỗi lưu tin nhắn ở cache:" << query.lastError().text();
    }
}

QVector<QVariantMap> DatabaseManager::getMessages(qint64 myId, qint64 friendId) {
    QMutexLocker locker(&mutex);
    QVector<QVariantMap> messages;
    QSqlQuery query(db);

    query.prepare("SELECT * FROM messages WHERE sender_id = :my AND friend_id = :friend "
                  "UNION ALL "
                  "SELECT * FROM messages WHERE sender_id = :friend AND friend_id = :my "
                  "ORDER BY created_at ASC");
    query.bindValue(":my", myId);
    query.bindValue(":friend", friendId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap msg;
            QString rawContent = query.value("content").toString();
            QString rawFiles   = query.value("files_json").toString();
            QString rawImages  = query.value("images_json").toString();

            QString decContent = SecurityUtils::process(rawContent, m_currentKey);
            QString decFiles   = SecurityUtils::process(rawFiles, m_currentKey);
            QString decImages  = SecurityUtils::process(rawImages, m_currentKey);

            msg["content"] = decContent;
            msg["files"]   = QJsonDocument::fromJson(decFiles.toUtf8()).array();
            msg["images"]  = QJsonDocument::fromJson(decImages.toUtf8()).array();
            msg["id"] = query.value("id");
            msg["sender_id"] = query.value("sender_id");
            msg["friend_id"] = query.value("friend_id");
            msg["created_at"]= query.value("created_at");
            msg["is_send"] = query.value("is_send");
            messages.append(msg);
        }
    }
    return messages;
}
void DatabaseManager::updateMessageStatus(const QString& id, int isSend)
{
    QMutexLocker locker(&mutex);
    QSqlQuery query(db);
    // chỉ tăng dần, không cho giảm - tránh sự kiện đến trễ/trùng làm sai lệch trạng thái
    query.prepare("UPDATE messages SET is_send = :is_send WHERE id = :id AND is_send < :is_send");
    query.bindValue(":is_send", isSend);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Lỗi update trạng thái tin nhắn:" << query.lastError().text();
    }
}
void DatabaseManager::markAllSeenFromFriend(qint64 friendId, qint64 myId)
{
    QMutexLocker locker(&mutex);
    QSqlQuery query(db);
    query.prepare("UPDATE messages SET is_send = 2 "
                  "WHERE sender_id = :my AND friend_id = :friend AND is_send < 2");
    query.bindValue(":my", myId);
    query.bindValue(":friend", friendId);

    if (!query.exec()) {
        qDebug() << "Lỗi update seen hàng loạt:" << query.lastError().text();
    }
}