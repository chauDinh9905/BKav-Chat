#include "DatabaseManager.h"
#include "databaseworker.h"

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {
    qRegisterMetaType<QVector<QVariantMap>>("QVector<QVariantMap>");

    m_worker = new DatabaseWorker();
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &DatabaseWorker::messagesReady, this, &DatabaseManager::messagesReady);
    m_workerThread.start();
}

DatabaseManager::~DatabaseManager() {
    m_workerThread.quit();
    m_workerThread.wait();
}

void DatabaseManager::init(qint64 myId) {
    QMetaObject::invokeMethod(m_worker, [this, myId]() {
        m_worker->init(myId);
    }, Qt::QueuedConnection);
}

void DatabaseManager::setEncryptionKey(const QString &key) {
    QMetaObject::invokeMethod(m_worker, [this, key]() {
        m_worker->setEncryptionKey(key);
    }, Qt::QueuedConnection);
}

void DatabaseManager::insertMessage(const QString& id, qint64 senderId, qint64 friendId,
                                    const QString& content, const QJsonArray& files,
                                    const QJsonArray& images, const QString& createdAt, int isSend) {
    QMetaObject::invokeMethod(m_worker, [this, id, senderId, friendId, content, files, images, createdAt, isSend]() {
        m_worker->insertMessage(id, senderId, friendId, content, files, images, createdAt, isSend);
    }, Qt::QueuedConnection);
}

void DatabaseManager::updateMessageStatus(const QString& id, int isSend) {
    QMetaObject::invokeMethod(m_worker, [this, id, isSend]() {
        m_worker->updateMessageStatus(id, isSend);
    }, Qt::QueuedConnection);
}

void DatabaseManager::markAllSeenFromFriend(qint64 friendId, qint64 myId) {
    QMetaObject::invokeMethod(m_worker, [this, friendId, myId]() {
        m_worker->markAllSeenFromFriend(friendId, myId);
    }, Qt::QueuedConnection);
}

quint64 DatabaseManager::requestMessages(qint64 myId, qint64 friendId) {
    quint64 reqId = ++m_requestCounter;
    QMetaObject::invokeMethod(m_worker, [this, myId, friendId, reqId]() {
        m_worker->getMessages(myId, friendId, reqId);
    }, Qt::QueuedConnection);
    return reqId;
}