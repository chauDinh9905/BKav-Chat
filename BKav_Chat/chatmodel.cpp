#include "chatmodel.h"

ChatModel::ChatModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ChatModel::rowCount(
    const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return messages.size();
}

QVariant ChatModel::data(
    const QModelIndex &index,
    int role) const
{
    if(!index.isValid())
        return QVariant();

    const MessageInfo &msg = messages[index.row()];

    switch (role) {
    case ContentRole:
        return msg.content;
    case IsMineRole:
        return msg.isMine;
    case ImagesRole:
        return QVariant::fromValue(msg.images);
    case FilesRole:
        return QVariant::fromValue(msg.files);
    default:
        return QVariant();
    }
}

void ChatModel::addMessage(
    const MessageInfo &message)
{
    beginInsertRows(
        QModelIndex(),
        messages.size(),
        messages.size());

    messages.append(message);

    endInsertRows();
}

void ChatModel::clear()
{
    beginResetModel();
    messages.clear();
    endResetModel();
}