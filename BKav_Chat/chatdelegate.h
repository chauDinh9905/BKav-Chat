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
    void setFriendAvatarUrl(const QString &url);
signals:
    void fileClicked(const QString &url, const QString &fileName);
    void imageClicked(const QString &url);
private:
    QSizeF wrappedTextSize(const QString &text, const QFont &font, qreal maxWidth) const;
    QString imageBaseUrl() const;
    bool editorEvent(QEvent *event,QAbstractItemModel *model,const QStyleOptionViewItem &option,const QModelIndex &index) override;
    bool isLastMineMessage(const QModelIndex &index) const;
    bool isLastInFriendGroup(const QModelIndex &index) const;
    QString m_friendAvatarUrl;
    static const int avatarGutter = 34;
    static const int tickGutter = 26;
};
#endif