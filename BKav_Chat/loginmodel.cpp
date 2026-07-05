#include "loginmodel.h"
#include <QObject>
#include <QString>
#include <QCryptographicHash>
#include "appconfig.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include "SecurityUtils.h"
#include "DatabaseManager.h"

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
    qDebug() << " Base URL:" << baseUrl;

    QUrl url(baseUrl + "/auth/login");
    qDebug() << " Full URL:" << url.toString();
    QNetworkRequest request(url);

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    qDebug() << " Signup - Plain password:" << password;
    qDebug() << "Signup - Hashed password:" << passwordHash;
    QJsonObject json;
    json["username"] = account;
    json["password"] = passwordHash;
    QJsonDocument doc(json);

    qDebug() << " Login request to:" << url.toString();
    qDebug() << " Payload:" << doc.toJson();

    QNetworkReply *reply = networkManager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << " HTTP Status Code:" << statusCode;
        if(reply->error() == QNetworkReply::NoError){
            QByteArray responseData = reply->readAll();
            qDebug() << "Server response:" << responseData;
            QJsonDocument resDoc = QJsonDocument::fromJson(responseData);
            if(resDoc.isNull()) {
                qDebug() << "Invalid JSON response";
                emit authenticationFailed();
                reply->deleteLater();
                return;
            }
            QJsonObject resJson = resDoc.object();
            qDebug() << "Parsed JSON:" << resJson;
            if(resJson["status"].toInt()){
                QJsonObject data = resJson["data"].toObject();

                //  SỬA: Dùng qint64 để tránh tràn số
                qint64 userId = data["user_id"].toVariant().toLongLong();
                QString token = data["token"].toString();
                QString username = data["username"].toString();
                QString displayName = data["display_name"].toString();
                AppConfig::instance().setProfile(QString::number(userId));

                qDebug() << "Login success! User ID:" << userId;
                qDebug() << "Token:" << token;

                QString localKey = SecurityUtils::generateLocalKey(token, userId);
                DatabaseManager::instance().setEncryptionKey(localKey);
                DatabaseManager::instance().init(userId);
                //QSettings settings("BKAV", "ChatApp");
                QString configPath = AppConfig::instance().getConfigFilePath();
                QSettings settings(configPath, QSettings::IniFormat);
                settings.setValue("auth/token", token);
                settings.setValue("auth/user_id", userId);
                settings.setValue("auth/username", username);
                settings.setValue("auth/display_name", username);

                emit authenticationSucceeded(userId);
            }else{
                qDebug() << " login error:" << reply->errorString();
                emit authenticationFailed();
                reply->deleteLater();
                return;
            }
        }else{
            qDebug() << "Lỗi kết nối mạng thật sự:" << reply->errorString();
            emit authenticationFailed();
            reply->deleteLater();
            return;
        }
    });
}