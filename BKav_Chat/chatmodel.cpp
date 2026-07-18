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
    case IsSendRole:
        return msg.isSend;
    case MessageIdRole:
        return msg.messageId;
    default:
        return QVariant();
    }
}

void ChatModel::updateMessageStatusById(const QString &messageId, int newStatus){
    if(messageId.isEmpty()) return;
    int n = messages.size();
    for(int i = 0; i < n; ++i){
        if(messages[i].messageId == messageId){
            if(newStatus > messages[i].isSend){
                messages[i].isSend = newStatus;
                QModelIndex idx = index(i);
                emit dataChanged(idx, idx, {IsSendRole});
            }
            break;
        }
    }
}

void ChatModel::markAllMineSeen(){
    if(messages.isEmpty()) return;
    bool changed = false;
    for (auto &m: messages){
        if(m.isMine && m.isSend < 2){
            m.isSend = 2;
            changed = true;
        }
    }
    if(changed) emit dataChanged(index(0), index(messages.size() - 1), {IsSendRole});
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