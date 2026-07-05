#ifndef DASHBOARDMODEL_H
#define DASHBOARDMODEL_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QAbstractListModel>
#include <QDateTime>

struct FriendInfo{
    QString friendId;
    QString displayName;
    QString avatarUrl;
    bool isOnline;
    int unreadCount;
    QDateTime lastMsgTime;
    FriendInfo(const QString &userId,
               const QString &displayName,
               const QString &avatarUrl,
               bool isOnline,
               int unreadCount,
               QDateTime lastMsgTime)
        : friendId(userId),
        displayName(displayName),
        avatarUrl(avatarUrl),
        isOnline(isOnline),
        unreadCount(unreadCount),
        lastMsgTime(lastMsgTime)
    {

    }
};

Q_DECLARE_METATYPE(FriendInfo)

class DashboardModel:public  QAbstractListModel{
    Q_OBJECT
public:
    explicit DashboardModel(QObject *parent = nullptr, const QString &userId = QString());
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    //thông tin dành cho mỗi một ng trong danh sách bạn bè
    enum FriendRoles{
        FriendIdRole = Qt::UserRole + 1,
        DisplayNameRole,
        AvatarUrlRole,
        IsOnlineRole,
        LastMsgTimeRole, // Dùng để ProxyModel sắp xếp
        UnreadCountRole  // Dùng để hiện Badge số tin nhắn
    };

    void setFriendList(const QVector<FriendInfo> &newList);
    void updateFriendStatus(const QString &friendId, bool isOnline);
    void updateFriendInfo(const QString &friendId, const QDateTime &newTime, int unreadCountIncrement);
    FriendInfo getFriendAt(int row); // const cho biết rằng hàm này không thay đỏi trạng của friend hiện tại mà nó truy xuất được
    void clear();

    QVector<FriendInfo> friends;
    QVector<QPixmap> friendAvatars;
    QHash<QString, int> rowMap;
    QHash<QString, int> avatarRowMap;
    void setAvatar(int row, const QPixmap &avatar);
    void resetUnreadCount(const QString &friendId);
    qint64 getMyId();
private:
    QString m_userId;
};


#endif // DASHBOARDMODEL_H
