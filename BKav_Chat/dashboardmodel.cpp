#include "dashboardmodel.h"

DashboardModel::DashboardModel(QObject *parent)
    :QAbstractListModel(parent){}

void DashboardModel::setFriendList(const QVector<FriendInfo> &newList){
    friends = newList;
}

FriendInfo DashboardModel::getFriendAt(int row){
    return friends.at(row);
}

void DashboardModel::updateFriendStatus(const QString &friendId, bool isOnline){
    int row = rowMap[friendId];
    friends[row].isOnline = isOnline;
}

int DashboardModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
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

    if (role == Qt::DisplayRole)
        return friends[index.row()].displayName;

    return QVariant();
}