#include "signupmodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>

SignUpModel::SignUpModel(QObject *parent)
    : QObject(parent){}

bool SignUpModel::checkCredentials(){
    if(displayName.isEmpty() || userName.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()){
        return false;
    }
    return (password == confirmPassword);
}

QString SignUpModel::hashPassword(const QString &plainPassword)
{
    QByteArray hash = QCryptographicHash::hash(plainPassword.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

bool SignUpModel::registerOnServer(){
    // Về sau sẽ xử lý code chi tiết ở đây khi đăng ký tài khoản, phải kiểm tra xem thông tin đăng ký, nếu tên hiển thị bị trùng với người khác thì trả về,
    // hiện tại thì kết nối thẳng đến database trên máy local, chưa thông qua server
    if (!checkCredentials()) {
        emit registrationFailed("Thông tin không đầy đủ hoặc mật khẩu không khớp!");
        return false;
    }
    if (!DatabaseManager::instance().connectToDatabase()) {
        emit registrationFailed("Không thể kết nối đến database!");
        return false;
    }
    QString hashedPass = hashPassword(password);
    QSqlQuery query;
    query.prepare("INSERT INTO users (display_name, username, password_hash) "
                  "VALUES (:display_name, :username, :password_hash)");

    query.bindValue(":display_name", displayName.trimmed());
    query.bindValue(":username", userName.trimmed());
    query.bindValue(":password_hash", hashedPass);

    if (query.exec()) {
        qDebug() << "Đăng ký thành công:" << userName;
        return true;
    } else {
        QString err = query.lastError().text();
        if (err.contains("Duplicate", Qt::CaseInsensitive)) {
            emit registrationFailed("Tài khoản đã tồn tại!");
        } else {
            emit registrationFailed("Lỗi: " + err);
        }
        qDebug() << "Register error:" << err;
        return false;
    }
    return true;
}