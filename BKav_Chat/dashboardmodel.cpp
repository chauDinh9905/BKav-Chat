#include "dashboardmodel.h"
#include "avatarcache.h"
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

DashboardModel::DashboardModel(QObject *parent, const QString &userId)
    :QAbstractListModel(parent), m_userId(userId){
    connect(&AvatarCache::instance(), &AvatarCache::avatarLoaded,
            this, [this](const QString &url, const QPixmap &px) {
        if (avatarRowMap.contains(url)) {
            setAvatar(avatarRowMap[url], px);
        }
            });
}

void DashboardModel::setFriendList(const QVector<FriendInfo> &newList){
    qDebug() << "setFriendList:" << newList.size();
    beginResetModel();
    friends = newList;
    rowMap.clear();
    for(int i = 0; i < friends.size(); ++i) {
        rowMap[friends[i].friendId] = i;
        avatarRowMap[friends[i].avatarUrl] = i;
    }
    friendAvatars.clear();
    friendAvatars.resize(friends.size());
    endResetModel();
}

FriendInfo DashboardModel::getFriendAt(int row){
    return friends.at(row);
}

void DashboardModel::updateFriendStatus(const QString &friendId, bool isOnline){
    int row = rowMap[friendId];
    friends[row].isOnline = isOnline;
    emit dataChanged(
        index(row),
        index(row));
}

int DashboardModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //qDebug() << "rowCount =" << friends.size();
    return friends.size();
}

void DashboardModel::clear(){
    beginResetModel();
    friends.clear();
    endResetModel();
}

QVariant DashboardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
/*
    if (role == Qt::DisplayRole)
        return friends[index.row()].displayName;
*/
    const auto &friendInfo = friends[index.row()];

    if (role == Qt::DecorationRole) {
        QPixmap baseAvatar;
        if (index.row() < friendAvatars.size() && !friendAvatars[index.row()].isNull()) {
            baseAvatar = friendAvatars[index.row()];
        } else {
            // Dùng ảnh mặc định nếu chưa load xong
            baseAvatar = QPixmap(50, 50);
            baseAvatar.fill(Qt::lightGray);
        }
        QPixmap combined(50, 50);
        combined.fill(Qt::transparent);

        QPainter painter(&combined);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addEllipse(0, 0, 50, 50);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, 50, 50, baseAvatar);
        painter.setClipping(false);
        // Vẽ chấm Online/Offline
        painter.setBrush(friendInfo.isOnline ? Qt::green : Qt::gray);
        painter.drawEllipse(35, 35, 12, 12);

        // Vẽ Badge số thông báo
        if (friendInfo.unreadCount > 0) {
            painter.setBrush(Qt::red);
            painter.drawEllipse(35, 0, 15, 15);
            painter.setPen(Qt::white);
            painter.drawText(35, 0, 15, 15, Qt::AlignCenter, QString::number(friendInfo.unreadCount));
        }
        return combined;
    }

    if (role == Qt::DisplayRole) {
        return friendInfo.displayName;
    }
    return QVariant();
}
void DashboardModel::setAvatar(int row, const QPixmap &avatar) {
    if (row >= 0 && row < friendAvatars.size()) {
        friendAvatars[row] = avatar;

        //hông báo cho QListView biết ảnh tại hàng 'row' đã có
        // Điều này kích hoạt việc vẽ lại (repaint) riêng hàng đó
        emit dataChanged(index(row), index(row), {Qt::DecorationRole});
    }
}
void DashboardModel::updateFriendInfo(const QString &friendId, const QDateTime &newTime, int unreadCountIncrement) {
    //  Kiểm tra xem người dùng có tồn tại trong danh sách không
    if (!rowMap.contains(friendId)) {
        qWarning() << "Friend not found in model:" << friendId;
        return;
    }

    //  Lấy vị trí dòng (row) trực tiếp từ rowMap (tốc độ O(1))
    int row = rowMap.value(friendId);

    //  Cập nhật dữ liệu
    friends[row].lastMsgTime = newTime;
    friends[row].unreadCount += unreadCountIncrement;

    //  Báo cho View (ProxyModel sẽ bắt được tín hiệu này)
    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, {LastMsgTimeRole, UnreadCountRole});
}
void DashboardModel::resetUnreadCount(const QString &friendId)
{
    if (!rowMap.contains(friendId)) return;
    int row = rowMap.value(friendId);
    friends[row].unreadCount = 0;
    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, {UnreadCountRole});
}

qint64 DashboardModel::getMyId(){
    return m_userId.toLongLong();
}
