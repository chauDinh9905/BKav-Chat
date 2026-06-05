#include "loginmodel.h"
#include <QObject>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

LogInModel::LogInModel(QObject *parent)
    :QObject(parent){}

bool LogInModel::validateInfo(){
    if(account.isEmpty() || password.isEmpty()){
        return false;
    }
    return true;
}

void LogInModel::authenticateWithServer(){
     // Hiện tại thì check tài khoản với database local
     // về sau khi kết nối vơi server thì sẽ khác
     QSqlQuery query;
     QString passwordHash;
     query.prepare("select u"
                   "from users u"
                   "where u.display_name = :account"
                   "and u.password_hash = :password");
     query.bindValue(":display_name", account);
     passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
     query.bindValue(":password_hash", passwordHash);
     if (!query.exec())
     {
         qDebug() << "SQL Error:" << query.lastError().text();
         return;
     }else{
             if (query.next())
             {
                 emit authenticationSucceeded();
             }
             else
             {
                 emit authenticationFailed();
              }
        }
}