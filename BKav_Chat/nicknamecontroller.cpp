#include "nicknamecontroller.h"
#include "appconfig.h"
#include <QMenu>
#include <QInputDialog>
#include <QLineEdit>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

NicknameController& NicknameController::instance()
{
    static NicknameController inst;
    return inst;
}

NicknameController::NicknameController(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void NicknameController::showMenuFor(const QString &friendId,
                                     const QString &currentDisplayName,
                                     const QString &originalName,
                                     const QPoint &globalPos,
                                     QWidget *parentWidget)
{
    QMenu menu(parentWidget);
    QAction *renameAction = menu.addAction("Đổi biệt danh");
    QAction *removeAction = menu.addAction("Xóa biệt danh");

    QAction *chosen = menu.exec(globalPos);
    if (!chosen) return;

    if (chosen == renameAction) {
        bool ok;
        QString newName = QInputDialog::getText(
            parentWidget, "Đổi biệt danh", "Nhập biệt danh mới:",
            QLineEdit::Normal, currentDisplayName, &ok);
        if (ok && !newName.trimmed().isEmpty()) {
            requestSetNickname(friendId, newName.trimmed());
        }
    } else if (chosen == removeAction) {
        requestRemoveNickname(friendId, originalName);
    }
}

void NicknameController::requestSetNickname(const QString &friendId, const QString &newName)
{
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    if (token.isEmpty()) {
        emit errorOccurred("Token missing");
        return;
    }

    QUrl url(AppConfig::instance().getBaseUrl() + "/nickname/set-nickname");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["FriendID"] = friendId.toLongLong();
    body["Nickname"] = newName;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, friendId, newName]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("Không thể đổi biệt danh: " + reply->errorString());
            reply->deleteLater();
            return;
        }
        QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
        if (res["status"].toInt() != 1) {
            emit errorOccurred(res["message"].toString());
            reply->deleteLater();
            return;
        }
        emit nicknameUpdated(friendId, newName);
        reply->deleteLater();
    });
}

void NicknameController::requestRemoveNickname(const QString &friendId, const QString &originalName)
{
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    if (token.isEmpty()) {
        emit errorOccurred("Token missing");
        return;
    }

    QUrl url(AppConfig::instance().getBaseUrl() + "/nickname/remove-nickname");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["FriendID"] = friendId.toLongLong();

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, friendId, originalName]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("Không thể xóa biệt danh: " + reply->errorString());
            reply->deleteLater();
            return;
        }
        QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
        if (res["status"].toInt() != 1) {
            emit errorOccurred(res["message"].toString());
            reply->deleteLater();
            return;
        }
        emit nicknameUpdated(friendId, originalName);
        reply->deleteLater();
    });
}