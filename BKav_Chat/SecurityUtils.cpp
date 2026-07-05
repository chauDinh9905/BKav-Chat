#include "SecurityUtils.h"
#include <QString>
#include <QCryptographicHash>

QString SecurityUtils::process(const QString &data, const QString &key) {
    QString result = data;
    for (int i = 0; i < data.length(); ++i) {
        result[i] = QChar(data[i].unicode() ^ key[i % key.length()].unicode());
    }
    return result;
}

QString SecurityUtils::generateLocalKey(const QString &authToken, qint64 userId) {
    QString rawKey = authToken + QString::number(userId);
    return QString(QCryptographicHash::hash(rawKey.toUtf8(), QCryptographicHash::Sha256).toHex());
}