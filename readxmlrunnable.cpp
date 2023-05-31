#include "readxmlrunnable.h"
#include <QDebug>
#include <QThread>

const pugi::char_t* PROPERTY_KEY = "custom_attrs"; //! 与wrp魏央那边同步

ReadXmlRunnable::ReadXmlRunnable(const QString& filepath, const UpdateFunc& updateFunc, bool isXcd) : filepath_(filepath),isXcd_(isXcd),updateFunc_(updateFunc)
{
    rootNode_             = new Node("Root");
}

void ReadXmlRunnable::run()
{
    ICDInfo icdInfo = readXmlOneIcd(filepath_);
    updateFunc_(icdInfo, isXcd_);
}

QString ReadXmlRunnable::getError()
{
    return errStr_;
}

bool ReadXmlRunnable::readXmlIwrp2(QMultiHash<QString, BlockInfoPtr>& blockInfoMultiHash, QStringList& blockTypes, const QString &filename)
{
    blockInfoMultiHash.clear();
    QString filepath = QDir::toNativeSeparators(filename);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filepath.toLocal8Bit());
    if(!result) {
        errStr_ = result.description();
        return false;
    }

    pugi::xml_node root_node                = doc.child("project");
    pugi::xml_node systemarchitecture_node  = root_node.child("systemarchitecture");
    pugi::xml_node modules_node             = systemarchitecture_node.child("modules");

    findChildModule(blockInfoMultiHash, blockTypes, modules_node, nullptr, nullptr);

    return true;
}

QString ReadXmlRunnable::getNodePath(Node *node)
{
    QStringList retPathList;
    while (node->parent() != nullptr) {
        retPathList.insert(0, "/" + node->name_);
        node = node->parent();
    }
    return retPathList.join("\n");
}

void ReadXmlRunnable::findChildModule(QMultiHash<QString, BlockInfoPtr>& blockInfoMultiHash,
                                      QStringList& blockTypes,
                                      const pugi::xml_node& modules,
                                      const std::shared_ptr<BlockInfo>& parentBlock,
                                      Node* parentNode)
{
    for(pugi::xml_node module = modules.first_child(); module; module = module.next_sibling()) {
        if(strcmp(module.name(), "module") != 0) {
            continue;
        }

        //! 是module节点
        QString type = module.child("type").child_value();
        if(type == "2") {           //! 是Block节点
            BlockInfoPtr blockInfo = std::make_shared<BlockInfo>();
            blockInfo->name = module.child("name").child_value();
            blockInfo->uuid = module.child("uuid").child_value();

            for(auto property = module.child(PROPERTY_KEY).first_child(); property; property = property.next_sibling()) {
                blockInfo->properties.insert(property.name(), property.child_value());
                if(!blockTypes.contains(property.name())) {
                    blockTypes.append(property.name());
                }
            }
            blockInfoMultiHash.insertMulti(blockInfo->name, blockInfo);

            //TODO.
            Node *node = new Node();
            node->name_ = module.child("name").child_value();
            node->type_ = type.toInt();
            if(parentNode == nullptr) {
                rootNode_->appendChild(node);
            } else {
                parentNode->appendChild(node);
            }

            blockInfo->namepath = getNodePath(node);
            node->fullname_ = blockInfo->namepath;
            node->uuid_     = blockInfo->uuid;
            findChildModule(blockInfoMultiHash, blockTypes, module.child("submodule").child("modules"), blockInfo, node);
        } else if (type == "5") {   //! 是IcdInGroup
            ICDGroupInfoPtr icdGroupInfo = std::make_shared<ICDGroupInfo>();
            icdGroupInfo->name = module.child("name").child_value();
            icdGroupInfo->uuid = module.child("uuid").child_value();
            icdGroupInfo->type = type.toInt();
            parentBlock->icdGroupInfoList.append(icdGroupInfo);

            for(pugi::xml_node icd = module.child("icds").first_child(); icd; icd = icd.next_sibling()) {
                icdGroupInfo->icdNameList.append(icd.child_value());
            }
        } else if (type == "6") {   //! 是IcdInGroup
            ICDGroupInfoPtr icdGroupInfo = std::make_shared<ICDGroupInfo>();
            icdGroupInfo->name = module.child("name").child_value();
            icdGroupInfo->uuid = module.child("uuid").child_value();
            icdGroupInfo->type = type.toInt();
            parentBlock->icdGroupInfoList.append(icdGroupInfo);

            for(pugi::xml_node icd = module.child("icds_output").first_child(); icd; icd = icd.next_sibling()) {
                icdGroupInfo->icdNameList.append(icd.child_value());
            }
        } else if (type == "0") {
            //TODO.
            Node *node = new Node();
            node->name_ = module.child("name").child_value();
            node->type_ = type.toInt();
            if(parentNode == nullptr) {
                rootNode_->appendChild(node);
            } else {
                parentNode->appendChild(node);
            }
            findChildModule(blockInfoMultiHash, blockTypes, module.child("submodule").child("modules"), nullptr, node);
        }
    }
}

ICDInfo ReadXmlRunnable::readXmlOneIcd(const QString &filename)
{
    QString filepath = QDir::toNativeSeparators(filename);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filepath.toStdString().data());
    if(!result)
    {
        errStr_ = result.description();
        return ICDInfo();
    }

    pugi::xml_node root_node                = doc.child("root");
    QString icdname = root_node.child("packageName").child_value();
    QString icdsize = root_node.child("dataSize").child_value();
    return ICDInfo(icdname, icdsize.toInt());
}
