#include "chat.h"
#include "SocketManager.h"
#include "appconfig.h"
#include "chatdelegate.h"
#include "chat.h"
#include "DatabaseManager.h"
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>

using namespace std;

Chat::Chat(
    qint64 myId,
    const QString &friendId,
    const QString &friendName,
    const QString &avatarPath,
    QWidget *parent)
    :
    QWidget(parent),
    myId(myId),
    friendId(friendId),
    friendName(friendName),
    avatarPath(avatarPath)
{
    qDebug() << "Chat constructor start";
    resize(650,700);

    setStyleSheet(
        "background:white;"
        );

    networkManager =
        new QNetworkAccessManager(this);

    model =
        new ChatModel(this);

    headerLayout =
        new QHBoxLayout();

    avatarLabel =
        new QLabel();

    avatarLabel->setFixedSize(40,40);

    nameLabel =
        new QLabel(friendName);

    nameLabel->setStyleSheet(
        "font-size:16px;"
        "font-weight:bold;"
        );

    closeButton =
        new QPushButton("X");

    closeButton->setFixedSize(30,30);

    headerLayout->addWidget(
        avatarLabel);

    headerLayout->addWidget(
        nameLabel);

    headerLayout->addStretch();

    headerLayout->addWidget(
        closeButton);

    messageView =
        new QListView(this);

    messageView->setModel(model);

    messageView->setItemDelegate(new ChatDelegate(this));
    messageView->setSpacing(2);

    messageView->setStyleSheet(
        "border:none;"
        "background:#F5F5F5;"
        );

    messageEdit =
        new QLineEdit();

    messageEdit->setPlaceholderText(
        "Nhập tin nhắn..."
        );

    messageEdit->setMinimumHeight(40);

    sendButton =
        new QPushButton();

    sendButton->setText("▶");

    emojiButton =
        new QPushButton("😊");

    imageButton =
        new QPushButton("🖼");

    fileButton =
        new QPushButton("📎");

    sendButton->setFixedSize(40,40);
    emojiButton->setFixedSize(40,40);
    imageButton->setFixedSize(40,40);
    fileButton->setFixedSize(40,40);

    bottomLayout =
        new QHBoxLayout();

    bottomLayout->addWidget(
        messageEdit);

    bottomLayout->addWidget(
        sendButton);

    bottomLayout->addWidget(
        emojiButton);

    bottomLayout->addWidget(
        imageButton);

    bottomLayout->addWidget(
        fileButton);

    attachmentPreviewBar = new QWidget();
    attachmentPreviewBar->setStyleSheet("background:#EFEFEF;");
    attachmentPreviewBar->setFixedHeight(70);
    attachmentPreviewBar->hide();

    attachmentPreviewLayout = new QHBoxLayout(attachmentPreviewBar);
    attachmentPreviewLayout->setContentsMargins(5,5,5,5);
    attachmentPreviewLayout->setSpacing(5);
    attachmentPreviewLayout->addStretch();

    mainLayout =
        new QVBoxLayout(this);

    mainLayout->addLayout(
        headerLayout);

    mainLayout->addWidget(
        messageView);
    mainLayout->addWidget(attachmentPreviewBar);

    mainLayout->addLayout(
        bottomLayout);

    connect(
        sendButton,
        &QPushButton::clicked,
        this,
        &Chat::sendMessage);

    connect(
        imageButton,
        &QPushButton::clicked,
        this,
        &Chat::selectImage);

    connect(
        fileButton,
        &QPushButton::clicked,
        this,
        &Chat::selectFile);

    connect(
        closeButton,
        &QPushButton::clicked,
        this,
        &Chat::closeChat);
    connect(
        &SocketManager::instance(),
        &SocketManager::messageReceived,
        this,
        &Chat::onMessageReceived);
    connect(messageView, &QListView::clicked, this, [this](const QModelIndex &index){
        QVector<FileInfo> files = index.data(ChatModel::FilesRole).value<QVector<FileInfo>>();
        if(!files.isEmpty()){
            QString baseUrl = AppConfig::instance().getBaseUrl();
            this->downloadFile(baseUrl + files[])
        }
    });

    if (auto *delegate = qobject_cast<ChatDelegate*>(messageView->itemDelegate())) {
        connect(delegate, &ChatDelegate::fileClicked, this, &Chat::downloadFile);
    }

    DatabaseManager::instance().init(myId);
    loadMessages();
}

