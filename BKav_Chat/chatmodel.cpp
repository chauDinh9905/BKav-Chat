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

    if(role == ContentRole)
        return messages[index.row()].content;
    if(role == IsMineRole)
        return messages[index.row()].isMine;
    return QVariant();
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