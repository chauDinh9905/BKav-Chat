#ifndef NICKNAMECONTROLLER_H
#define NICKNAMECONTROLLER_H

#include <QObject>
#include <QString>
#include <QPoint>
#include <QNetworkAccessManager>

class NicknameController : public QObject
{
    Q_OBJECT
public:
    static NicknameController& instance();

    // Hiện QMenu tại globalPos, xử lý luôn việc đổi/xóa biệt danh
    void showMenuFor(const QString &friendId,
                     const QString &currentDisplayName,
                     const QString &originalName,
                     const QPoint &globalPos,
                     QWidget *parentWidget);

signals:
    void nicknameUpdated(const QString &friendId, const QString &newDisplayName);
    void errorOccurred(const QString &message);

private:
    explicit NicknameController(QObject *parent = nullptr);
    NicknameController(const NicknameController&) = delete;
    NicknameController& operator=(const NicknameController&) = delete;

    void requestSetNickname(const QString &friendId, const QString &newName);
    void requestRemoveNickname(const QString &friendId, const QString &originalName);

    QNetworkAccessManager *m_networkManager;
};

#endif // NICKNAMECONTROLLER_H