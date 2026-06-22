#ifndef LOGINMODEL_H
#define LOGINMODEL_H

#include <QString>
#include <QCheckBox>
#include <QPushButton>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class LogInModel:public QObject{
    Q_OBJECT

public:
    explicit LogInModel(QObject *parent = nullptr);

    QString account;
    QString password;
    bool rememberInfo;
    bool validateInfo();
    void authenticateWithServer();

    void setAccount(const QString &acc) { account = acc; }
    void setPassword(const QString &pwd) { password = pwd; }

private:
    QNetworkAccessManager *networkManager;

signals:
    void authenticationSucceeded(qint64 user_id);
    void authenticationFailed();


};

#endif // LOGINMODEL_H
