#ifndef CHATDELEGATE_H
#define CHATDELEGATE_H
#include <QStyledItemDelegate>
#include <QPainter>

class ChatDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
signals:
    void fileClicked(const QString &url, const QString &fileName);
    void imageClicked(const QString &url);
private:
    QString imageBaseUrl() const;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};
#endif