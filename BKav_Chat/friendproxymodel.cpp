#include "friendproxymodel.h"
#include "dashboardmodel.h" // Để biết các enum roles

FriendProxyModel::FriendProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    // Bật tính năng tự động sắp xếp khi dữ liệu trong model gốc thay đổi
    setDynamicSortFilter(true);
}

bool FriendProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    // Lấy thời gian của tin nhắn cuối cùng từ model gốc
    QDateTime leftTime = sourceModel()->data(left, DashboardModel::LastMsgTimeRole).toDateTime();
    QDateTime rightTime = sourceModel()->data(right, DashboardModel::LastMsgTimeRole).toDateTime();

    if (!leftTime.isValid()) leftTime = QDateTime::fromMSecsSinceEpoch(0);
    if (!rightTime.isValid()) rightTime = QDateTime::fromMSecsSinceEpoch(0);
    // Nếu muốn người mới nhất lên đầu, thì thời gian của 'left' phải lớn hơn 'right'
    return leftTime > rightTime;
}