/**
* @filename readxmlrunnable.h
* @brief    读取Iwrp2 & ICD Files XML文件的线程类
* @author   wanghp
* @date     2023-05-18
*/

#ifndef READXMLRUNNABLE_H
#define READXMLRUNNABLE_H

#include "pugixml.h"
#include "global.h"
#include <QRunnable>

typedef std::function<void(const ICDInfo& icdInfo, bool isXcd)> UpdateFunc;

class ReadXmlRunnable : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit ReadXmlRunnable(const QString &filepath, const UpdateFunc& func, bool isXcd = false);

    void    run() override;

    Node*   getRootNode() {
        return rootNode_;
    }
    QString getError();
    ICDInfo readXmlOneIcd(const QString& filename);
    bool    readXmlIwrp2(QMultiHash<QString, BlockInfoPtr>& blockInfoMultiHash, QStringList& blockTypes, const QString &filename);

private:
    void processBlock(const QList<BlockInfoPtr> &blocks,
                      const QStringList &types,
                      QMultiMap<QString, QMap<QString, QString> > &retMap);

    QString getNodePath(Node* node);
    void findChildModule(QMultiHash<QString, BlockInfoPtr>& blockInfoMultiHash,
                         QStringList& blockTypes,
                         const pugi::xml_node& modules,
                         const std::shared_ptr<BlockInfo>& parentBlock,
                         Node *parentNode);

signals:
    void doneICDInfo(ICDInfo icdInfo, bool isXcd);

private:
    QString     errStr_;
    QString     filepath_;
    bool        isXcd_;
    Node*       rootNode_;
    UpdateFunc  updateFunc_;
};

#endif // READXMLRUNNABLE_H
