#ifndef FRIENDPROXYMODEL_H
#define FRIENDPROXYMODEL_H
#include <QSortFilterProxyModel>

class FriendProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit FriendProxyModel(QObject *parent = nullptr);

protected:
    // Quyết định thứ tự hiển thị: trả về true nếu left nằm trước right
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    // (Tùy chọn) Nếu bạn muốn thêm tính năng lọc (ví dụ: tìm kiếm bạn bè)
     bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
#endif // FRIENDPROXYMODEL_H
