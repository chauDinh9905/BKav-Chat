#ifndef LOGINMODEL_H
#define LOGINMODEL_H

#include <QString>
#include <QCheckBox>
#include <QPushButton>
#include <QObject>

class LogInModel:public QObject{
    Q_OBJECT

public:
    explicit LogInModel(QObject *parent = nullptr);

    QString account;
    QString password;
    bool rememberInfo;
    bool validateInfo();
    void authenticateWithServer();

signals:
    void authenticationSucceeded();
    void authenticationFailed();

};

#endif // LOGINMODEL_H
