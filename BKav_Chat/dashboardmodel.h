#ifndef DASHBOARDMODEL_H
#define DASHBOARDMODEL_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QAbstractListModel>

struct FriendInfo{
    QString friendId;
    QString displayName;
    QString avatarUrl;
    bool isOnline;
    FriendInfo(const QString &userId,
               const QString &displayName,
               const QString &avatarUrl,
               bool isOnline)
        : friendId(userId),
        displayName(displayName),
        avatarUrl(avatarUrl),
        isOnline(isOnline)
    {
    }
};
Q_DECLARE_METATYPE(FriendInfo)

class DashboardModel:public  QAbstractListModel{
    Q_OBJECT
public:
    explicit DashboardModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    //thông tin dành cho mỗi một ng trong danh sách bạn bè
    enum FriendRoles{
        FriendIdRole = Qt::UserRole + 1,
        DisplayNameRole,
        AvatarUrlRole,
        IsOnlineRole
    };

    void setFriendList(const QVector<FriendInfo> &newList);
    void updateFriendStatus(const QString &friendId, bool isOnline);
    FriendInfo getFriendAt(int row); // const cho biết rằng hàm này không thay đỏi trạng của friend hiện tại mà nó truy xuất được
    void clear();

    QVector<FriendInfo> friends;
    QHash<QString, int> rowMap;
};


#endif // DASHBOARDMODEL_H
