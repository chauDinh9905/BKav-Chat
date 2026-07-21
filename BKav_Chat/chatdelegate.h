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
    void setFileProgress(const QString &fileUrl, int percent); // 0-99 = đang tải, -1 = xong/reset
    int fileProgress(const QString &fileUrl) const;
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
    bool shouldShowTimeSeparator(const QModelIndex &index) const;
    QString formatSeparatorTime(const QDateTime &dt) const;
    QString m_friendAvatarUrl;
    QHash<QString, int> m_fileProgress;
    static const int avatarGutter = 34;
    static const int tickGutter = 26;
    static const int separatorHeight = 26;
};
#endif