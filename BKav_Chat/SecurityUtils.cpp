#include "SecurityUtils.h"
#include <QString>
#include <QCryptographicHash>
#include <QDebug>
#include <QRegularExpression>

QString SecurityUtils::process(const QString &data, const QString &key) {
    if (key.isEmpty()) {
        qDebug() << "SecurityUtils::process gọi với key rỗng, bỏ qua mã hoá";
        return data;
    }
    if (data.isEmpty()) return data;

    QByteArray keyBytes = key.toUtf8();

    static const QRegularExpression base64Pattern("^[A-Za-z0-9+/]+={0,2}$");
    bool looksLikeCiphertext = (data.length() % 4 == 0) && base64Pattern.match(data).hasMatch();

    if (looksLikeCiphertext) {
        // Giải mã: data đang là base64 của bản đã XOR
        QByteArray cipherBytes = QByteArray::fromBase64(data.toLatin1());
        QByteArray plainBytes;
        plainBytes.resize(cipherBytes.size());
        for (int i = 0; i < cipherBytes.size(); ++i)
            plainBytes[i] = cipherBytes[i] ^ keyBytes[i % keyBytes.size()];
        return QString::fromUtf8(plainBytes);
    } else {
        // Mã hoá: data là plaintext gốc
        QByteArray plainBytes = data.toUtf8();
        QByteArray cipherBytes;
        cipherBytes.resize(plainBytes.size());
        for (int i = 0; i < plainBytes.size(); ++i)
            cipherBytes[i] = plainBytes[i] ^ keyBytes[i % keyBytes.size()];
        return QString::fromLatin1(cipherBytes.toBase64());
    }
}

QString SecurityUtils::generateLocalKey(const QString &authToken, qint64 userId) {
    QString rawKey = authToken + QString::number(userId);
    return QString(QCryptographicHash::hash(rawKey.toUtf8(), QCryptographicHash::Sha256).toHex());
}