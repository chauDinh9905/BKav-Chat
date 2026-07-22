#include "chat.h"
#include "SocketManager.h"
#include "appconfig.h"
#include "chatdelegate.h"
#include "chat.h"
#include "DatabaseManager.h"
#include "nicknamecontroller.h"
#include <QSet>
#include <algorithm>
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

using namespace std;
static const qint64 MAX_ATTACHMENT_SIZE = 100LL * 1024 * 1024; // 100MB
QHash<QString, Chat*> Chat::s_openChats;

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

    nameButton = new QPushButton(friendName);
    nameButton->setCursor(Qt::PointingHandCursor);
    nameButton->setStyleSheet(
        "QPushButton {"
        "  font-size:16px; font-weight:bold;"
        "  border:none; background:transparent;"
        "  text-align:left; padding:0px;"
        "}"
        "QPushButton:hover { color:#1565C0; }"
        );

    closeButton =new QPushButton("X");

    closeButton->setFixedSize(30,30);

    headerLayout->addWidget(avatarLabel);

    headerLayout->addWidget(nameButton);

    headerLayout->addStretch();

    headerLayout->addWidget(closeButton);

    messageView =new QListView(this);

    messageView->setModel(model);

    chatDelegate = new ChatDelegate(this);
    messageView->setItemDelegate(chatDelegate);
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
    connect(nameButton, &QPushButton::clicked, this, [this]() {
        QPoint globalPos = nameButton->mapToGlobal(QPoint(0, nameButton->height()));
        NicknameController::instance().showMenuFor(this->friendId, nameButton->text(), this->friendName, globalPos, this);
    });

    connect(&NicknameController::instance(), &NicknameController::nicknameUpdated, this,
            [this](const QString &fId, const QString &newName) {
                if (fId == this->friendId) {
                    setDisplayName(newName);
                }
            });

    connect(&NicknameController::instance(), &NicknameController::errorOccurred, this,
            [this](const QString &msg) {
                QMessageBox::warning(this, "Lỗi", msg);
            });
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
        connect(delegate, &ChatDelegate::imageClicked, this, &Chat::showImagePreview);
    }
    connect(messageEdit, &QTextEdit::textChanged, this, &Chat::adjustMessageEditHeight);
    connect(&DatabaseManager::instance(), &DatabaseManager::messagesReady,
            this, &Chat::onCachedMessagesReady);
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
    QStringList tooLargeFiles;
    for (const QString &f : files) {
        QFileInfo fi(f);
        if (fi.size() > MAX_ATTACHMENT_SIZE) {
            tooLargeFiles << fi.fileName();
            continue;
        }
        pendingAttachments.append({f, true});
        addAttachmentPreview(f, true);
    }
    refreshAttachmentPreviewLayout();
    if (!tooLargeFiles.isEmpty()) {
        QMessageBox::warning(this, "File quá lớn",
                             "Các ảnh sau vượt quá 100MB và không được thêm:\n" + tooLargeFiles.join("\n"));
    }
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
    QStringList tooLargeFiles;
    for (const QString &f : files) {
        QFileInfo fi(f);
        if (fi.size() > MAX_ATTACHMENT_SIZE) {
            tooLargeFiles << fi.fileName();
            continue;
        }
        pendingAttachments.append({f, false});
        addAttachmentPreview(f, false);
    }
    refreshAttachmentPreviewLayout();
    if (!tooLargeFiles.isEmpty()) {
        QMessageBox::warning(this, "File quá lớn",
                             "Các file sau vượt quá 100MB và không được thêm:\n" + tooLargeFiles.join("\n"));
    }
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
            QMessageBox::warning(this, "Lỗi kết nối",
                                 "Không thể gửi tin nhắn: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject resObj = doc.object();

        if (resObj["status"].toInt() != 1) {
            QString errMsg = resObj["message"].toString();
            qDebug() << errMsg;
            QMessageBox::warning(this, "Không thể gửi tin nhắn",
                                 errMsg.isEmpty() ? "Đã có lỗi xảy ra, vui lòng thử lại." : errMsg);

            reply->deleteLater();
            return;
        }

        QJsonObject data = resObj["data"].toObject();
        QJsonArray filesJson = data["Files"].toArray();     // dùng cho insertMessage (SQLite)
        QJsonArray imagesJson = data["Images"].toArray();
        auto images = parseImages(data["Images"].toArray());
        auto files = parseFiles(data["Files"].toArray());
        QString msgId = data["id"].toString();
        if(!model->hasMessage(msgId)){
            DatabaseManager::instance().insertMessage(
                msgId, myId, friendId.toLongLong(),
                text, filesJson, imagesJson, data["CreatedAt"].toString(), data["isSend"].toInt()
                );

            MessageInfo msg(
                myId, friendId.toLongLong(),
                data["Content"].toString(),
                files, images,
                QDateTime::fromString(data["CreatedAt"].toString(), Qt::ISODate),
                QDateTime::currentDateTime(),
                data["isSend"].toInt(),
                true,
                msgId
                );
            model->addMessage(msg);
        } else {
            // Đã có do message_sync đến trước — chỉ cần đồng bộ trạng thái isSend nếu cần
            model->updateMessageStatusById(msgId, data["isSend"].toInt());
        }
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
    if(type == "message_sync"){
        QString msgId = obj["id"].toString();
        qint64 fromId = obj["from"].toVariant().toLongLong();
        qint64 toId   = obj["to"].toVariant().toLongLong();

        // Chỉ quan tâm nếu đúng cuộc hội thoại đang mở trong khung chat này
        bool belongsHere = (fromId == myId && toId == friendId.toLongLong())
                           || (fromId == friendId.toLongLong() && toId == myId);
        if(!belongsHere) return;

        if(model->hasMessage(msgId)) return; // đã insert rồi (REST optimistic ở cửa sổ gốc)

        QVector<ImageInfo> imagesVector;
        if(obj.contains("images") && obj["images"].isArray()){
            QJsonArray imgArr = obj["images"].toArray();
            for(auto v : as_const(imgArr)){
                QJsonObject o = v.toObject();
                imagesVector.append({o["urlImage"].toString(), o["FileName"].toString()});
            }
        }
        QVector<FileInfo> filesVector;
        if(obj.contains("files") && obj["files"].isArray()){
            QJsonArray fileArr = obj["files"].toArray();
            for(auto v : as_const(fileArr)){
                QJsonObject o = v.toObject();
                filesVector.append({o["urlFile"].toString(), o["FileName"].toString()});
            }
        }

        bool isMine = (fromId == myId);
        MessageInfo msg(
            fromId, toId,
            obj["content"].toString(),
            filesVector, imagesVector,
            QDateTime::fromString(obj["createAt"].toString(), Qt::ISODate),
            QDateTime::currentDateTime(),
            isMine ? 0 : 1,
            isMine,
            msgId
            );
        model->addMessage(msg);
        DatabaseManager::instance().insertMessage(
            msgId, fromId, toId,
            obj["content"].toString(), obj["files"].toArray(), obj["images"].toArray(),
            obj["createAt"].toString(), isMine ? 0 : 1
            );
        return;
    }
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
        QDateTime::fromString(obj["createAt"].toString(), Qt::ISODate),
        QDateTime::currentDateTime(),
        1,
        false,
        obj["id"].toString());

    model->addMessage(msg);
    DatabaseManager::instance().insertMessage(
               obj["id"].toString(), senderId, myId,
               content, obj["files"].toArray(), obj["images"].toArray(),
               obj["createAt"].toString(), 1
                   );
    SocketManager::instance().markSeen(myId, friendId.toLongLong());
}

