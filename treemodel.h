/**
* @filename treemodel.h
* @brief    TreeModel类是对QAbstractItemModel的继承，使用了 自定义数据类型Node，用于原始数据的存储；
*           其中包含了WSortFilterModelProxy是用于过滤数据的，支持多个字符串的过滤；
* @author   wanghp
* @date     2023-05-17
*/

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include "global.h"
#include <QSortFilterProxyModel>
#include <QDebug>



class TreeModel : public QAbstractItemModel
{
public:
    TreeModel(const QStringList& headers, QObject *parent = Q_NULLPTR);
    ~TreeModel() override;
    void                setRootNode(Node *node) {
        beginResetModel();
        rootNode_ = node;
        endResetModel();
    }

    QVariant            headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant            data(const QModelIndex &index, int role) const override;

    QModelIndex         index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex         parent(const QModelIndex &index) const override;
    int                 rowCount(const QModelIndex &parent) const override;
    int                 columnCount(const QModelIndex &parent) const override;
    Qt::ItemFlags       flags(const QModelIndex &index) const override;

    QModelIndex         indexFromNodeFullName(QString blockFullname) const;
    Node*               nodeFromIndex(const QModelIndex &index) const;
private:
    QStringList headers_;   //水平表头
    Node*       rootNode_;  //根节点
};

class WSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit WSortFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent)
    {}
    ~WSortFilterProxyModel() override{}

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        QStringList keyStrs = this->filterRegExp().pattern().split("|", QString::SkipEmptyParts);
        if(keyStrs.size() == 0)
        {
            return true;
        }

        QString data =  sourceModel()->index(sourceRow, 0, sourceParent).data(Qt::DisplayRole).toString();
        if(keyStrs.contains(data))
        {
            return true;
        }
        else
        {
            QModelIndex source_index = sourceModel()->index(sourceRow, 0, sourceParent);
            for(int k=0; k<sourceModel()->rowCount(source_index); ++k)
            {
                if(filterAcceptsRow(k, source_index))
                {
                    return true;
                }
            }
        }
        return false;
    }

private:

};

#endif // TREEMODEL_H
