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
    QUrl url(baseUrl + "/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString hashedPass = hashPassword(password);
    qDebug() << "🔐 Signup - Plain password:" << password;
    qDebug() << "🔐 Signup - Hashed password:" << hashedPass;
    QJsonObject json;
    json["display_name"] = displayName.trimmed();
    json["username"] = userName.trimmed();
    json["password"] = hashedPass;
    QJsonDocument doc(json);

    qDebug() << "📤 Sending to:" << url.toString();
    qDebug() << "📦 Payload:" << doc.toJson();

    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "✅ HTTP Status Code:" << statusCode;
        if(reply->error() != QNetworkReply::NoError){
            emit registrationFailed(reply->errorString());
            reply->deleteLater();
            return;int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "✅ HTTP Status Code:" << statusCode;
        }
        QByteArray response = reply->readAll();
        qDebug() << "📦 Server Response:" << response;
        QJsonDocument resDoc = QJsonDocument::fromJson(response);
        if(resDoc.isNull()) {
            qDebug() << "❌ Invalid JSON response";
            emit registrationFailed("Server returned invalid response");
            reply->deleteLater();
            return;
        }
        QJsonObject object = resDoc.object();
        qDebug() << "📊 Parsed JSON:" << object;
        if(!object["status"].toInt()){
            QString errorMsg = object["message"].toString();
            qDebug() << "❌ Server error:" << errorMsg;
            emit registrationFailed(errorMsg);
        }else{
            QJsonObject data = object["data"].toObject();
            qDebug() << "✅ Registration success! User ID:" << data["user_id"].toInt();
            //int userId = data["user_id"].toInt();
            qint64 userId = data["user_id"].toDouble();

            QString token = data["token"].toString();
            QString username = data["username"].toString();
            QString displayName = data["display_name"].toString();
            //QSettings settings("BKAV", "ChatApp");
            QString configPath = AppConfig::instance().getConfigFilePath();
            QSettings settings(configPath, QSettings::IniFormat);
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