void Chat::loadMessages()
{
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

    model->clear();
    m_cacheMessages.clear();
    m_restMessages.clear();
    m_cacheLoaded = false;
    m_restLoaded = false;

    // 1. Yêu cầu cache SQLite (bất đồng bộ)
    m_pendingDbRequestId = DatabaseManager::instance().requestMessages(myId, friendId.toLongLong());

    // 2. Yêu cầu REST song song
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            m_restLoaded = true;
            tryMergeAndDisplay();
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj["status"].toInt() != 1) {
            m_restLoaded = true;
            tryMergeAndDisplay();
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
            for (auto imgVal : as_const(imgArr)) {
                QJsonObject imgObj = imgVal.toObject();
                imagesVector.append({imgObj["urlImage"].toString(), imgObj["FileName"].toString()});
            }
            QVector<FileInfo> filesVector;
            QJsonArray fileArr = m["Files"].toArray();
            for (auto fileVal : as_const(fileArr)) {
                QJsonObject fileObj = fileVal.toObject();
                filesVector.append({fileObj["urlFile"].toString(), fileObj["FileName"].toString()});
            }

            MessageInfo msg(
                senderId, receiverId,
                m["Content"].toString(),
                filesVector, imagesVector,
                QDateTime::fromString(m["CreatedAt"].toString(), Qt::ISODate),
                QDateTime::currentDateTime(),
                m["isSend"].toInt(),
                isMine,
                m["id"].toString()
                );
            m_restMessages.append(msg);

            // vẫn cập nhật cache như cũ, không phụ thuộc thứ tự hiển thị
            DatabaseManager::instance().insertMessage(
                m["id"].toString(), senderId, receiverId,
                m["Content"].toString(), m["Files"].toArray(), m["Images"].toArray(),
                m["CreatedAt"].toString(), m["isSend"].toInt()
                );
        }

        m_restLoaded = true;
        tryMergeAndDisplay();
        reply->deleteLater();
    });
}

