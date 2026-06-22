#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QPushButton>
#include <QLineEdit>
#include <QListView>
#include <QLabel>
#include <QWebSocket>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include "dashboardmodel.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

class DashboardModel;

class Dashboard:public QWidget{
    Q_OBJECT
public:
    explicit Dashboard(DashboardModel *model, QWidget *parent = nullptr);
    ~Dashboard();

signals:
    void logOutRequest();
    void openChatRequest(const FriendInfo &friendInfo);
private slots:
    void onAvatarClicked(); // khi có signal người dùng ấn vào avatar của mình
    void onAvartaUploadFinished(); // khi có signal từ server báo rằng ng dùng đã load ảnh đại diện mới lên xong
    void onSearchTextChanged(const QString &text); // khi có signal người dùng điền ký tự vào ô tìm kiếm
    void triggerSearch(); // đếm ngược mỗi khi người dùng dừng điền kí tự trên ô tìm kiếm, khi hàm này kết thúc thì những ký tự sẽ được gửi lên server
    void onFriendSelected(QModelIndex index);// khi có signal người dùng ấn vào một người trong danh sách bạn bè
    void changeAvatar();
    void logOut();

private:
    DashboardModel *model;

    QPushButton *avatarButton;
    QLineEdit *searchFriend;
    QListView *friendListView;
    QLabel *title;
    QLabel *titleList;
    QLabel *displayName;

    QTimer *searchDebounceTimer;
    QWebSocket *activeSocket;
    QNetworkAccessManager *networkManager;

    QString *friendChatId;
    qint64 myId;

    QHBoxLayout *headerLayout;
    QVBoxLayout *mainLayout, *userProfile;

    QByteArray convertPixmapToByteArray(const QPixmap &pixmap);
    void loadAvatarFromServer(const QString &avatarUrl);

public:
    void loadFriendList();
    void loadCurrentUser();
    void loadAvatarFromBase64(const QString &base64String);
    FriendInfo friendAt(int row);
    qint64 getMyId();
};


#endif // DASHBOARD_H
