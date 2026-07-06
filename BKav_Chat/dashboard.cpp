#include "dashboard.h"
#include "dashboardmodel.h"
#include "SocketManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QFile>
#include <QTimer>
#include "appconfig.h"
#include <QFileDialog>
#include <QPixmap>
#include <QIcon>
#include <QUrlQuery>
#include <QMenu>
#include <QHttpMultiPart>
#include <QMessageBox>
#include <QPainterPath>
#include <QPainter>

using namespace std;

Dashboard::Dashboard(DashboardModel *model, QWidget *parent)
    : QWidget(parent), model(model)
{
    this->setWindowTitle("BKav Chat");
    this->setFixedSize(550, 700);
    this->setStyleSheet("background-color: white;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(5);

    title = new QLabel("Bkav Chat", this);
    title->setStyleSheet("color: blue; font-size: 24px; font-weight: bold;");
    title->setAlignment(Qt::AlignLeft);

    avatarButton = new QPushButton(this);
    avatarButton->setFixedSize(60, 60);
    avatarButton->setStyleSheet(
        "QPushButton {"
        "   border-radius: 30px;"
        "   outline: none;"
        "}"
        );
    avatarButton->setCursor(Qt::PointingHandCursor);

    displayName = new QLabel(this);
    displayName->setStyleSheet("color: #000000; font-size: 16px; font-weight: bold;");

    searchFriend = new QLineEdit(this);
    searchFriend->setPlaceholderText("Tìm kiếm");
    searchFriend->setStyleSheet(
        "QLineEdit{"
        "   border: 1px solid #C0C0C0;"
        "   border-radius: 6px;"  // Bo góc vuông nhẹ mềm mại
        "   padding: 6px 10px;"   // Đẩy chữ lùi vào trong không bị đè lên góc bo
        "   background-color: #FAFAFA;"
        "   font-size: 13px;"
        "   color: #000000;"
        "}"
        );

    titleList = new QLabel("Danh sách bạn bè", this);
    titleList->setStyleSheet("color: #000000; font-size: 16px; font-weight: normal; margin-top: 5px;");

    friendListView = new QListView(this);
    friendListView->setStyleSheet(
        "QListView {"
        "   border: none;"
        "   background: transparent;"
        "   padding: 0px 10px; /* Thêm padding trái/phải để danh sách thu hẹp lại, không dính sát viền màn hình */"
        "}"
        );
    //friendListView->setMaximumWidth(150);
    //friendListView->setMaximumHeight(100);

    proxyModel = new FriendProxyModel(this);
    this->model = model;
    proxyModel->setSourceModel(model);
    searchDebounceTimer = new QTimer(this);

    friendListView->setModel(proxyModel);
    searchDebounceTimer->setSingleShot(true);

    headerLayout = new QHBoxLayout();
    userProfile = new QVBoxLayout();
    userProfile->setSpacing(4);
    userProfile->setContentsMargins(0, 0, 0, 0);
    userProfile->addWidget(avatarButton, 0, Qt::AlignCenter);
    userProfile->addWidget(displayName, 0, Qt::AlignCenter);
    userProfile->addStretch();

    headerLayout->addWidget(title, Qt::AlignLeft);
    headerLayout->addStretch();
    headerLayout->addLayout(userProfile);

    mainLayout->addLayout(headerLayout);
    //mainLayout->addSpacing(5);
    mainLayout->addWidget(searchFriend);
    mainLayout->addWidget(titleList);
    mainLayout->addWidget(friendListView);
    mainLayout->addStretch();


    connect(avatarButton, &QPushButton::clicked, this, &Dashboard::onAvatarClicked);
    connect(searchDebounceTimer, &QTimer::timeout, this, &Dashboard::triggerSearch);
    connect(friendListView, &QListView::clicked, this, &Dashboard::onFriendSelected);
    connect(searchFriend, &QLineEdit::textChanged, this, &Dashboard::onSearchTextChanged);

    networkManager = new QNetworkAccessManager(this);

    loadCurrentUser();
    loadFriendList();
    //SocketManager::instance().connectToServer();
    connect(&SocketManager::instance(), &SocketManager::connected,
            this, [this]() {
        m_socketConnected = true;
        if (myId != 0) {                       // myId đã sẵn sàng
            SocketManager::instance().registerUser(myId);
        }
            });
    connect(&SocketManager::instance(), &SocketManager::messageReceived,
            this, &Dashboard::onNewMessageReceived);
}

void Dashboard::loadCurrentUser()
{
    //QSettings settings("BKAV", "ChatApp");
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();

    QString baseUrl = AppConfig::instance().getBaseUrl();

    QNetworkRequest request(QUrl(baseUrl + "/user/info"));

    request.setRawHeader("Authorization",QString("Bearer %1").arg(token).toUtf8());

    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished,this,[this, reply](){
                QByteArray response = reply->readAll();

                if(reply->error() != QNetworkReply::NoError)
                {
                    qDebug() << reply->errorString();
                    reply->deleteLater();
                    return;
                }
                QJsonDocument doc = QJsonDocument::fromJson(response);
                QJsonObject obj = doc.object();
                if(obj["status"].toInt() != 1)
                {
                    qDebug()
                    << obj["message"].toString();

                    reply->deleteLater();
                    return;
                }
                QJsonObject data = obj["data"].toObject();
                myId = data["Id"].toVariant().toLongLong();
                displayName->setText(data["FullName"].toString());
                // SocketManager::instance().registerUser(myId);
                if (m_socketConnected) {                              // socket đã connect trước rồi
                    SocketManager::instance().registerUser(myId);
                }

                QString avatar = data["Avatar"] .toString();
                if(!avatar.isEmpty())
                {
                   loadAvatarFromServer(avatar);
                }
                reply->deleteLater();
            });
}