void Chat::selectImage()
{
    QStringList files =
        QFileDialog::getOpenFileNames(
            this,
            "Chọn ảnh",
            "",
            "Images (*.png *.jpg *.jpeg)"
            );

    if(files.isEmpty())
        return;

    for (const QString &f : files) {
        pendingAttachments.append({f, true});
        addAttachmentPreview(f, true);
    }
}

void Chat::selectFile()
{
    QStringList files =
        QFileDialog::getOpenFileNames(
            this,
            "Chọn file");

    if(files.isEmpty())
        return;

    for (const QString &f : files) {
        pendingAttachments.append({f, false});
        addAttachmentPreview(f, false);
    }
}

void Chat::addAttachmentPreview(const QString &filePath, bool isImage)
{
    QWidget *thumb = new QWidget();
    thumb->setFixedSize(60,60);

    QVBoxLayout *thumbLayout = new QVBoxLayout(thumb);
    thumbLayout->setContentsMargins(0,0,0,0);

    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(50,50);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("border:1px solid #ccc; background:white;");

    if (isImage) {
        QPixmap pix(filePath);
        iconLabel->setPixmap(pix.scaled(50,50, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        iconLabel->setText("📄");
        iconLabel->setToolTip(QFileInfo(filePath).fileName());
    }
    thumbLayout->addWidget(iconLabel);

    QPushButton *removeBtn = new QPushButton("×", thumb);
    removeBtn->setFixedSize(16,16);
    removeBtn->move(44,0);
    removeBtn->setStyleSheet("border-radius:8px; background:#999; color:white; font-size:10px;");

    connect(removeBtn, &QPushButton::clicked, this, [=]() {
        pendingAttachments.removeAll(PendingAttachment{filePath, isImage});
        thumb->deleteLater();
        if (pendingAttachments.isEmpty())
            attachmentPreviewBar->hide();
    });

    attachmentPreviewLayout->insertWidget(attachmentPreviewLayout->count() - 1, thumb);
    attachmentPreviewBar->show();
}

void Chat::clearAttachments()
{
    pendingAttachments.clear();
    while (attachmentPreviewLayout->count() > 1) {
        QLayoutItem *item = attachmentPreviewLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    attachmentPreviewBar->hide();
}
void Chat::sendMessage()
{
    QString text = messageEdit->text().trimmed();
    if (text.isEmpty()&& pendingAttachments.isEmpty()) return;

    //QSettings settings("BKAV", "ChatApp");
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    if (token.isEmpty()) {
        qDebug() << "Token missing";
        return;
    }
    QString baseUrl = AppConfig::instance().getBaseUrl();

    QUrl url(baseUrl + "/message/send-message");

    QNetworkRequest request(url);
    request.setRawHeader("Authorization",QString("Bearer %1").arg(token).toUtf8());
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart friendPart;
    friendPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                         QVariant("form-data; name=\"FriendID\""));
    friendPart.setBody(friendId.toUtf8());
    multiPart->append(friendPart);

    QHttpPart contentPart;
    contentPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                          QVariant("form-data; name=\"Content\""));
    contentPart.setBody(text.toUtf8());
    multiPart->append(contentPart);

    QMimeDatabase mimeDb;
    for (const auto &att : pendingAttachments) {
        QFile *file = new QFile(att.filePath);
        if (!file->open(QIODevice::ReadOnly)) { delete file; continue; }

        QHttpPart filePart;
        QString mimeType = mimeDb.mimeTypeForFile(att.filePath).name();
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mimeType));
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant(QString("form-data; name=\"files\"; filename=\"%1\"")
                                        .arg(QFileInfo(att.filePath).fileName())));
        filePart.setBodyDevice(file);
        file->setParent(multiPart);
        multiPart->append(filePart);
    }


    QNetworkReply *reply = networkManager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject resObj = doc.object();

        if (resObj["status"].toInt() != 1) {
            qDebug() << resObj["message"].toString();
            reply->deleteLater();
            return;
        }

        QJsonObject data = resObj["data"].toObject();
        QJsonArray filesJson = data["Files"].toArray();     // dùng cho insertMessage (SQLite)
        QJsonArray imagesJson = data["Images"].toArray();
        auto images = parseImages(data["Images"].toArray());
        auto files = parseFiles(data["Files"].toArray());

        DatabaseManager::instance().insertMessage(
            data["id"].toString(), myId, friendId.toLongLong(),
            text, filesJson, imagesJson, QDateTime::currentDateTime().toString()
            );
        // update UI từ server response
        MessageInfo msg(
            myId,
            friendId.toLongLong(),
            data["Content"].toString(),
            files,
            images,
            QDateTime::currentDateTime(),
            QDateTime::currentDateTime(),
            1,
            true
            );

        model->addMessage(msg);
        //SocketManager::instance().sendMessage(myId, friendId.toLongLong(), text);
        qDebug() << "myId: " << myId << " friendID: " << friendId << " " << text;
        messageEdit->clear();
        clearAttachments();
        reply->deleteLater();
    });
}

