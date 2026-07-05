#ifndef ACCOUNTMANAGE_H
#define ACCOUNTMANAGE_H
#include <QObject>
#include <QStandardPaths>
#include <QDir>

class AccountManager : public QObject {
    Q_OBJECT
public:
    static AccountManager& instance() {
        static AccountManager inst;
        return inst;
    }

    void setActiveUser(const QString &userId) {
        m_activeId = userId;
        // Tạo thư mục nếu chưa tồn tại
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                       + "/Users/" + userId;
        QDir().mkpath(path);
        emit userChanged(userId);
    }

    QString getActiveUserDir() const {
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        + "/Users/" + m_activeId;
    }

    QString getActiveId() const { return m_activeId; }

signals:
    void userChanged(const QString &userId);

private:
    AccountManager() = default;
    QString m_activeId;
};

#endif // ACCOUNTMANAGE_H
