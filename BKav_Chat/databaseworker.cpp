#include "databaseworker.h"
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "SecurityUtils.h"

DatabaseWorker::DatabaseWorker(QObject *parent) : QObject(parent) {}

DatabaseWorker::~DatabaseWorker() {
    if (db.isOpen()) db.close();
    QString name = m_connectionName;
    db = QSqlDatabase();
    if (!name.isEmpty())
        QSqlDatabase::removeDatabase(name);
}

void DatabaseWorker::init(qint64 myId) {
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(basePath + "/cache");
    if (!dir.exists()) dir.mkpath(".");

    QString dbFileName = QString("cache_%1.db").arg(myId);
    QString dbPath = dir.filePath(dbFileName);
    m_connectionName = QString("conn_%1").arg(myId);

    if (QSqlDatabase::contains(m_connectionName)) {
        db = QSqlDatabase::database(m_connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        db.setDatabaseName(dbPath);
    }

    if (!db.open()) {
        qDebug() << "Không thể mở database:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.exec("PRAGMA journal_mode=WAL");

    bool success = query.exec("CREATE TABLE IF NOT EXISTS messages ("
                              "id TEXT PRIMARY KEY, "
                              "sender_id INTEGER, "
                              "friend_id INTEGER, "
                              "content TEXT, "
                              "files_json TEXT, "
                              "images_json TEXT, "
                              "created_at TEXT, "
                              "is_send INTEGER DEFAULT 1)");
    if (!success) qDebug() << "Lỗi tạo bảng:" << query.lastError().text();

    query.exec("CREATE INDEX IF NOT EXISTS idx_participants ON messages(sender_id, friend_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_created_at ON messages(created_at)");
    query.exec("DELETE FROM messages WHERE created_at < datetime('now', '-3 days')");
}

void DatabaseWorker::setEncryptionKey(const QString &key) {
    if (key.isEmpty()) {
        qWarning() << "Cảnh báo: Khóa mã hóa đang bị trống!";
        return;
    }
    m_currentKey = key;
}

void DatabaseWorker::insertMessage(const QString& id, qint64 senderId, qint64 friendId,
                                   const QString& content, const QJsonArray& files,
                                   const QJsonArray& images, const QString& createdAt, int isSend) {
    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO messages (id, sender_id, friend_id, content, files_json, images_json, created_at, is_send) "
                  "VALUES (:id, :s_id, :f_id, :content, :files, :images, :time, :is_send)");

    QString filesStr = QJsonDocument(files).toJson(QJsonDocument::Compact);
    QString imagesStr = QJsonDocument(images).toJson(QJsonDocument::Compact);

    QString encContent = SecurityUtils::process(content, m_currentKey);
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

    if (!query.exec())
        qDebug() << "Lỗi lưu tin nhắn ở cache:" << query.lastError().text();
}

void DatabaseWorker::getMessages(qint64 myId, qint64 friendId, quint64 requestId) {
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
    emit messagesReady(requestId, messages);
}

void DatabaseWorker::updateMessageStatus(const QString& id, int isSend) {
    QSqlQuery query(db);
    query.prepare("UPDATE messages SET is_send = :is_send WHERE id = :id AND is_send < :is_send");
    query.bindValue(":is_send", isSend);
    query.bindValue(":id", id);
    if (!query.exec())
        qDebug() << "Lỗi update trạng thái tin nhắn:" << query.lastError().text();
}

void DatabaseWorker::markAllSeenFromFriend(qint64 friendId, qint64 myId) {
    QSqlQuery query(db);
    query.prepare("UPDATE messages SET is_send = 2 "
                  "WHERE sender_id = :my AND friend_id = :friend AND is_send < 2");
    query.bindValue(":my", myId);
    query.bindValue(":friend", friendId);
    if (!query.exec())
        qDebug() << "Lỗi update seen hàng loạt:" << query.lastError().text();
}