#ifndef DASHBOARDMODEL_H
#define DASHBOARDMODEL_H

#include <QObject>
#include <QVector>
#include <QHash>

struct FriendInfo{
    QString friendId;
    QString displayName;
    QString avatarUrl;
    bool isOnline;
};

class DashboardModel:public QObject{
    Q_OBJECT
public:
    explicit DashboardModel(QObject *parent = nullptr);

    //thông tin dành cho mỗi một ng trong danh sách bạn bè
    enum FriendRoles{
        FriendIdRole = Qt::UserRole + 1,
        DisplayNameRole,
        AvatarUrlRole,
        IsOnlineRole
    };

    void setFriendList(const QVector<FriendInfo> &newList);
    void updateFriendStatus(const QString &friendId, bool isOnline);
    FriendInfo getFriendAt(int row) const; // const cho biết rằng hàm này không thay đỏi trạng của friend hiện tại mà nó truy xuất được
private:
    QVector<FriendInfo> friends;
    QHash<QString, int> rowMap;
};


#endif // DASHBOARDMODEL_H
