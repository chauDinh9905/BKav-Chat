#include "loginmodel.h"
#include <QObject>
#include <QString>

LogInModel::LogInModel(QObject *parent)
    :QObject(parent){}

bool LogInModel::validateInfo(){
    if(account.isEmpty() || password.isEmpty()){
        return false;
    }
    return true;
}

void LogInModel::authenticateWithServer(){

}