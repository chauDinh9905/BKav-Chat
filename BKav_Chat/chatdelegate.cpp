#include "chatdelegate.h"
#include "chatmodel.h"
#include "imagecache.h"
#include "appconfig.h"
#include <QMouseEvent>
#include <QTextLayout>
#include <QTextEdit>
#include <QtMath>
#include <QPainterPath>
#include <QPixmap>
#include <QDate>

ChatDelegate::ChatDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
    connect(&ImageCache::instance(), &ImageCache::imageReady, this, [this]() {
        emit const_cast<ChatDelegate*>(this)->sizeHintChanged(QModelIndex());
    });
}

QString ChatDelegate::imageBaseUrl() const
{
    return AppConfig::instance().getBaseUrl();
}
static QFont emojiCapableFont(const QFont &base)
{
    QFont f = base;
#if defined(Q_OS_WIN)
    f.setFamilies({ base.family(), "Segoe UI Emoji" });
#elif defined(Q_OS_MACOS)
    f.setFamilies({ base.family(), "Apple Color Emoji" });
#else
    f.setFamilies({ base.family(), "Noto Color Emoji" });
#endif
    return f;
}
void ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString content = index.data(ChatModel::ContentRole).toString();
    bool isMine = index.data(ChatModel::IsMineRole).toBool();
    QVector<ImageInfo> images = index.data(ChatModel::ImagesRole).value<QVector<ImageInfo>>();
    QVector<FileInfo> files = index.data(ChatModel::FilesRole).value<QVector<FileInfo>>();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    bool showSep = shouldShowTimeSeparator(index);
    int sepH = showSep ? separatorHeight : 0;
    int padding = 10;
    int maxWidth = option.rect.width() * 0.65;
    int thumbSize = 120;
    int spacing = 6;

    QFont font = emojiCapableFont(option.font);
    QFontMetrics fm(font);
    // Tính chiều cao phần text (nếu có nội dung)
    QSizeF textSize;
    if(!content.isEmpty()){
        textSize = wrappedTextSize(content, font, maxWidth - 2*padding);
    }
    // Tính layout lưới ảnh (tối đa 3 ảnh/dòng, giống Messenger)
    int imgCols = qMin(3, qMax(1, images.size()));
    int imgRows = images.isEmpty() ? 0 : (images.size() + imgCols - 1) / imgCols;
    int imagesBlockW = images.isEmpty() ? 0 : imgCols * thumbSize + (imgCols - 1) * spacing;
    int imagesBlockH = images.isEmpty() ? 0 : imgRows * thumbSize + (imgRows - 1) * spacing;

    //Tính chiều cao phần file chip (mỗi file 1 dòng)
    const int fileChipH = 36;
    int filesBlockH = files.isEmpty() ? 0 : files.size() * (fileChipH + 4);

    int bubbleW = qMax(qCeil(textSize.width()), imagesBlockW) + 2 * padding;
    bubbleW = qMax(bubbleW, files.isEmpty() ? 0 : maxWidth); // file chip rộng theo maxWidth cho dễ đọc tên
    int bubbleH = 2 * padding
                  + (content.isEmpty() ? 0 : qCeil(textSize.height()) + spacing)
                  + imagesBlockH + (images.isEmpty() ? 0 : spacing)
                  + filesBlockH;

    int x = isMine ? option.rect.right() - bubbleW - 10 : option.rect.left() + 10 + avatarGutter;
    int y = option.rect.top() + 5 + sepH;
    QRect bubbleRect(x, y, bubbleW, bubbleH);
    if(showSep){
        QDateTime msgTime = index.data(ChatModel::CreateAtRole).toDateTime();
        QString sepText = formatSeparatorTime(msgTime);
        painter->setPen(Qt::black);
        QFont sepFont = font;
        sepFont.setPointSize(qMax(8, font.pointSize() - 1));
        painter->setFont(sepFont);
        QRect sepRect(option.rect.left(), option.rect.top(), option.rect.width(), separatorHeight);
        painter->drawText(sepRect, Qt::AlignCenter, sepText);
    }

    // Nếu chỉ có ảnh (không text, không file) -> bong bóng trong suốt, chỉ viền nhẹ (giống Messenger)
    bool onlyImages = content.isEmpty() && files.isEmpty() && !images.isEmpty();
    if (!onlyImages) {
        painter->setBrush(isMine ? QColor("#DCF8C6") : QColor("#FFFFFF"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(bubbleRect, 10, 10);
    }

    int curY = bubbleRect.top() + padding;

    // Vẽ text
    if (!content.isEmpty()) {
        painter->setPen(Qt::black);
        painter->setFont(font);
        QTextLayout drawLayout(content, font);
        QTextOption opt;
        opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        drawLayout.setTextOption(opt);
        drawLayout.beginLayout();
        qreal lineY = 0;
        while(true){
            QTextLine line = drawLayout.createLine();
            if (!line.isValid()) break;
            line.setLineWidth(bubbleRect.width() - 2 * padding);
            line.setPosition(QPointF(0, lineY));
            lineY += line.height();
        }
        drawLayout.endLayout();
        drawLayout.draw(painter, QPointF(bubbleRect.left() + padding, curY));
        curY += qCeil(textSize.height()) + spacing;
    }
    // Vẽ ảnh (lưới)
    for (int i = 0; i < images.size(); ++i) {
        int row = i / imgCols;
        int col = i % imgCols;
        int ix = bubbleRect.left() + padding + col * (thumbSize + spacing);
        int iy = curY + row * (thumbSize + spacing);
        QRect thumbRect(ix, iy, thumbSize, thumbSize);

        QString fullUrl = imageBaseUrl() + images[i].urlImage;
        QPixmap pix = ImageCache::instance().get(fullUrl);

        painter->setPen(QColor("#DDDDDD"));
        painter->setBrush(QColor("#EEEEEE"));
        painter->drawRoundedRect(thumbRect, 8, 8);

        if (!pix.isNull()) {
            QPixmap scaled = pix.scaled(thumbSize, thumbSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QPixmap clipped = scaled.copy(0, 0, thumbSize, thumbSize);
            painter->drawPixmap(thumbRect, clipped);
        } else {
            painter->setPen(Qt::gray);
            painter->drawText(thumbRect, Qt::AlignCenter, "…");
        }
    }
    if (!images.isEmpty())
        curY += imagesBlockH + spacing;

    // Vẽ file chip (icon + tên file)
    for (const FileInfo &f : files) {
        QRect chipRect(bubbleRect.left() + padding, curY, bubbleRect.width() - 2 * padding, fileChipH);
        painter->setBrush(QColor("#F0F0F0"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(chipRect, 6, 6);

        painter->setPen(Qt::black);
        painter->setFont(font);
        QRect iconRect(chipRect.left() + 6, chipRect.top(), 24, chipRect.height());
        painter->drawText(iconRect, Qt::AlignCenter, "📄");

        QRect nameRect(chipRect.left() + 34, chipRect.top(), chipRect.width() - 40, chipRect.height());
        QString elided = fm.elidedText(f.fileName, Qt::ElideMiddle, nameRect.width());
        painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

        curY += fileChipH + 4;
    }

    if(!isMine && isLastInFriendGroup(index) && !m_friendAvatarUrl.isEmpty()){
        int avatarSize = 28;
        QRect avatarRect(option.rect.left() + 4, bubbleRect.bottom() - avatarSize, avatarSize, avatarSize);
        QPixmap avatarPix = ImageCache::instance().get(m_friendAvatarUrl);
        if(!avatarPix.isNull()){
            QPixmap scaled = avatarPix.scaled(avatarSize, avatarSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QPainterPath clip;
            clip.addEllipse(avatarRect);
            painter->save();
            painter->setClipPath(clip);
            painter->drawPixmap(avatarRect, scaled);
            painter->restore();
        }else{
            painter->setBrush(Qt::gray);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(avatarRect);
        }
    }
    if(isMine && isLastMineMessage(index)){
        int isSend = index.data(ChatModel::IsSendRole).toInt();
        QString tickText;
        QColor tickColor = Qt::blue;
        switch(isSend){
        case 0:
            tickText = "✓";
            break;
        case 1:
            tickText = "✓✓";
            tickColor = Qt::blue;
            break;
        case 2:
            tickText = "đã xem";
            tickColor = Qt::lightGray;
            break;
        default:
            tickText = "✓";
            break;
        }
        QFont tickFont = font;
        tickFont.setPointSize(qMax(7, font.pointSize() - 2));
        painter->setPen(tickColor);
        painter->setFont(tickFont);
        QFontMetrics tickFm(tickFont);
        QRect tickRect(bubbleRect.left(), bubbleRect.bottom() + 2, bubbleRect.width(), tickFm.height() + 2);
        painter->drawText(tickRect, Qt::AlignRight | Qt::AlignVCenter, tickText);
    }
    painter->restore();

}

QSize ChatDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString content = index.data(ChatModel::ContentRole).toString();
    QVector<ImageInfo> images = index.data(ChatModel::ImagesRole).value<QVector<ImageInfo>>();
    QVector<FileInfo> files = index.data(ChatModel::FilesRole).value<QVector<FileInfo>>();
    int maxWidth = option.rect.width()*0.65;
    int padding = 10;
    int thumbSize = 120;
    int spacing = 6;
    int fileChipH = 36;

    QFont font = emojiCapableFont(option.font);
    int textH = 0;
    if (!content.isEmpty()) {
        textH = qCeil(wrappedTextSize(content, font, maxWidth - 2 * padding).height()) + spacing;
    }
    int imgCols = qMin(3, qMax(1, images.size()));
    int imgRows = images.isEmpty() ? 0 : (images.size() + imgCols - 1) / imgCols;
    int imagesH = images.isEmpty() ? 0 : imgRows * thumbSize + (imgRows - 1) * spacing + spacing;

    int filesH = files.isEmpty() ? 0 : files.size() * (fileChipH + 4);
    int sepH = shouldShowTimeSeparator(index) ? separatorHeight : 0;
    bool isMine = index.data(ChatModel::IsMineRole).toBool();
    int tickH = (isMine && isLastMineMessage(index)) ? 18 : 0;
    int totalH = 2 * padding + textH + imagesH + filesH + 10 + tickH;
    return QSize(option.rect.width(), totalH + sepH);
}

bool ChatDelegate::editorEvent(QEvent*event, QAbstractItemModel *itemModel,const QStyleOptionViewItem &option, const QModelIndex &index){
    if (event->type() != QEvent::MouseButtonRelease){
        return false;
    }

    auto *mouse = static_cast<QMouseEvent *>(event);
    QString content = index.data(ChatModel::ContentRole).toString();
    QVector<ImageInfo> images = index.data(ChatModel::ImagesRole).value<QVector<ImageInfo>>();
    QVector<FileInfo> files = index.data(ChatModel::FilesRole).value<QVector<FileInfo>>();
    int padding = 10;
    int maxWidth = option.rect.width() * 0.65;
    int thumbSize = 120;
    int spacing = 6;
    int fileChipH = 36;
    QFont font = emojiCapableFont(option.font);
    QSizeF textSize;
    if(!content.isEmpty()){
        textSize = wrappedTextSize(content, font, maxWidth - 2*padding);
    }

    int imgCols = qMin(3, qMax(1, images.size()));
    int imgRows = images.isEmpty()? 0 : (images.size()+imgCols-1)/imgCols;

    int imagesBlockW =images.isEmpty()? 0: imgCols*thumbSize+(imgCols-1)*spacing;

    int imagesBlockH =images.isEmpty()? 0: imgRows*thumbSize+(imgRows-1)*spacing;

    int filesBlockH =files.isEmpty()? 0: files.size()*(fileChipH+4);

   int bubbleW = qMax(qCeil(textSize.width()), imagesBlockW) + 2 * padding;

    bubbleW = qMax(bubbleW,files.isEmpty() ? 0 : maxWidth);

    int bubbleH =
        2*padding
         + (content.isEmpty() ? 0 : qCeil(textSize.height()) + spacing)
        +imagesBlockH
        +(images.isEmpty()?0:spacing)
        +filesBlockH;

    bool isMine =
        index.data(ChatModel::IsMineRole).toBool();

    int x =isMine? option.rect.right()-bubbleW-10 : option.rect.left()+10 + avatarGutter;
    int sepH = shouldShowTimeSeparator(index) ? separatorHeight : 0;
    int y = option.rect.top()+5+sepH;

    QRect bubbleRect(x,y,bubbleW,bubbleH);

    int curY =
        bubbleRect.top()+padding;

    if(!content.isEmpty())
        curY += qCeil(textSize.height())+spacing;

    if(!images.isEmpty()){
        for (int i = 0; i < images.size(); ++i) {
            int row = i / imgCols;
            int col = i % imgCols;
            int ix = bubbleRect.left() + padding + col * (thumbSize + spacing);
            int iy = curY + row * (thumbSize + spacing);
            QRect thumbRect(ix, iy, thumbSize, thumbSize);

            if (thumbRect.contains(mouse->pos())) {
                emit imageClicked(imageBaseUrl() + images[i].urlImage);
                return true;
            }
        }
        curY += imagesBlockH+spacing;
    }
    for(int i=0;i<files.size();++i)
    {
        QRect chipRect(
            bubbleRect.left()+padding,
            curY,
            bubbleRect.width()-2*padding,
            fileChipH);

        if(chipRect.contains(mouse->pos()))
        {
            emit fileClicked(imageBaseUrl()+files[i].urlFile,files[i].fileName);
            return true;
        }
        curY += fileChipH+4;
    }
    return false;
}

QSizeF ChatDelegate::wrappedTextSize(const QString &text, const QFont &font, qreal maxWidth) const
{
    QTextLayout layout(text, font);
    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout.setTextOption(opt);
    layout.beginLayout();
    qreal height = 0, lineWidth = 0;
    while (true) {
        QTextLine line = layout.createLine();
        if (!line.isValid()) break;
        line.setLineWidth(maxWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        lineWidth = qMax(lineWidth, line.naturalTextWidth());
    }
    layout.endLayout();
    return QSizeF(std::ceil(lineWidth), std::ceil(height));
}
void ChatDelegate::setFriendAvatarUrl(const QString &url){
    m_friendAvatarUrl = url;
}

bool ChatDelegate::isLastMineMessage(const QModelIndex &index) const{
    if(!index.data(ChatModel::IsMineRole).toBool()) return false;
    int rowCount = index.model()->rowCount();
    for(int r = rowCount - 1; r > index.row(); --r){
        if(index.model()->index(r, 0).data(ChatModel::IsMineRole).toBool())
            return false;
    }
    return true;
}
bool ChatDelegate::isLastInFriendGroup(const QModelIndex &index) const{
    if(index.data(ChatModel::IsMineRole).toBool()) return false;
    int row = index.row();
    int rowCount = index.model() -> rowCount();
    if(row == rowCount - 1) return true;
    QModelIndex next = index.model() -> index(row + 1, 0);
    return next.data(ChatModel::IsMineRole).toBool();
}
bool ChatDelegate::shouldShowTimeSeparator(const QModelIndex &index) const{
    if(index.row() == 0) return true;
    QDateTime cur = index.data(ChatModel::CreateAtRole).toDateTime();
    QModelIndex prev = index.model()->index(index.row() - 1, 0);
    QDateTime prevTime = prev.data(ChatModel::CreateAtRole).toDateTime();
    if(!cur.isValid() || !prevTime.isValid()) return false;
    return prevTime.secsTo(cur) >= 30*60;
}
QString ChatDelegate::formatSeparatorTime(const QDateTime &dt) const{
    QDate today = QDate::currentDate();
    if(dt.date() == today){
        return dt.toString("HH:mm");
    }else if(dt.date() == today.addDays(-1)){
        return "Hôm qua, " + dt.toString("HH:mm");
    }else{
        return dt.toString("d MM yyyy, HH:mm");
    }
}