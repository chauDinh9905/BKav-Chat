#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "chatmodel.h"

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
    void onMessageReceived(
        const QString &message);

private:
    qint64 myId;
    QString friendId;
    QString friendName;
    QString avatarPath;

    QLabel *avatarLabel;
    QLabel *nameLabel;

    QPushButton *closeButton;

    QListView *messageView;

    QLineEdit *messageEdit;

    QPushButton *sendButton;
    QPushButton *emojiButton;
    QPushButton *imageButton;
    QPushButton *fileButton;

    ChatModel *model;

    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QHBoxLayout *bottomLayout;

    QNetworkAccessManager *networkManager;
public:
    void loadMessages();
    MessageInfo createMessageFromVariant(const QVariantMap &data);
};

#endif