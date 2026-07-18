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

Q_DECLARE_METATYPE(ImageInfo)
Q_DECLARE_METATYPE(FileInfo)
Q_DECLARE_METATYPE(QVector<ImageInfo>)
Q_DECLARE_METATYPE(QVector<FileInfo>)

struct MessageInfo
{
    qint64 userId;
    qint64 friendId;
    QString content;
    QVector<FileInfo> files;
    QVector<ImageInfo> images;
    QDateTime createdAt;
    QDateTime updatedAt;
    int isSend;
    bool isMine;
    QString messageId;
    MessageInfo(
        qint64 userId,
        qint64 friendId,
        const QString &content,
        const QVector<FileInfo> &files,
        const QVector<ImageInfo> &images,
        const QDateTime &createdAt,
        const QDateTime &updatedAt,
        int isSend,
        bool isMine,
        const QString &messageId = QString())
        :
        userId(userId),
        friendId(friendId),
        content(content),
        files(files),
        images(images),
        createdAt(createdAt),
        updatedAt(updatedAt),
        isSend(isSend),
        isMine(isMine),
        messageId(messageId)
    {
    }
};
class ChatModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        ContentRole = Qt::UserRole + 1,
        IsMineRole,
        ImagesRole,
        FilesRole,
        IsSendRole,
        MessageIdRole
    };
    explicit ChatModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent =QModelIndex()) const override;
    QVariant data(const QModelIndex &index,int role) const override;
    void addMessage(const MessageInfo &message);
    void clear();
    const MessageInfo& messageAt(int row) const {
        return messages[row];
    }
    void updateMessageStatusById(const QString &messageId, int newStatus);
    void markAllMineSeen();
private:
    QVector<MessageInfo> messages;
};

#endif