QVector<ImageInfo> Chat::parseImages(const QJsonArray &arr) {
    QVector<ImageInfo> result;
    for (auto v : as_const(arr)) {
        QJsonObject o = v.toObject();
        result.append({o["urlImage"].toString(), o["FileName"].toString()});
    }
    return result;
}
QVector<FileInfo> Chat::parseFiles(const QJsonArray &arr) {
    QVector<FileInfo> result;
    for (auto v : as_const(arr)) {
        QJsonObject o = v.toObject();
        result.append({o["urlFile"].toString(), o["FileName"].toString()});
    }
    return result;
}
void Chat::closeChat()
{
    qDebug() << "closeChat called";
    qDebug() << "emit closeRequested";
    emit closeRequested();
}

void Chat::onMessageReceived(
    const QString &message)
{
    qDebug() << "onMessageReceived:" << message;
    QJsonDocument doc =QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON received ở onMessageReceived";
        return;
    }

    QJsonObject obj =
        doc.object();

    qint64 senderId =
        obj["from"].toVariant().toLongLong();

    qDebug() << "friend id:" << friendId.toLongLong();

    if(senderId != friendId.toLongLong())
        return;

    QString content = obj["content"].toString();

    QVector<ImageInfo> imagesVector;
    if (obj.contains("images") && obj["images"].isArray()) {
        QJsonArray imgArr = obj["images"].toArray();
        for (auto v : as_const(imgArr)) {
            QJsonObject imgObj = v.toObject();
            ImageInfo img;
            img.urlImage = imgObj["urlImage"].toString();
            img.fileName = imgObj["FileName"].toString();
            imagesVector.append(img);
        }
    }

    QVector<FileInfo> filesVector;
    if (obj.contains("files") && obj["files"].isArray()) {
        QJsonArray fileArr = obj["files"].toArray();
        for (auto v : as_const(fileArr)) {
            QJsonObject fileObj = v.toObject();
            FileInfo f;
            f.urlFile = fileObj["urlFile"].toString();
            f.fileName = fileObj["FileName"].toString();
            filesVector.append(f);
        }
    }
    MessageInfo msg(
        senderId,
        myId,
        content,
        filesVector,
        imagesVector,
        QDateTime::currentDateTime(),
        QDateTime::currentDateTime(),
        1,
        false);

    model->addMessage(msg);
}