void Dashboard::loadFriendList()
{
    //QSettings settings("BKAV", "ChatApp");
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    QString baseUrl = AppConfig::instance().getBaseUrl();
    QNetworkRequest request(QUrl(baseUrl + "/message/list-friend"));
    request.setRawHeader("Authorization",QString("Bearer %1").arg(token).toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            {
                QByteArray response = reply->readAll();
                if(reply->error())
                {
                    qDebug() << reply->errorString();
                    reply->deleteLater();
                    return;
                }

                QJsonDocument doc = QJsonDocument::fromJson(response);

                QJsonObject obj = doc.object();

                if(obj["status"].toInt() != 1)
                {
                    qDebug() << obj["message"].toString();
                    reply->deleteLater();
                    return;
                }

                QJsonArray users = obj["data"].toArray();
                QVector<FriendInfo> list;
                for(const QJsonValue &value : as_const(users))
                {
                    QJsonObject user = value.toObject();
                    qDebug() << "USER =" << user;
                   // qDebug() << "user_id =" << user["user_id"].toVariant().toLongLong();

                    //list.append(FriendInfo(QString::number(user["user_id"].toVariant().toLongLong()),user["display_name"].toString(), user["avatar_path"].toString(),false, 0, QDateTime::currentDateTime()));
                    list.append(FriendInfo(
                        QString::number(user["FriendID"].toVariant().toLongLong()),
                        user["FullName"].toString(),
                        user["Avatar"].toString(),
                        user["isOnline"].toBool(),
                        user["UnreadCount"].toInt(),
                        user["LastMsgTime"].toString().isEmpty()
                            ? QDateTime()
                            : QDateTime::fromString(user["LastMsgTime"].toString(), Qt::ISODate)
                        ));
                }
                qDebug() << "LIST SIZE =" << list.size();
                model->setFriendList(list);
                for (int i = 0; i < list.size(); i++) {
                    const QString &avatarPath = list[i].avatarUrl;
                    if (avatarPath.isEmpty()){
                        continue;
                    }

                    QString baseUrl = AppConfig::instance().getBaseUrl();
                    baseUrl.chop(4);
                    QString fullUrl = baseUrl + avatarPath;
                    qDebug() << "Fetching avatar:" << fullUrl;
                    QNetworkReply *reply = networkManager->get(
                        QNetworkRequest(QUrl(fullUrl)));
                    if(!reply){
                        qDebug() << "Đường dẫn ảnh không hợp lệ";
                    }

                    connect(reply, &QNetworkReply::finished, this, [this, reply, i]() {
                        qDebug() << "Reply finished, error:" << reply->error();
                        qDebug() << "HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                        qDebug() << "Data size:" << reply->bytesAvailable();
                        if (reply->error() == QNetworkReply::NoError) {
                            QPixmap px;
                            if (px.loadFromData(reply->readAll())){
                                qDebug() << "Pixmap loaded, calling setAvatar at row" << i;
                                model->setAvatar(i, px);
                            }else{
                                 qDebug() << "Failed to decode pixmap";
                            }
                        }
                        reply->deleteLater();
                    });
                }
                reply->deleteLater();
            });
}

