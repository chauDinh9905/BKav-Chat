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
#include <QScrollArea>
#include <QGridLayout>
#include <QKeyEvent>
#include <QTimer>
#include <algorithm>
using namespace std;

Chat::Chat(
    qint64 myId,const QString &friendId,const QString &friendName,const QString &avatarPath,QWidget *parent)
    :QWidget(parent),myId(myId),friendId(friendId),friendName(friendName),avatarPath(avatarPath)
{
    qDebug() << "Chat constructor start";
    resize(650,700);

    setStyleSheet("background:white;");

    networkManager = new QNetworkAccessManager(this);

    model =new ChatModel(this);

    headerLayout =new QHBoxLayout();

    avatarLabel =new QLabel();

    avatarLabel->setFixedSize(40,40);

    nameLabel =new QLabel(friendName);

    nameLabel->setStyleSheet("font-size:16px;""font-weight:bold;");

    closeButton =new QPushButton("X");

    closeButton->setFixedSize(30,30);

    headerLayout->addWidget(avatarLabel);

    headerLayout->addWidget(nameLabel);

    headerLayout->addStretch();

    headerLayout->addWidget(closeButton);

    messageView =new QListView(this);

    messageView->setModel(model);

    messageView->setItemDelegate(new ChatDelegate(this));
    messageView->setSpacing(2);
    messageView->setResizeMode(QListView::Adjust);
    messageView->setStyleSheet("border:none;""background:#F5F5F5;");
    if(auto *delegate = qobject_cast<ChatDelegate*>(messageView->itemDelegate())){
        if(!avatarPath.isEmpty()){
            QString avatarBase = AppConfig::instance().getBaseUrl();
            avatarBase.chop(4);
            delegate->setFriendAvatarUrl(avatarBase + avatarPath);
        }
    }

    messageEdit = new QTextEdit();

    messageEdit->setPlaceholderText("Nhập tin nhắn...");

    messageEdit->setFixedHeight(40);
    messageEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageEdit->installEventFilter(this);

    sendButton =new QPushButton();

    sendButton->setText("▶");

    emojiButton =new QPushButton("😊");

    imageButton =new QPushButton("🖼");
    fileButton =new QPushButton("📎");

    sendButton->setFixedSize(40,40);
    emojiButton->setFixedSize(40,40);
    imageButton->setFixedSize(40,40);
    fileButton->setFixedSize(40,40);

    bottomLayout =new QHBoxLayout();

    bottomLayout->addWidget(messageEdit);

    bottomLayout->addWidget(sendButton);

    bottomLayout->addWidget(emojiButton);

    bottomLayout->addWidget(imageButton);

    bottomLayout->addWidget(fileButton);

    attachmentPreviewBar = new QWidget();
    attachmentPreviewBar->setStyleSheet("background:#EFEFEF;");
    attachmentPreviewBar->setFixedHeight(70);
    attachmentPreviewBar->hide();

    attachmentPreviewLayout = new QHBoxLayout(attachmentPreviewBar);
    attachmentPreviewLayout->setContentsMargins(5,5,5,5);
    attachmentPreviewLayout->setSpacing(5);
    attachmentPreviewLayout->addStretch();

    attachmentPreviewScroll = new QScrollArea();     // scroll area bọc ngoài, đây mới là widget add vào mainLayout
    attachmentPreviewScroll->setWidget(attachmentPreviewBar);
    attachmentPreviewScroll->setWidgetResizable(false); // để attachmentPreviewBar tự tính sizeHint theo nội dung, không bị ép co giãn theo scroll area
    attachmentPreviewScroll->setFixedHeight(80);        // 70 (thumbnail) + margin, cố định chiều cao, không phình theo window
    attachmentPreviewScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    attachmentPreviewScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    attachmentPreviewScroll->setFrameShape(QFrame::NoFrame);
    attachmentPreviewScroll->setStyleSheet("background:#EFEFEF;");
    attachmentPreviewScroll->hide();
    mainLayout =new QVBoxLayout(this);

    mainLayout->addLayout(headerLayout);

    mainLayout->addWidget(messageView);
    mainLayout->addWidget(attachmentPreviewScroll);

    mainLayout->addLayout(bottomLayout);

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
    connect(emojiButton, &QPushButton::clicked, this, &Chat::selectEmoji);
    if (auto *delegate = qobject_cast<ChatDelegate*>(messageView->itemDelegate())) {
        connect(delegate, &ChatDelegate::fileClicked, this, &Chat::downloadFile);
    }
    connect(messageEdit, &QTextEdit::textChanged, this, &Chat::adjustMessageEditHeight);
    DatabaseManager::instance().init(myId);
    loadMessages();
}