void Chat::loadMessages()
{
    //QSettings settings("BKAV", "ChatApp");
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    if (token.isEmpty()) {
        qDebug() << "Token missing";
        return;
    }
    QString baseUrl = AppConfig::instance().getBaseUrl();

    QUrl url(baseUrl + "/message/get-message?FriendID=" + friendId);
    qDebug() << "friendId from get message: " << friendId;

    model->clear();

    auto cachedMsgs = DatabaseManager::instance().getMessages(myId, friendId.toLongLong());
    for(auto& msgData : cachedMsgs) {
        // Chuyển QVariantMap thành MessageInfo và add vào model
        MessageInfo msg(msgData["sender_id"].toLongLong(), msgData["friend_id"].toLongLong(), msgData["content"].toString(), {}, {}, QDateTime::fromString(msgData["created_at"].toString(), Qt::ISODate), QDateTime::currentDateTime(),1, msgData["sender_id"].toLongLong() == myId);
        model->addMessage(msg);
    }

    QNetworkRequest request(url);
    request.setRawHeader("Authorization",QString("Bearer %1").arg(token).toUtf8());
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj["status"].toInt() != 1) {
            reply->deleteLater();
            return;
        }

        QJsonArray arr = obj["data"].toArray();

        for (auto v : as_const(arr)) {
            QJsonObject m = v.toObject();
            bool isMine = (m["MessageType"].toInt() == 1);
            qint64 senderId = isMine ? myId : friendId.toLongLong();
            qint64 receiverId = isMine ? friendId.toLongLong() : myId;

            QVector<ImageInfo> imagesVector;
            QJsonArray imgArr = m["Images"].toArray();
            for(auto imgVal : as_const(imgArr)) {
                QJsonObject imgObj = imgVal.toObject();
                ImageInfo img;
                img.urlImage = imgObj["urlImage"].toString();
                img.fileName = imgObj["FileName"].toString();
                imagesVector.append(img);
            }

            QVector<FileInfo> filesVector;
            QJsonArray fileArr = m["Files"].toArray();
            for(auto fileVal : as_const(fileArr)) {
                QJsonObject fileObj = fileVal.toObject();
                FileInfo f;
                f.urlFile = fileObj["urlFile"].toString();
                f.fileName = fileObj["FileName"].toString();
                filesVector.append(f);
            }

            MessageInfo msg(
                senderId,
                receiverId,
                m["Content"].toString(),
                filesVector, // Files
                imagesVector, // Images
                QDateTime::currentDateTime(), // Bạn có thể parse m["CreatedAt"].toString() nếu muốn đúng giờ
                QDateTime::currentDateTime(),
                1,
                isMine
                );

            model->addMessage(msg);
        }

        reply->deleteLater();
    });
}

MessageInfo Chat::createMessageFromVariant(const QVariantMap &data) {
    // Chuyển đổi từ QVariantMap (SQLite) hoặc QJsonObject (Server) thành MessageInfo
    return MessageInfo(
        data["sender_id"].toLongLong(),
        data["friend_id"].toLongLong(),
        data["content"].toString(),
        {}, // Bạn cần parse lại JSON string thành QVector
        {},
        QDateTime::fromString(data["created_at"].toString(), Qt::ISODate),
        QDateTime::currentDateTime(),
        1,
        (data["sender_id"].toLongLong() == myId)
        );
}

void Chat::downloadFile(const QString &url, const QString &fileName){
    QString saveFilePath = QFileDialog::getSaveFileName(this, "File", fileName);
    if(saveFilePath.isEmpty()) return;

    QUrl fileUrl(url);
    QNetworkRequest request(fileUrl);
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=](){
        if(reply->error() == QNetworkReply::NoError){
            QFile file(saveFilePath);
            if(file.open(QIODevice::WriteOnly)){
                file.write(reply->readAll());
                file.close();
            }
        }else{
            QMessageBox::critical(this, "Lỗi", "không thể tải file: " + reply->errorString());
        }
        reply->deleteLater();
    });
}
