#ifndef LOGINMODEL_H
#define LOGINMODEL_H

#include <QString>
#include <QCheckBox>
#include <QPushButton>

class LogInModel:public QObject{
    Q_OBJECT

public:
    explicit LogInModel(Object *parrent = nullptr);

    QString account;
    QString password;
    bool rememberInfo;
    bool validate();
};

#endif // LOGINMODEL_H
