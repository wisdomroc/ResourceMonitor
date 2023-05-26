#include "data_center.h"

#include <QDebug>
#include <QDir>
#include <QThreadPool>
#include <Windows.h>
#include <QElapsedTimer>
#include <QMessageBox>

DataCenter::DataCenter(QObject *parent) : QObject(parent)
{

}



void DataCenter::setIwrp2FilePath(const QString &filePath)
{
    iwrp2FilePath_ = filePath;
    ReadXmlRunnable *runnable = new ReadXmlRunnable("", nullptr);
    QElapsedTimer mstimer;
    mstimer.start();
    bool ok = runnable->readXmlIwrp2(blockInfoMultiHash_, blockTypes_, iwrp2FilePath_);
    float time =(double)mstimer.nsecsElapsed()/(double)1000000;
    qDebug() <<"readXmlIwrp2 time= " <<time<<"ms";// 输出运行时间（ms）
    qDebug() << "blocks count: " << blockInfoMultiHash_.size() << endl;
    if(!ok) {
        QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("打开文件：%1失败，错误：%2").arg(filePath).arg(runnable->getError()));
    }
    rootNode_ = runnable->getRootNode();
}


void DataCenter::setIcdAndXcdFilePath(const QString &icdPath, const QString &xcdPath)
{
    icdFilePath_ = icdPath;
    xcdFilePath_ = xcdPath;
    xcdInfoMap_.clear();
    icdInfoMap_.clear();


    SYSTEM_INFO si;
    memset(&si,0,sizeof(SYSTEM_INFO));
    GetSystemInfo(&si);
    int number = si.dwNumberOfProcessors - 1;
    qDebug() << "current Computer has :" << number << "+ 1 CPU CORE ..." << endl;
    QThreadPool::globalInstance()->setMaxThreadCount(number);

    QElapsedTimer mstimer;
    mstimer.start();


    QDir dir_icd(icdPath);
    QStringList icdFileNames = dir_icd.entryList(QDir::Files);
    for(const auto& icdFileName : icdFileNames) {
        if(!icdFileName.startsWith("Group-")) continue;
        QString curIcdFileName = icdPath + "/" + icdFileName;

        auto func = std::bind(&DataCenter::UpdateFunc, this, std::placeholders::_1, std::placeholders::_2);
        ReadXmlRunnable *readXmlRunnable = new ReadXmlRunnable(curIcdFileName, func);
        QThreadPool::globalInstance()->start(readXmlRunnable);
    }

    QDir dir_xcd(xcdPath);
    QStringList xcdFileNames = dir_xcd.entryList(QDir::Files);
    for(const auto& xcdFileName : xcdFileNames) {
        if(!xcdFileName.startsWith("Group-")) continue;
        QString curXcdFileName = xcdPath + "/" + xcdFileName;

        auto func = std::bind(&DataCenter::UpdateFunc, this, std::placeholders::_1, std::placeholders::_2);
        ReadXmlRunnable *readXmlRunnable = new ReadXmlRunnable(curXcdFileName, func, true);
        QThreadPool::globalInstance()->start(readXmlRunnable);
    }

    QThreadPool::globalInstance()->waitForDone();
    float time =(double)mstimer.nsecsElapsed()/(double)1000000;
    qDebug() <<"readXmlICDs time= " <<time<<"ms";// 输出运行时间（ms）
}


QStringList DataCenter::getFilterNames() const
{
    QStringList blockNames = blockInfoMultiHash_.keys();
    blockNames.removeDuplicates();
    blockNames.sort();
    return blockNames;
}

QStringList DataCenter::getFilterTypes() const
{
    return blockTypes_;
}

QMap<QString, QMap<QString, InfoTuple> > DataCenter::getDatasPercent(const QStringList &names, const QStringList &types, ShowLevel showLevel)
{
    QMultiMap<QString, QMap<QString, InfoTuple> > retMap;
    QElapsedTimer mstimer;
    mstimer.start();

    if(!names.empty()) {
        for(const auto &name : names) {
            QList<BlockInfoPtr> blocks = blockInfoMultiHash_.values(name);
            processBlock(blocks, types, showLevel, retMap);
        }
    } else {
            processBlock(blockInfoMultiHash_.values(), types, showLevel, retMap);
    }

    float time =(double)mstimer.nsecsElapsed()/(double)1000000;
    qDebug() <<"processBlock time= " <<time<<"ms";// 输出运行时间（ms）

    return std::move(retMap);
}

void DataCenter::slot_DoneICDInfo(ICDInfo icdInfo, bool isXcd)
{
    if(isXcd) {
        xcdInfoMap_.insert(icdInfo.name_, icdInfo);
    } else {
        icdInfoMap_.insert(icdInfo.name_, icdInfo);
    }
}

void DataCenter::processBlock(const QList<BlockInfoPtr> &blocks, const QStringList &types, ShowLevel showLevel, QMultiMap<QString, QMap<QString, InfoTuple>> &retMap)
{
    QStringList local_types = types;
    bool findType = false;
    for(const auto &block : blocks) {
        for(const auto &type : local_types) {
            if(block->properties.keys().contains(type)) {
                findType = true;
                break;
            }
        }

        if(types.empty()){
            findType = true;
        }

        if(findType) {
            QMap<QString, InfoTuple> oneBlockPercents;
            if(types.empty()) {
                local_types = block->properties.keys();
            }
            for(const auto &type : local_types) {
                int icdSizes = 0;
                QStringList icdnameList; //! 去重后的icd
                QString keyType = type;
                keyType.remove("M_");
                for(auto &icdgroup : block->icdGroupInfoList) {
                    if(!icdgroup->name.contains(keyType)) continue;
                    for(auto &icd : icdgroup->icdNameList) {
                        icdnameList.append(icd);
                    }
                }
                for(auto &icdname : icdnameList) {
                    icdSizes += icdInfoMap_.value(icdname).size_; //这里即使找不到，进行插入也是0，不影响最终计算
                    icdSizes += xcdInfoMap_.value(icdname).size_; //这里即使找不到，进行插入也是0，不影响最终计算
                }
                int realSize = icdSizes;
                int customSize  = block->properties.value(type).toInt();
                double percent = ((double)realSize) / customSize;
                if(isnan(percent)) continue;

                if(percent - 1.0 > PRECISION && !(showLevel & RedLevel))
                    continue;
                if(qAbs(percent - 1.0) < PRECISION && !(showLevel & YellowLevel))
                    continue;
                if(1.0 - percent > PRECISION && !(showLevel & GreenLevel))
                    continue;

                oneBlockPercents.insert(type, std::make_tuple(QString::number(percent, 'f', 2), realSize, customSize));
//                qDebug() << "==========>" << block->name << "," << type << "," << QString::number(percent, 'f', 2) << endl;
            }
            retMap.insertMulti(block->name + "\n(\n" + block->namepath + "\n)", oneBlockPercents);
        }
    }
}