void Dashboard::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    searchDebounceTimer->start(300); // Gõ phím liên tục sẽ kéo lại cót 300ms, ngừng gõ mới kích hoạt tìm kiếm
}

void Dashboard::onFriendSelected(QModelIndex proxyIndex)
{
    qDebug() << "1";
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    FriendInfo selectedFriend = model->getFriendAt(sourceIndex.row());

    qDebug() << "2";

    qDebug()
        << selectedFriend.friendId
        << selectedFriend.displayName
        << selectedFriend.avatarUrl;

    qDebug() << "3";
    model->resetUnreadCount(selectedFriend.friendId);
    model->updateFriendInfo(selectedFriend.friendId, selectedFriend.lastMsgTime, 0);
    emit openChatRequest(selectedFriend);

    qDebug() << "4";
}
void Dashboard::triggerSearch(){
    QString keyword = searchFriend->text().trimmed();
    //QSettings settings("BKAV", "ChatApp");
    /*
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    QString baseUrl = AppConfig::instance().getBaseUrl();
    QUrl url( baseUrl + "/user/search");
    QUrlQuery query;
    query.addQueryItem("keyword", keyword);
    if(keyword.isEmpty())
    {
        loadFriendList();
        return;
    }
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            {
                QByteArray response = reply->readAll();
                qDebug() << response;
                if(reply->error())
                {
                    qDebug() << reply->errorString();
                    reply->deleteLater();
                    return;
                }
                QJsonDocument doc = QJsonDocument::fromJson(response);
                QJsonObject obj = doc.object();
                if(obj["status"].toInt() != 1)
                {
                    qDebug() << obj["message"].toString();
                    reply->deleteLater();
                    return;
                }
                QJsonArray users = obj["data"].toArray();
                QVector<FriendInfo> list;
                for(const auto &value : as_const(users))
                {
                    QJsonObject user = value.toObject();
                     qDebug() << "SEARCH USER =" << user;
                    qDebug() << "user_id =" << user["user_id"].toVariant().toLongLong();
                    list.append(FriendInfo(QString::number(user["user_id"].toVariant().toLongLong()),user["display_name"].toString(), user["avatar_path"].toString(),false, 0, QDateTime::currentDateTime()));
                }
                qDebug() << "SEARCH LIST SIZE =" << list.size();
                model->setFriendList(list);
                reply->deleteLater();
            });*/
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRegularExpression(keyword);
}

void Dashboard::onAvatarClicked()
{
    // Tạo menu
    QMenu *chatMenu = new QMenu(this);
    QAction *actionChangeAvatar = chatMenu->addAction("Thay ảnh đại diện");
    QAction *actionLogout = chatMenu->addAction("Đăng xuất");

    // Áp dụng QSS để đổi con trỏ chuột thành hình bàn tay
    chatMenu->setStyleSheet(
        "QMenu {"
        "   background-color: #ffffff;"
        "   border: 1px solid #dcdcdc;"
        "   border-radius: 8px;"
        "   padding: 4px;"
        "}"
        "QMenu::item {"
        "   padding: 8px 32px 8px 12px;"
        "   background-color: transparent;"
        "   cursor: pointing-hand;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #F5F5F5;"
        "   color: #000000;"
        "}"
        );

    // Hiển thị menu tại vị trí con trỏ chuột
    connect(actionChangeAvatar, &QAction::triggered, this, &Dashboard::changeAvatar);
    connect(actionLogout, &QAction::triggered, this, &Dashboard::logOut);
    chatMenu->exec(QCursor::pos());
}

