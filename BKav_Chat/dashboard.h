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

class DashboardModel;

class Dashboard:public QWidget{
    Q_OBJECT
public:
    explicit Dashboard(DashboardModel *model, QWidget *parent = nullptr);
    ~Dashboard();

private slots:
    void onAvatarClicked(); // khi có signal người dùng ấn vào avatar của mình
    void onAvartaUploadFinished(); // khi có signal từ server báo rằng ng dùng đã load ảnh đại diện mới lên xong
    void onSearchTextChanged(const QString &text); // khi có signal người dùng điền ký tự vào ô tìm kiếm
    void triggerSearch(); // đếm ngược mỗi khi người dùng dừng điền kí tự trên ô tìm kiếm, khi hàm này kết thúc thì những ký tự sẽ được gửi lên server
    void onFriendSelected(const QModelIndex &index);// khi có signal người dùng ấn vào một người trong danh sách bạn bè

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

    QString friendChatId;
    int myId;

    QHBoxLayout headerLayout;
    QVBoxLayout mainLayout, userProfile;

    QByteArray convertPixmapToByteArray(const QPixmap &pixmap);
    void loadAvatarFromServer(const QString &avatarUrl);
};


#endif // DASHBOARD_H
