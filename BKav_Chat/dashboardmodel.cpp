#include "dashboardmodel.h"

DashboardModel::DashboardModel(QObject *parent)
    :QObject(parent){}

void DashboardModel::setFriendList(const QVector<FriendInfo> &newList){
    friends = newList;
}

void DashboardModel::getFriendAt(int row){
    return friends.at(row);
}

void DashboardModel::updateFriendStatus(const QString &friendId, bool isOnline){
    int row = rowMap[friendId];
    friends.at(row).isOnline = isOnline;
}