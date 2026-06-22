#include "dashboardmodel.h"
#include <QDebug>
DashboardModel::DashboardModel(QObject *parent)
    :QAbstractListModel(parent){}

void DashboardModel::setFriendList(const QVector<FriendInfo> &newList){
    qDebug() << "setFriendList:" << newList.size();
    beginResetModel();
    friends = newList;
    rowMap.clear();
    for(int i = 0; i < friends.size(); ++i) {
        rowMap[friends[i].friendId] = i; // Giả sử FriendInfo có trường friendId kiểu QString
    }
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

    if (role == Qt::DisplayRole)
        return friends[index.row()].displayName;

    return QVariant();
}