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
#include "databasemanager.h"
#include <QFileDialog>
#include <QPixmap>
#include <QIcon>
#include <QSqlQuery>
#include <QSqlError>

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

    this->model = model;
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
    QSqlQuery query(DatabaseManager::instance().getDatabase());

    if(keyword.isEmpty()){
        query.prepare("select u.user_id, u.display_name, u.avatar_path "
                    "from users u "
                      "where u.user_id != :my_id");
    }else{
        query.prepare("select u.user_id, u.display_name, u.avatar_path "
                      "from users u "
                      "where u.user_id != :my_id and u.display_name like :keyword ");
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
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("select u.username, u.display_name, u.avatar_path "
                  "from users u "
                  "where u.user_id = :my_id");
    //qDebug() << "prepare =" << ok;
    qDebug() << "prepare error =" << query.lastError().text();
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
                    "background-color: grey;""}"
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
    QSqlQuery query(DatabaseManager::instance().getDatabase());

    bool ok = query.prepare(
        "SELECT u.username, u.display_name, u.avatar_path "
        "FROM users u "
        "WHERE u.user_id = ?"
        );

    qDebug() << "prepare =" << ok;
    qDebug() << "prepare error =" << query.lastError().text();

    query.addBindValue(myId);

    ok = query.exec();

    qDebug() << "exec =" << ok;
    qDebug() << "exec error =" << query.lastError().text();
    if(ok){
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

void Dashboard::setCurrentUser(int userId)
{
    myId = userId;

    initUserCache();
    loadFriendList();
}

void Dashboard::onAvatarClicked()
{
    qDebug() << "Nút avatar được nhấn, đang mở thư mục chọn ảnh...";

    // 1. Mở hộp thoại chọn file ảnh từ máy tính
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Chọn ảnh đại diện"),                 // Tiêu đề cửa sổ
        QDir::homePath(),                      // Thư mục mặc định mở ra (Thư mục Home của Ubuntu)
        tr("Hình ảnh (*.png *.jpg *.jpeg)")     // Bộ lọc chỉ cho phép chọn các định dạng ảnh này
        );

    // 2. Kiểm tra xem người dùng có thực sự chọn file hay bấm "Cancel"
    if (!filePath.isEmpty()) {
        qDebug() << "Đường dẫn ảnh đã chọn:" << filePath;

        // 3. Cập nhật ngay lập tức ảnh vừa chọn lên nút QPushButton làm background cục bộ
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            // Bo tròn hoặc scale ảnh cho vừa khít với kích thước của nút bấm
            QIcon buttonIcon(pixmap.scaled(avatarButton->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            avatarButton->setIcon(buttonIcon);
            avatarButton->setIconSize(avatarButton->size()); // Đảm bảo icon chiếm trọn nút

            // Nếu bạn đang dùng StyleSheet để làm background, có thể đổi bằng dòng này thay thế:
            avatarButton->setStyleSheet(QString("border-image: url(%1); border-radius: 25px;").arg(filePath));

            QSqlQuery query(DatabaseManager::instance().getDatabase());
            query.prepare("UPDATE users SET avatar_path = :path WHERE user_id = :id");
            query.bindValue(":path", filePath);
            query.bindValue(":id", myId); // myId là id của bạn hiện tại

            if (!query.exec()) {
                qDebug() << "Lưu đường dẫn avatar thất bại:" << query.lastError().text();
            } else {
                qDebug() << "Đã cập nhật đường dẫn avatar vào DB!";
            }
        }

        //  Gửi file ảnh này lên server hoặc xử lý tiếp sang hàm upload
        // loadAvatarFromServer(filePath);
    } else {
        qDebug() << "Người dùng đã hủy chọn file.";
    }
}

void Dashboard::onAvartaUploadFinished(){

}

void Dashboard::onFriendSelected(const QModelIndex &index){

}

Dashboard::~Dashboard()
{
}

