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
    explicit NicknameController(QWidget *parentWidget, QObject *parent = nullptr);

public slots:
    void handleRequest(const QString &friendId,
                       const QString &currentDisplayName,
                       const QString &originalName,
                       const QPoint &globalPos);

signals:
    void nicknameUpdated(const QString &friendId, const QString &newDisplayName);
    void errorOccurred(const QString &message);

private:
    void requestSetNickname(const QString &friendId, const QString &newName);
    void requestRemoveNickname(const QString &friendId, const QString &originalName);

    QWidget *m_parentWidget;
    QNetworkAccessManager *m_networkManager;
};

#endif // NICKNAMECONTROLLER_H