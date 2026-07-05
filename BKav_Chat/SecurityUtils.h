#ifndef SECURITYUTILS_H
#define SECURITYUTILS_H
#include <QString>

class SecurityUtils {
public:
    static QString process(const QString &data, const QString &key);
    static QString generateLocalKey(const QString &authToken, qint64 userId);
};
#endif // SECURITYUTILS_H