void Chat::adjustMessageEditHeight()
{
    const int minH = 40;
    const int maxH = 120;

    int docH = static_cast<int>(messageEdit->document()->size().height()) + 12;
    int newH = qBound(minH, docH, maxH);

    if (messageEdit->height() != newH)
        messageEdit->setFixedHeight(newH);

    messageEdit->setVerticalScrollBarPolicy(
        docH > maxH ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
}

void Chat::selectImage()
{
    QStringList files =
        QFileDialog::getOpenFileNames(this,"Chọn ảnh","","Images (*.png *.jpg *.jpeg)");
    if(files.isEmpty())
        return;
    for (const QString &f : files) {
        pendingAttachments.append({f, true});
        addAttachmentPreview(f, true);
    }
    refreshAttachmentPreviewLayout();
}
void Chat::refreshAttachmentPreviewLayout()
{
    QTimer::singleShot(0, this, [this]() {
        attachmentPreviewScroll->show();
        attachmentPreviewLayout->activate();
        attachmentPreviewBar->adjustSize();
        attachmentPreviewScroll->updateGeometry();
        if (layout()) layout()->activate();});
}

void Chat::selectFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this,"Chọn file");
    if(files.isEmpty())
        return;
    for (const QString &f : files) {
        pendingAttachments.append({f, false});
        addAttachmentPreview(f, false);
    }
    refreshAttachmentPreviewLayout();
}

