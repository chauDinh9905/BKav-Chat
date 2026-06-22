#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QVector>

struct ImageInfo
{
    QString urlImage;
    QString fileName;
};

struct FileInfo
{
    QString urlFile;
    QString fileName;
};
struct MessageInfo
{
    int userId;
    int friendId;

    QString content;

    QVector<FileInfo> files;
    QVector<ImageInfo> images;

    QDateTime createdAt;
    QDateTime updatedAt;

    int isSend;

    bool isMine;

    MessageInfo(
        int userId,
        int friendId,
        const QString &content,
        const QVector<FileInfo> &files,
        const QVector<ImageInfo> &images,
        const QDateTime &createdAt,
        const QDateTime &updatedAt,
        int isSend,
        bool isMine)
        :
        userId(userId),
        friendId(friendId),
        content(content),
        files(files),
        images(images),
        createdAt(createdAt),
        updatedAt(updatedAt),
        isSend(isSend),
        isMine(isMine)
    {
    }
};
class ChatModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        ContentRole = Qt::UserRole + 1,
        IsMineRole
    };
    explicit ChatModel(QObject *parent = nullptr);

    int rowCount(
        const QModelIndex &parent =
        QModelIndex()) const override;

    QVariant data(
        const QModelIndex &index,
        int role) const override;

    void addMessage(
        const MessageInfo &message);

    void clear();

private:
    QVector<MessageInfo> messages;
};

#endif