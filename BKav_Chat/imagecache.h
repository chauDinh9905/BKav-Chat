#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>

class ImageCache : public QObject
{
    Q_OBJECT
public:
    static ImageCache& instance()
    {
        static ImageCache inst;
        return inst;
    }

    // Trả về pixmap nếu đã có trong cache (memory hoặc disk).
    // Nếu chưa có, bắt đầu tải ngầm và trả về pixmap rỗng; khi xong sẽ emit imageReady(url).
    QPixmap get(const QString &fullUrl)
    {
        if (memCache.contains(fullUrl))
            return memCache.value(fullUrl);

        QString diskPath = diskPathFor(fullUrl);
        if (QFile::exists(diskPath)) {
            QPixmap pix(diskPath);
            memCache.insert(fullUrl, pix);
            return pix;
        }

        if (!pending.contains(fullUrl)) {
            pending.insert(fullUrl);
            download(fullUrl);
        }
        return QPixmap();
    }

signals:
    void imageReady(const QString &url);

private:
    ImageCache()
    {
        cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/img_cache";
        QDir().mkpath(cacheDir);
        netManager = new QNetworkAccessManager(this);
    }

    QString diskPathFor(const QString &url) const
    {
        QString hash = QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex();
        return cacheDir + "/" + hash + ".cache";
    }

    void download(const QString &fullUrl)
    {
        QNetworkReply *reply = netManager->get(QNetworkRequest(QUrl(fullUrl)));
        connect(reply, &QNetworkReply::finished, this, [=]() {
            pending.remove(fullUrl);
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QPixmap pix;
                if (pix.loadFromData(data)) {
                    memCache.insert(fullUrl, pix);
                    QFile f(diskPathFor(fullUrl));
                    if (f.open(QIODevice::WriteOnly))
                        f.write(data);
                    emit imageReady(fullUrl);
                }
            }
            reply->deleteLater();
        });
    }

    QHash<QString, QPixmap> memCache;
    QSet<QString> pending;
    QString cacheDir;
    QNetworkAccessManager *netManager;
};

#endif