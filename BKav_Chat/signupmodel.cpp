#include "signupmodel.h"

SignUpModel::SignUpModel(QObject *parent)
    : QObject(parent){}

bool SignUpModel::checkCredentials(){
    if(displayName.isEmpty() || userName.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()){
        return false;
    }
    return (password == confirmPassword);
}

bool SignUpModel::registerOnServer(){
    // Về sau sẽ xử lý code chi tiết ở đây khi đăng ký tài khoản
    return true;
}