void Chat::addAttachmentPreview(const QString &filePath, bool isImage)
{
    int thumbW = isImage ? 60 : 170;
    QWidget *thumb = new QWidget();
    thumb->setFixedSize(thumbW, 70);
    if (isImage) {
        QVBoxLayout *thumbLayout = new QVBoxLayout(thumb);
        thumbLayout->setContentsMargins(2, 0, 2, 0);
        thumbLayout->setSpacing(2);
        QLabel *iconLabel = new QLabel();
        iconLabel->setFixedSize(50, 50);
        QPixmap pix(filePath);
        iconLabel->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("border:1px solid #ccc; background:white;");
        thumbLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    } else {
        QHBoxLayout *thumbLayout = new QHBoxLayout(thumb);
        thumbLayout->setContentsMargins(6, 0, 20, 0);   // chừa lề phải cho nút xoá "×"
        thumbLayout->setSpacing(6);
        thumbLayout->setAlignment(Qt::AlignVCenter);
        QLabel *iconLabel = new QLabel();
        iconLabel->setFixedSize(40, 40);
        iconLabel->setText("📄");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("border:1px solid #ccc; background:white;");
        thumbLayout->addWidget(iconLabel, 0, Qt::AlignVCenter);

        // Hiện tên file rút gọn ngay dưới icon, kèm "..." nếu quá dài
        QFileInfo fileInfo(filePath);
        QString fileName = QFileInfo(filePath).fileName();
        QString baseName = fileInfo.completeBaseName(); // tên không kèm đuôi, ví dụ "BaoCaoCuoiKy"
        QString suffix = fileInfo.suffix();
        QLabel *nameLabel = new QLabel();
        nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        nameLabel->setStyleSheet("font-size:9px; color:#333; background:transparent; border:none;");
        nameLabel->setWordWrap(false);
        QFontMetrics fm(nameLabel->font());
        int availWidth = thumbW - 40 - 20 - 12; // trừ icon, lề phải, spacing
        QString displayText;
        if (!suffix.isEmpty()) {
            QString ext = "." + suffix;
            int extWidth = fm.horizontalAdvance(ext);
            QString elidedBase = fm.elidedText(baseName, Qt::ElideRight, availWidth - extWidth);
            displayText = elidedBase + ext;
        } else {
            displayText = fm.elidedText(fileName, Qt::ElideRight, availWidth);
        }
        nameLabel->setText(displayText);
        nameLabel->setToolTip(fileName); // hover vẫn hiện tên đầy đủ

        thumbLayout->addWidget(nameLabel);
    }
    QPushButton *removeBtn = new QPushButton("×", thumb);
    removeBtn->setFixedSize(16, 16);
    removeBtn->move(thumbW - 16, 0);
    removeBtn->setStyleSheet("border-radius:8px; background:#999; color:white; font-size:10px;");
    connect(removeBtn, &QPushButton::clicked, this, [=]() {
        pendingAttachments.removeAll(PendingAttachment{filePath, isImage});
        attachmentPreviewLayout->removeWidget(thumb);
        thumb->deleteLater();
        attachmentPreviewLayout->activate();
        attachmentPreviewBar->adjustSize();
        if (pendingAttachments.isEmpty())
            attachmentPreviewScroll->hide();
    });
    attachmentPreviewLayout->insertWidget(attachmentPreviewLayout->count() - 1, thumb);
    attachmentPreviewLayout->activate();
    attachmentPreviewBar->adjustSize();
    attachmentPreviewScroll->updateGeometry();
}
bool Chat::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == emojiPopup) {
        if (event->type() == QEvent::Hide) {
            lastEmojiPopupCloseMs = QDateTime::currentMSecsSinceEpoch();
        }
        return QWidget::eventFilter(obj, event);
    }
    if (obj == messageEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                return false;
            }
            sendMessage();
            return true;
        }
        return false;
    }

    return QWidget::eventFilter(obj, event);
}
void Chat::selectEmoji(){
    if(emojiPopup && emojiPopup->isVisible()){
        emojiPopup->close();
        return;
    }
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if(now - lastEmojiPopupCloseMs < 150){
        return;
    }
    static const QStringList emojis = {
        "😀","😃","😄","😁","😆","😅","🤣","😂","🙂","🙃",
        "😉","😊","😇","🥰","😍","🤩","😘","😗","😚","😙",
        "😋","😛","😜","🤪","😝","🤑","🤗","🤭","🫢","🫣",
        "🤫","🤔","🫡","🤐","🤨","😐","😑","😶","🫥","😶‍🌫️",
        "😏","😒","🙄","😬","🤥","😌","😔","😪","🤤","😴",
        "😷","🤒","🤕","🤢","🤮","🥵","🥶","🥴","😵","🤯",
        "😕","😟","🙁","☹️","😮","😯","😲","😳","🥺","😭",
        "😢","😥","😰","😨","😱","😖","😣","😞","😓","😩",
        "😫","🥱","😤","😡","🤬",
        "👍","👎","👌","✌️","🤞","🤟","🤘","🤙",
        "👋","👏","🙌","👐","🤲","🙏","💪",
        "🫶","🫰","👈","👉","👆","👇","☝️","✋","🤚","🖐️",
        "❤️","🩷","🧡","💛","💚","🩵","💙","💜",
        "🖤","🩶","🤍","🤎","💔","❤️‍🔥","❤️‍🩹","❣️","💕","💞","💓","💗",
        "🐶","🐱","🐭","🐹","🐰","🦊","🐻","🐼","🐨","🐯",
        "🦁","🐮","🐷","🐸","🐵","🐔","🐧","🐦","🦄","🐝",
        "🍎","🍌","🍇","🍉","🍓","🍒","🥝","🍍",
        "🥥","🥑","🍔","🍕","🌭","🍟","🌮",
        "🍣","🍜","🍙","🍩","🍪","🍫","🍰","🎂","☕","🍺",
        "⚽","🏀","🏈","⚾","🎾","🏐","🏓","🏸",
        "🥊","🥋","🎳","⛳","🎮","🎲","🎯","🎸","🎹",
        "🚗","🚕","🚙","🚌","🚎","🏎️","🚓","🚑",
        "🚒","🚜","🚲","🛵","🏍️","✈️","🚀","🚁","🚢",
        "☀️","🌤️","⛅","🌧️","⛈️","❄️","🌈",
        "🌙","⭐","🌍","🌎","🌏","🌊","🌸","🌹","🌴","🍀"
    };
    if (!emojiPopup) {
        emojiPopup = new QWidget(this, Qt::Popup);
        emojiPopup->installEventFilter(this);
        emojiPopup->setFixedSize(260, 220);
        emojiPopup->setStyleSheet(
            "background:white;"
            "border:1px solid #DDDDDD;"
            "border-radius:10px;"
            );

        QVBoxLayout *outer = new QVBoxLayout(emojiPopup);
        outer->setContentsMargins(0, 0, 0, 0);

        QScrollArea *scroll = new QScrollArea(emojiPopup);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setStyleSheet("background:transparent;");
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *content = new QWidget();
        content->setStyleSheet("background:transparent;");
        QGridLayout *grid = new QGridLayout(content);
        grid->setSpacing(2);
        grid->setContentsMargins(8, 8, 8, 8);

        const int columns = 7;
        int row = 0, col = 0;
        for (const QString &emoji : emojis) {
            QPushButton *btn = new QPushButton(emoji, content);
            btn->setFixedSize(32, 32);
            btn->setStyleSheet(
                "QPushButton { font-size:18px; border:none; border-radius:4px; }"
                "QPushButton:hover { background:#F0F0F0; }"
                );

            connect(btn, &QPushButton::clicked, this, [this, emoji]() {
                QTextCursor cursor = messageEdit->textCursor();
                cursor.insertText(emoji);
                messageEdit->setTextCursor(cursor);
                messageEdit->setFocus();
            });

            grid->addWidget(btn, row, col);
            if (++col >= columns) { col = 0; ++row; }
        }
        grid->setRowStretch(row + 1, 1);

        scroll->setWidget(content);
        outer->addWidget(scroll);
    }

    // Neo góc dưới-phải của popup vào góc trên-phải của nút emoji
    QPoint buttonTopRight = emojiButton->mapToGlobal(QPoint(emojiButton->width(), 0));
    QPoint popupPos(buttonTopRight.x() - emojiPopup->width(),
                    buttonTopRight.y() - emojiPopup->height());
    emojiPopup->move(popupPos);
    emojiPopup->show();
}