void Chat::onCachedMessagesReady(quint64 requestId, QVector<QVariantMap> messages)
{
    if (requestId != m_pendingDbRequestId) return;

    for (auto &msgData : messages) {
        auto images = parseImages(msgData["images"].toJsonArray());
        auto files  = parseFiles(msgData["files"].toJsonArray());
        MessageInfo msg(
            msgData["sender_id"].toLongLong(), msgData["friend_id"].toLongLong(),
            msgData["content"].toString(), files, images,
            QDateTime::fromString(msgData["created_at"].toString(), Qt::ISODate),
            QDateTime::currentDateTime(), msgData["is_send"].toInt(),
            msgData["sender_id"].toLongLong() == myId, msgData["id"].toString());
        m_cacheMessages.append(msg);
    }

    m_cacheLoaded = true;
    tryMergeAndDisplay();
}

void Chat::tryMergeAndDisplay()
{
    if (!m_cacheLoaded || !m_restLoaded) return; // chờ đủ cả 2 nguồn

    QVector<MessageInfo> merged;
    QSet<QString> seenIds;

    // Ưu tiên dữ liệu REST (nguồn xác thực từ server) trước
    for (const auto &m : m_restMessages) {
        if (!seenIds.contains(m.messageId)) {
            merged.append(m);
            seenIds.insert(m.messageId);
        }
    }
    // Bù thêm tin chỉ có trong cache (ví dụ tin gửi khi offline, chưa kịp đồng bộ)
    for (const auto &m : m_cacheMessages) {
        if (!seenIds.contains(m.messageId)) {
            merged.append(m);
            seenIds.insert(m.messageId);
        }
    }

    std::sort(merged.begin(), merged.end(), [](const MessageInfo &a, const MessageInfo &b) {
        return a.createdAt < b.createdAt;
    });

    for (const auto &m : merged) {
        if (!model->hasMessage(m.messageId))
            model->addMessage(m);
    }

    QTimer::singleShot(0, this, [this]() { messageView->scrollToBottom(); });
    SocketManager::instance().markSeen(myId, friendId.toLongLong());
}
void Chat::finishFileProgress(const QString &url, qint64 startMs)
{
    if (!chatDelegate) return;

    const int minDisplayMs = 500; // thời gian tối thiểu để mắt kịp thấy vòng tròn
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - startMs;

    // Đảm bảo % lên tới 100 trước khi tắt, để không bị "nhảy cóc" từ 20% -> icon mũi tên
    chatDelegate->setFileProgress(url, 99);
    messageView->viewport()->repaint();

    qint64 remain = minDisplayMs - elapsed;
    if (remain <= 0) remain = 0;

    QTimer::singleShot(remain, this, [this, url](){
        if (chatDelegate) {
            chatDelegate->setFileProgress(url, -1);
            messageView->viewport()->update(); // ở đây dùng update() bình thường được, không gấp
        }
    });
}
void Chat::downloadFile(const QString &url, const QString &fileName){
    if (chatDelegate && chatDelegate->fileProgress(url) >= 0)
        return;

    QString saveFilePath = QFileDialog::getSaveFileName(this, "File", fileName);
    if(saveFilePath.isEmpty()) return;

    QUrl fileUrl(url);
    QNetworkRequest request(fileUrl);
    QNetworkReply *reply = networkManager->get(request);

    QFile *outFile = new QFile(saveFilePath);
    if(!outFile->open(QIODevice::WriteOnly)){
        QMessageBox::critical(this, "Lỗi", "Không thể tạo file để lưu.");
        delete outFile;
        reply->abort();
        reply->deleteLater();
        return;
    }

    qint64 startMs = QDateTime::currentMSecsSinceEpoch();

    if (chatDelegate) {
        chatDelegate->setFileProgress(url, 0);
        messageView->viewport()->repaint(); // repaint() = vẽ NGAY, không chờ event loop gộp lại
    }

    connect(reply, &QNetworkReply::downloadProgress, this,
            [this, url](qint64 bytesReceived, qint64 bytesTotal){
                if (bytesTotal > 0 && chatDelegate) {
                    int percent = static_cast<int>((bytesReceived * 100) / bytesTotal);
                    chatDelegate->setFileProgress(url, percent);
                    messageView->viewport()->repaint();
                }
            });

    connect(reply, &QNetworkReply::readyRead, outFile, [reply, outFile](){
        outFile->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::finished, this, [=](){
        outFile->write(reply->readAll());
        outFile->close();
        outFile->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            QFile::remove(saveFilePath);
            QMessageBox::critical(this, "Lỗi", "Không thể tải file: " + reply->errorString());
        }

        finishFileProgress(url, startMs); // thay vì set -1 ngay, gọi hàm này
        reply->deleteLater();
    });
}
void Chat::showImagePreview(const QString &url)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Xem ảnh");
    dialog->resize(600, 600);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *imgLabel = new QLabel();
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setText("Đang tải...");
    imgLabel->setMinimumSize(400, 400);
    layout->addWidget(imgLabel, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *saveBtn = new QPushButton("💾 Lưu ảnh");
    QPushButton *closeBtn = new QPushButton("Đóng");
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::close);
    connect(saveBtn, &QPushButton::clicked, this, [=]() {
        saveImageToDisk(url, dialog);
    });

    // Tải ảnh full-size để hiển thị trong dialog
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, dialog, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pix;
            if (pix.loadFromData(reply->readAll())) {
                QPixmap scaled = pix.scaled(
                    imgLabel->size().isEmpty() ? QSize(560, 560) : dialog->size(),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
                imgLabel->setPixmap(scaled);
            } else {
                imgLabel->setText("Không thể hiển thị ảnh");
            }
        } else {
            imgLabel->setText("Lỗi tải ảnh: " + reply->errorString());
        }
        reply->deleteLater();
    });

    dialog->show();
}

