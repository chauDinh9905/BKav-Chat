#include "chatdelegate.h"
#include "chatmodel.h"
#include "imagecache.h"
#include "appconfig.h"
#include <QMouseEvent>
#include <QTextLayout>
#include <QTextEdit>
#include <QtMath>

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
        qDebug()<< "[paint] dang chay" << option.rect.width() << "maxWidth: " << maxWidth
                 << "textSize: "<< textSize << "content len:" << content.length();
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

    int x = isMine ? option.rect.right() - bubbleW - 10 : option.rect.left() + 10;
    int y = option.rect.top() + 5;
    QRect bubbleRect(x, y, bubbleW, bubbleH);

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

    int totalH = 2 * padding + textH + imagesH + filesH + 10;
    return QSize(option.rect.width(), totalH);
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

    int x =
        isMine
            ? option.rect.right()-bubbleW-10
            : option.rect.left()+10;

    int y =
        option.rect.top()+5;

    QRect bubbleRect(x,y,bubbleW,bubbleH);

    int curY =
        bubbleRect.top()+padding;

    if(!content.isEmpty())
        curY += qCeil(textSize.height())+spacing;

    if(!images.isEmpty())
        curY += imagesBlockH+spacing;
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