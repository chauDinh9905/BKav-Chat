#include "loginmodel.h"
#include <QObject>
#include <QString>
#include <QCryptographicHash>
#include "appconfig.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>

LogInModel::LogInModel(QObject *parent)
    :QObject(parent){
    networkManager = new QNetworkAccessManager(this);
}

bool LogInModel::validateInfo(){
    if(account.isEmpty() || password.isEmpty()){
        return false;
    }
    return true;
}

void LogInModel::authenticateWithServer(){

    if (!validateInfo())
    {
        emit authenticationFailed();
        return;
    }

    QString baseUrl = AppConfig::instance().getBaseUrl();

    QUrl url(baseUrl + "/api/auth/login");

    QNetworkRequest request(url);

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QJsonObject json;
    json["username"] = account;
    json["password"] = passwordHash;
    QJsonDocument doc(json);

    QNetworkReply *reply = networkManager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if(reply->error() == QNetworkReply::NoError){
            QByteArray responseData = reply->readAll();
            QJsonDocument resDoc = QJsonDocument::fromJson(responseData);
            QJsonObject resJson = resDoc.object();

            if(resJson["status"].toInt()){
                QJsonObject data =
                    resJson["data"].toObject();

                int userId =
                    data["user_id"].toInt();

                QString token =
                    data["token"].toString();

                QString username =
                    data["username"].toString();

                QString displayName = data["display_name"].toString();

                QSettings settings("BKAV", "ChatApp");

                settings.setValue("auth/token", token);
                settings.setValue("auth/user_id", userId);
                settings.setValue("auth/username", username);
                settings.setValue("auth/display_name", username);

                emit authenticationSucceeded(userId);
            }else{
                emit authenticationFailed();
            }
        }
    });
}