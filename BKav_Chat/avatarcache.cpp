#include "avatarcache.h"
#include "appconfig.h"

AvatarCache& AvatarCache::instance() {
    static AvatarCache inst;
    return inst;
}

AvatarCache::AvatarCache(QObject *parent)
    : QObject(parent), nam(new QNetworkAccessManager(this)) {}

QPixmap AvatarCache::get(const QString &url) {
    if (url.isEmpty()) return QPixmap();
    if (cache.contains(url)) return cache[url];
    if (!pending.contains(url)) fetch(url);
    return QPixmap(); // trả placeholder, sẽ update sau
}

void AvatarCache::fetch(const QString &url) {
    pending.insert(url);

    // Ghép full URL (bỏ "/api")
    QString baseUrl = AppConfig::instance().getBaseUrl();
    baseUrl.chop(4);
    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(baseUrl + url)));

    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        pending.remove(url);
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap px;
            if (px.loadFromData(reply->readAll())) {
                cache[url] = px;
                emit avatarLoaded(url, px);
            }
        }
        reply->deleteLater();
    });
}