void Chat::clearAttachments()
{
    pendingAttachments.clear();
    while (attachmentPreviewLayout->count() > 1) {
        QLayoutItem *item = attachmentPreviewLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    attachmentPreviewScroll->hide();
}
void Chat::sendMessage()
{
    QString text = messageEdit->toPlainText().trimmed();
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
            text, filesJson, imagesJson, data["CreatedAt"].toString(), data["isSend"].toInt()
            );
        // update UI từ server response
        MessageInfo msg(
            myId,
            friendId.toLongLong(),
            data["Content"].toString(),
            files,
            images,
            QDateTime::fromString(data["CreatedAt"].toString(), Qt::ISODate),
            QDateTime::currentDateTime(),
            data["isSend"].toInt(),
            true,
            data["id"].toString()
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

    QJsonObject obj =doc.object();
    QString type = obj["type"].toString();
    if(type == "message_delivered"){
        model->updateMessageStatusById(obj["messageId"].toString(), 1);
        DatabaseManager::instance().updateMessageStatus(obj["messageId"].toString(), 1);
        return;
    }
    if(type == "message_seen"){
        qint64 byId = obj["by"].toVariant().toLongLong();
        if(byId == friendId.toLongLong()){
            model->markAllMineSeen();
            DatabaseManager::instance().markAllSeenFromFriend(friendId.toLongLong(), myId);
        }
        return;
    }
    qint64 senderId =obj["from"].toVariant().toLongLong();
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
        QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate),
        QDateTime::currentDateTime(),
        1,
        false,
        obj["id"].toString());

    model->addMessage(msg);
    DatabaseManager::instance().insertMessage(
               obj["id"].toString(), senderId, myId,
               content, obj["files"].toArray(), obj["images"].toArray(),
               obj["createdAt"].toString(), 1
                   );
    SocketManager::instance().markSeen(myId, friendId.toLongLong());
}

void Chat::loadMessages()
{
    //QSettings settings("BKAV", "ChatApp");
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    QString lastTime;
    if (token.isEmpty()) {
        qDebug() << "Token missing";
        return;
    }
    QString baseUrl = AppConfig::instance().getBaseUrl();

    QUrl url(baseUrl + "/message/get-message?FriendID=" + friendId + (lastTime.isEmpty() ? "" : "&LastTime=" + lastTime));
    qDebug() << "friendId from get message: " << friendId;

    model->clear();

    auto cachedMsgs = DatabaseManager::instance().getMessages(myId, friendId.toLongLong());
    for(auto& msgData : cachedMsgs) {
        // Chuyển QVariantMap thành MessageInfo và add vào model
        auto images = parseImages(msgData["images"].toJsonArray());
        auto files  = parseFiles(msgData["files"].toJsonArray());
        MessageInfo msg(msgData["sender_id"].toLongLong(), msgData["friend_id"].toLongLong(), msgData["content"].toString(), files,images, QDateTime::fromString(msgData["created_at"].toString(), Qt::ISODate), QDateTime::currentDateTime(),msgData["is_send"].toInt(), msgData["sender_id"].toLongLong() == myId, msgData["id"].toString());
        model->addMessage(msg);
        lastTime = msgData["created_at"].toString();
    }
    QTimer::singleShot(0, this, [this]() {
        messageView->scrollToBottom();
    });
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
            QString msgId = m["id"].toString();
            if(!model->hasMessage(msgId)){
                MessageInfo msg(
                    senderId,
                    receiverId,
                    m["Content"].toString(),
                    filesVector, // Files
                    imagesVector, // Images
                    QDateTime::fromString(m["CreatedAt"].toString(), Qt::ISODate), // có thể parse m["CreatedAt"].toString() nếu muốn đúng giờ
                    QDateTime::currentDateTime(),
                    m["isSend"].toInt(),
                    isMine,
                    m["id"].toString()
                    );

                model->addMessage(msg);
                DatabaseManager::instance().insertMessage(
                                   m["id"].toString(), senderId, receiverId,
                                   m["Content"].toString(), m["Files"].toArray(), m["Images"].toArray(),
                                   m["CreatedAt"].toString(), m["isSend"].toInt()
                                       );
            }
        }
        QTimer::singleShot(0, this, [this]() {
            messageView->scrollToBottom();
        });
        SocketManager::instance().markSeen(myId, friendId.toLongLong());
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
    qDebug() << "url file:" << url;
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
