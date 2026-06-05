#include "dashboard.h"
#include "dashboardmodel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QFile>
#include <QTimer>

Dashboard::Dashboard(DashboardModel *model, QWidget *parent)
    : QWidget(parent), model(model)
{
    this->setWindowTitle("BKav Chat");
    this->setFixedSize(550, 700);
    this->setStyleSheet("background-color: white;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    title = new QLabel("Bkav Chat", this);
    title->setStyleSheet("color: blue; font-size: 24px; font-weight: bold;");
    title->setAlignment(Qt::AlignLeft);

    avatarButton = new QPushButton(this);
    avatarButton->setFixedSize(46, 46);
    avatarButton->setStyleSheet("QPushButton{""border-radius: 23px;""}");
    avatarButton->setCursor(Qt::PointingHandCursor);

    displayName = new QLabel(this);
    displayName->setStyleSheet("color: #000000; font-size: 12px; font-weight: bold;");

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
    friendListView->setStyleSheet("QListView { border: none; background: transparent; }");

    model = new DashboardModel(this);
    searchDebounceTimer = new QTimer(this);

    friendListView->setModel(model);
    searchDebounceTimer->setSingleShot(true);

    headerLayout = new QHBoxLayout();
    userProfile = new QVBoxLayout();
    userProfile->setSpacing(4);
    userProfile->addWidget(avatarButton, Qt::AlignCenter);
    userProfile->addWidget(displayName, Qt::AlignCenter);

    headerLayout->addWidget(title, Qt::AlignLeft);
    headerLayout->addStretch();
    headerLayout->addLayout(userProfile);

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(searchFriend);
    mainLayout->addWidget(titleList);
    mainLayout->addWidget(friendListView);

    initUserCache();
    loadFriendList();

    connect(avatarButton, &QPushButton::clicked, this, &Dashboard::onAvatarClicked);
    connect(searchDebounceTimer, &QTimer::timeout, this, &Dashboard::triggerSearch);
    connect(friendListView, &QListView::clicked, this, &Dashboard::onFriendSelected);
    connect(searchFriend, &QLineEdit::textChanged, this, &Dashboard::onSearchTextChanged);
}

void Dashboard::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    searchDebounceTimer->start(300); // Gõ phím liên tục sẽ kéo lại cót 300ms, ngừng gõ mới kích hoạt tìm kiếm
}

void onFriendSelected(){
    //Về sau sẽ xử lý đoạn mở khung chat
    return;
}

void Dashboard::triggerSearch(){
    QString keyword = searchFriend->text().trimmed();
    QSqlQuery query;

    if(keyword.isEmpty()){
        query.prepare("select u.user_id, u.display_name, u.avatar_path"
                    "from users u"
                      "where u.user_id != :my_id");
    }else{
        query.prepare("select u.user_id, u.display_name, u.avatar_path"
                      "from users u"
                      "where u.user_id != :my_id and u.display_name like :keyword");
        query.bindValue(":keyword", "%" + keyword + "%");
    }
    query.bindValue(":my_id", myId);
    if(query.exec()){
        model->clear();
        while(query.next()){
            QString friendId = query.value(0).toString();
            QString friendName = query.value(1).toString();
            QString friendAvatarPath = query.value(2).toString();

            model->friends.append(FriendInfo(friendId, friendName, friendAvatarPath, false));
            model->rowMap[friendId] = model->friends.size() - 1;
        }
    }else{
        qDebug() << "Lỗi tìm kiếm bạn bè:" << query.lastError().text();
    }
}

void onAvatarClicked(){
    //Về sau xử lý việc thay ảnh đại diện
    return;
}

void Dashboard::initUserCache(){
    QSqlQuery query;
    query.prepare("select u.username, u.display_name, u.avatar_path"
                  "from users u"
                  "where u.user_id = :my_id");
    query.bindValue(":my_id", myId);
    if(query.exec() && query.next()){
        displayName->setText(query.value(1).toString());
        QString avatarPath = query.value(2).toString();
        if(!avatarPath.isEmpty() && QFile::exists(avatarPath)){
            avatarButton->setStyleSheet(
                QString(
                    "QPushButton {"
                    "background-image: url(%1);"
                    "background-position: center;"
                    "background-repeat: no-repeat;"
                    "}"
                    "QPushButton:hover {"
                    "   opacity: 0.85;"
                    "}"
                    ).arg(avatarPath)
                );
        }else{
            avatarButton->setStyleSheet(
                "QPushButton {"
                    "background-color: light-blue;""}"
                );
            avatarButton->setText(displayName->text().left(1).toUpper());
        }
        qDebug() << "Nạp cache thành công: "<< query.lastError().text();
    }else{
        qDebug() << "Không tìm thấy thông tin trong database: "<< query.lastError().text();
    }

    return;
}

void Dashboard::loadFriendList(){
    QSqlQuery query;
    query.prepare("select u.user_id, u.display_name, u.avatar_path"
                  "from users u"
                  "where u.users_id != :my_id");
    query.bindValue("my_id", myId);
    if(query.exec()){
        model->clear();
        while(query.next()){
            QString friendId = query.value(0).toString();
            QString friendName = query.value(1).toString();
            QString friendAvatarPath = query.value(2).toString();

            model->friends.append(FriendInfo(friendId, friendName, friendAvatarPath, false));
            model->rowMap[friendId] = model->friends.size() - 1;
        }
    }else{
        qDebug() << "Lỗi truy vấn danh sách người dùng: "<< query.lastError().text();
    }
    return;
}