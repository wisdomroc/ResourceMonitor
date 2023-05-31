#include "treemodel.h"
#include <QPixmap>

TreeModel::TreeModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel (parent),
      headers_(headers)
{

}

TreeModel::~TreeModel()
{
    delete rootNode_;
}

// 获取表头数据
QVariant TreeModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return headers_.at(section);
        }
    }
    return QVariant();
}

// 获取index.row行，index.column列数据
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node* item = nodeFromIndex(index);
    if (role == Qt::DisplayRole)
    {
        return item->name_;
    }
    if(role == UUID_ROLE)
    {
        return item->uuid_;
    }
    if(role == Qt::ToolTipRole)
    {
        return item->fullname_;
    }
    if(role == Qt::DecorationRole)
    {
        Node *item = nodeFromIndex(index);
        QString filepath;
        switch (item->type_) {
        case 0:
            filepath = ":/img/package.png";
            break;
        case 2:
            filepath = ":/img/block.png";
            break;
        case 5:
            filepath = ":/img/icdin.png";
            break;
        case 6:
            filepath = ":/img/icdout.png";
            break;
        }
        QPixmap pixmap(filepath);
        return QVariant::fromValue(pixmap);
    }

    return QVariant();
}

// 在parent节点下，第row行，第column列位置上创建索引
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Node* parentNode = nodeFromIndex(parent);
    Node* node = parentNode->child(row);
    if (node)
        return createIndex(row, column, node);
    else
        return QModelIndex();
}

// 创建index的父索引
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node* node = nodeFromIndex(index);
    Node* parentNode = node->parent();

    if (parentNode == rootNode_)
        return QModelIndex();
    return createIndex(parentNode->row(), 0, parentNode);
}

// 获取索引parent下有多少行
int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    Node* node = nodeFromIndex(parent);
    return node->childCount();
}

// 返回索引parent下有多少列
int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return headers_.size();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    return flags;
}

QModelIndex TreeModel::indexFromNodeUuid(QString uuid, Node* node) const
{
    QModelIndex index;
    if(!node) {
        node = rootNode_;
    }

    for(auto childNode : node->childNodes_) {
        if(childNode->uuid_ == uuid) {
            index = createIndex(childNode->row(), 0, childNode);
            break;
        } else {
            index = indexFromNodeUuid(uuid, childNode);
            if(index != QModelIndex()) {
                break;
            }
        }
    }
    return index;
}

Node* TreeModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        Node *item = static_cast<Node*>(index.internalPointer());
        return item;
    }
    return rootNode_;
}
