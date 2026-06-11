#include "signupmodel.h"
#include "appconfig.h"
#include <QNetworkReply>
#include <QDebug>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>

SignUpModel::SignUpModel(QObject *parent)
    : QObject(parent){
    networkManager = new QNetworkAccessManager(this);
}

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
    if (!checkCredentials()) {
        emit registrationFailed("Thông tin không đầy đủ hoặc mật khẩu không khớp!");
        return false;
    }
    QString baseUrl = AppConfig::instance().getBaseUrl();
    QUrl url(baseUrl + "/api/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString hashedPass = hashPassword(password);
    QJsonObject json;
    json["display_name"] = displayName.trimmed();
    json["username"] = userName.trimmed();
    json["password"] = hashedPass;
    QJsonDocument doc(json);

    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        QByteArray response = reply->readAll();
        if(reply->error() != QNetworkReply::NoError){
            emit registrationFailed(reply->errorString());
            reply->deleteLater();
            return;
        }
        QJsonDocument resDoc = QJsonDocument::fromJson(response);
        QJsonObject object = resDoc.object();
        if(!object["status"].toInt()){
            emit registrationFailed(object["message"].toString());
        }else{
            QJsonObject data = object["data"].toObject();
            int userId = data["user_id"].toInt();
            QString token = data["token"].toString();
            QString username = data["username"].toString();
            QString displayName = data["display_name"].toString();
            QSettings settings("BKAV", "ChatApp");

            settings.setValue("auth/token", token);
            settings.setValue("auth/user_id", userId);
            settings.setValue("auth/username", username);
            settings.setValue("auth/display_name", displayName);

            emit registrationSuccess();
        }
        reply->deleteLater();
        return;
    });
    return true;
}