void Chat::saveImageToDisk(const QString &url, QWidget *parentDialog)
{
    QString fileName = QFileInfo(QUrl(url).path()).fileName();
    if (fileName.isEmpty())
        fileName = "image.png";

    QString saveFilePath = QFileDialog::getSaveFileName(
        parentDialog ? parentDialog : this, "Lưu ảnh", fileName);
    if (saveFilePath.isEmpty())
        return;

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(saveFilePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
            }
        } else {
            QMessageBox::critical(this, "Lỗi", "Không thể tải ảnh: " + reply->errorString());
        }
        reply->deleteLater();
    });
}
void Chat::setDisplayName(const QString &name)
{
    nameButton->setText(name);
}
void Chat::openChatWindow(qint64 myId, const QString &friendId,
                          const QString &friendName, const QString &avatarPath)
{
    if (s_openChats.contains(friendId)) {
        Chat *existing = s_openChats.value(friendId);
        existing->raise();
        existing->activateWindow();
        return;
    }

    Chat *chat = new Chat(myId, friendId, friendName, avatarPath, nullptr);
    chat->setWindowFlags(Qt::Window);
    chat->setAttribute(Qt::WA_DeleteOnClose);
    chat->setWindowTitle(friendName);

    s_openChats.insert(friendId, chat);

    connect(chat, &Chat::closeRequested, chat, &QWidget::close);
    connect(chat, &QObject::destroyed, chat, [friendId]() {
        s_openChats.remove(friendId);
    });

    chat->show();
    chat->raise();
    chat->activateWindow();
}

bool Chat::isChatOpen(const QString &friendId)
{
    return s_openChats.contains(friendId);
}

void Chat::closeAllChatWindows()
{
    // close() sẽ trigger destroyed -> tự remove khỏi s_openChats
    const auto chats = s_openChats.values();
    for (Chat *c : chats) {
        c->close();
    }
}