void Dashboard::changeAvatar()
{
    QString fileName =
        QFileDialog::getOpenFileName(
            this,
            "Select Avatar",
            QString(),
            "Images (*.png *.jpg *.jpeg)");

    if(fileName.isEmpty())
        return;

    QFile *file = new QFile(fileName);

    if(!file->open(QIODevice::ReadOnly))
    {
        delete file;
        return;
    }
    //QSettings settings("BKAV", "ChatApp");
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    QString baseUrl = AppConfig::instance().getBaseUrl();
    QHttpMultiPart *multiPart =
        new QHttpMultiPart(
            QHttpMultiPart::FormDataType);
    QHttpPart avatarPart;
    avatarPart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant(
            QString(
                "form-data; "
                "name=\"avatar\"; "
                "filename=\"%1\"")
                .arg(
                    QFileInfo(
                        fileName)
                        .fileName()
                    )
            )
        );

    avatarPart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(avatarPart);
    QNetworkRequest request(QUrl(baseUrl + "/user/update"));
    request.setRawHeader("Authorization",("Bearer " + token).toUtf8());
    QNetworkReply *reply = networkManager->post(request,multiPart);
    multiPart->setParent(reply);
    connect(reply, &QNetworkReply::finished, this, &Dashboard::onAvartaUploadFinished);
}
void Dashboard::onAvartaUploadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply)
        return;
    QByteArray response =
        reply->readAll();
    if(reply->error())
    {
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }
    QJsonObject obj =
        QJsonDocument::fromJson(
            response)
            .object();
    if(obj["status"].toInt() == 1)
    {
        QMessageBox::information(
            this,
            "Success",
            "Avatar updated");
        loadCurrentUser();
    }
    else
    {
        QMessageBox::warning(
            this,
            "Error",
            obj["message"]
                .toString());
    }
    reply->deleteLater();
}

void Dashboard::logOut(){
    //QSettings settings("BKAV","ChatApp");
    QString configPath = AppConfig::instance().getConfigFilePath();
    QSettings settings(configPath, QSettings::IniFormat);
    SocketManager::instance().unregisterUser(myId);
    settings.remove("auth");
    emit logOutRequest();
}
void Dashboard::loadAvatarFromServer(const QString &avatarPath)
{
    QString baseUrl = AppConfig::instance().getBaseUrl();
    baseUrl.chop(4);
    QUrl url(baseUrl + avatarPath);
    qDebug() << baseUrl + avatarPath;
    QNetworkReply *reply = networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            {
        if(reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Avatar error:"
                     << reply->errorString();

            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();

        QPixmap pixmap;

        if(!pixmap.loadFromData(data))
        {
            qDebug() << "Cannot decode avatar image";
            reply->deleteLater();
            return;
        }

        QPixmap rounded(60, 60);
        rounded.fill(Qt::transparent);
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, 60, 60);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, 60, 60, pixmap.scaled(60, 60,
                                                       Qt::KeepAspectRatioByExpanding,
                                                       Qt::SmoothTransformation));
        avatarButton->setIcon(QIcon(rounded));
        avatarButton->setIconSize(avatarButton->size());

        qDebug() << "Avatar loaded";

        reply->deleteLater();
            });
}

void Dashboard::onNewMessageReceived(const QString &message)
{
    qDebug() << "RAW PACKET:" << message;

    QJsonObject obj = QJsonDocument::fromJson(message.toUtf8()).object();
    QString type = obj["type"].toString();
    if (type == "presence") {
        QString userId = QString::number(obj["userId"].toVariant().toLongLong());
        bool isOnline = obj["isOnline"].toBool();
        qDebug() << "presence userId:" << userId << "isOnline:" << isOnline;
        model->updateFriendStatus(userId, isOnline);
        return;
    }

    // Tin nhắn thường
    if (!obj.contains("from")) return;
    QString fromId = QString::number(obj["from"].toVariant().toLongLong());
    model->updateFriendInfo(fromId, QDateTime::currentDateTime(), 1);
}

qint64 Dashboard::getMyId()
{
    return myId;
}
Dashboard::~Dashboard()
{
    disconnect(&SocketManager::instance(), nullptr, this, nullptr);
}

