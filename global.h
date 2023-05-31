#ifndef GLOBAL_H
#define GLOBAL_H

#include <memory>
#include <QObject>
#include <QDir>

#include <QList>
#include <QVector>
#include <QMultiHash>
#include <QMap>
#include <QMultiMap>

const double PRECISION = 0.001;
using InfoTuple = std::tuple<QString, uint, uint>;

enum ShowMode {
    ChartMode,
    TableMode
};

enum ShowLevel {
    NoneLevel   = 0x0000,
    RedLevel    = 0x0001,
    YellowLevel = 0x0002,
    GreenLevel  = 0x0004,
    RYLevel     = RedLevel | YellowLevel,
    RGLevel     = RedLevel | GreenLevel,
    YGLevel     = YellowLevel | GreenLevel,
    RYGLevel    = RedLevel | YellowLevel | GreenLevel
};

const int UUID_ROLE = Qt::UserRole + 1;


struct ICDInfo {
    ICDInfo():size_(0) {
    }
    ICDInfo(QString name, int size) {
        name_ = name;
        size_ = size;
    }
    bool isEmpty() {
        return name_.isEmpty() && size_ == 0;
    }
    QString name_;   // 实际ICD的名称
    int size_;   // 实际ICD的大小
};

struct ICDGroupInfo {
    QString name;
    int type;                   // 5是IcdIn, 6是IcdOut
    QString uuid;               // IcdInGroup or IcdOutGroup 图元的uuid
    QList<QString> icdNameList; // 图元中所包含的 实际ICD文件
};
using ICDGroupInfoPtr = std::shared_ptr<ICDGroupInfo>;


struct BlockInfo {
    QString name;                       // Block的名称
    QString namepath;                   // Block的整体层级
    QString uuid;                       // Block图元的uuid
    QMap<QString, QString> properties;  // Block中的自定义属性
    QList<ICDGroupInfoPtr> icdGroupInfoList;
};
using BlockInfoPtr = std::shared_ptr<BlockInfo>;

struct Node {
    Node(const QString& name = "", const QString& fullname = "", int type = -1, int number = -1):name_(name),fullname_(fullname),type_(type),parentNode_(nullptr),number_(number){}
    void appendChild(Node *node) {
        node->number_ = childNodes_.size();
        node->parentNode_ = this;
        childNodes_.append(node);
    }

    Node* child(int row) {
        if(row < 0 || row >= childNodes_.size()) {
            return nullptr;
        }
        return childNodes_.at(row);
    }

    Node* parent() {
        return parentNode_;
    }

    int row() {
        return number_;
    }

    int childCount() {
        return childNodes_.size();
    }

    QString         name_;       //名称
    QString         fullname_;   //完整路径
    QString         uuid_;       //uuid
    int             type_;       //类型
    Node*           parentNode_; //父节点
    QList<Node*>    childNodes_; //子节点
    int             number_;     //当前节点在父节点中是第几个子节点
};

#endif // GLOBAL_H
