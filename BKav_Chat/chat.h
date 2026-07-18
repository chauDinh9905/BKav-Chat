#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScrollArea>
#include "chatmodel.h"

struct PendingAttachment {
    QString filePath;
    bool isImage;
};

inline bool operator==(const PendingAttachment &a, const PendingAttachment &b) {
    return a.filePath == b.filePath && a.isImage == b.isImage;
}
class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(
        qint64 myId,
        const QString &friendId,
        const QString &friendName,
        const QString &avatarPath,
        QWidget *parent = nullptr);

signals:
    void closeRequested();

private slots:
    void sendMessage();
    void selectImage();
    void selectFile();
    void closeChat();
    void selectEmoji();
    void onMessageReceived(
        const QString &message);

private:
    qint64 myId;
    qint64 lastEmojiPopupCloseMs = 0;
    QString friendId;
    QString friendName;
    QString avatarPath;

    QLabel *avatarLabel;
    QLabel *nameLabel;

    QPushButton *closeButton;

    QListView *messageView;

    QTextEdit *messageEdit;

    QPushButton *sendButton;
    QPushButton *emojiButton;
    QPushButton *imageButton;
    QPushButton *fileButton;

    ChatModel *model;

    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QHBoxLayout *bottomLayout;

    QNetworkAccessManager *networkManager;
    QWidget *attachmentPreviewBar;
    QWidget *emojiPopup = nullptr;
    QHBoxLayout *attachmentPreviewLayout;
    QScrollArea *attachmentPreviewScroll;
    QVector<PendingAttachment> pendingAttachments;
    void addAttachmentPreview(const QString &filePath, bool isImage);
    void clearAttachments();
public:
    void loadMessages();
    MessageInfo createMessageFromVariant(const QVariantMap &data);
    QVector<ImageInfo> parseImages(const QJsonArray &arr);
    QVector<FileInfo> parseFiles(const QJsonArray &arr);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void downloadFile(const QString &url, const QString &fileName);
    void adjustMessageEditHeight();
};

#endif