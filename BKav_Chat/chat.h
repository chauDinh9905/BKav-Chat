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
#include <QHash>
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
    static void openChatWindow(qint64 myId, const QString &friendId,
                               const QString &friendName, const QString &avatarPath);
    static bool isChatOpen(const QString &friendId);
    static void closeAllChatWindows(); // khi logout
signals:
    void closeRequested();
    void nicknameMenuRequested(const QString &friendId,const QString &currentDisplayName,const QString &originalName,const QPoint &globalPos);
private slots:
    void sendMessage();
    void selectImage();
    void selectFile();
    void closeChat();
    void selectEmoji();
    void onMessageReceived(const QString &message);
    void showImagePreview(const QString &url);
    void setDisplayName(const QString &name);
private:
    qint64 myId;
    qint64 lastEmojiPopupCloseMs = 0;
    QString friendId;
    QString friendName;
    QString avatarPath;

    QLabel *avatarLabel;
    QPushButton *nameButton;

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
    void saveImageToDisk(const QString &url, QWidget *parentDialog = nullptr);
    static QHash<QString, Chat*> s_openChats;
public:
    void loadMessages();
    MessageInfo createMessageFromVariant(const QVariantMap &data);
    QVector<ImageInfo> parseImages(const QJsonArray &arr);
    QVector<FileInfo> parseFiles(const QJsonArray &arr);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void refreshAttachmentPreviewLayout();
private slots:
    void downloadFile(const QString &url, const QString &fileName);
    void adjustMessageEditHeight();
};

#endif