#ifndef AVATARCACHE_H
#define AVATARCACHE_H
#pragma once
#include <QObject>
#include <QHash>
#include <QSet>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AvatarCache : public QObject {
    Q_OBJECT
public:
    static AvatarCache& instance();
    // Trả về pixmap nếu có, tự động kick fetch nếu chưa có
    QPixmap get(const QString &url);

signals:
    void avatarLoaded(const QString &url, const QPixmap &pixmap);

private:
    explicit AvatarCache(QObject *parent = nullptr);
    void fetch(const QString &url);

    QHash<QString, QPixmap> cache;
    QSet<QString> pending;
    QNetworkAccessManager *nam;
};
#endif // AVATARCACHE_H
