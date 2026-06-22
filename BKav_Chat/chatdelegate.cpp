#include "chatdelegate.h"
#include "chatmodel.h"

ChatDelegate::ChatDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString content = index.data(ChatModel::ContentRole).toString();
    bool isMine = index.data(ChatModel::IsMineRole).toBool();

    painter->save();

    int padding = 10;
    int maxWidth = option.rect.width() * 0.65;

    QFont font = option.font;
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(
        QRect(0, 0, maxWidth - 2 * padding, 0),
        Qt::TextWordWrap,
        content
        );

    int bubbleW = textRect.width() + 2 * padding;
    int bubbleH = textRect.height() + 2 * padding;

    int x = isMine
                ? option.rect.right() - bubbleW - 10
                : option.rect.left() + 10;
    int y = option.rect.top() + 5;

    QRect bubbleRect(x, y, bubbleW, bubbleH);

    // Vẽ bong bóng
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(isMine ? QColor("#DCF8C6") : QColor("#FFFFFF"));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, 10, 10);

    // Vẽ text
    painter->setPen(Qt::black);
    painter->setFont(font);
    painter->drawText(
        bubbleRect.adjusted(padding, padding, -padding, -padding),
        Qt::TextWordWrap,
        content
        );

    painter->restore();
}

QSize ChatDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString content = index.data(ChatModel::ContentRole).toString();
    int maxWidth = 400;
    int padding = 10;

    QFontMetrics fm(option.font);
    QRect textRect = fm.boundingRect(
        QRect(0, 0, maxWidth - 2 * padding, 0),
        Qt::TextWordWrap,
        content
        );

    return QSize(option.rect.width(), textRect.height() + 2 * padding + 10);
}