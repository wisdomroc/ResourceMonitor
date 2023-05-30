/**
* @filename data_center.h
* @brief    数据中心，通过pugi库进行xml文件信息的读取，并且存入自定义结构中
*           提供系列数据获取接口
* @author   wanghp
* @date     2023-05-10
*/

#ifndef DATA_CENTER_H
#define DATA_CENTER_H

#include <QObject>
#include "readxmlrunnable.h"
#include <mutex>

class DataCenter : public QObject
{
    Q_OBJECT
public:
    explicit DataCenter(QObject *parent = nullptr);

    void setIwrp2FilePath(const QString& filePath);
    void setIcdAndXcdFilePath(const QString& icdPath, const QString& xcdPath);

    Node*       getRootNode() {
        return rootNode_;
    }

    void UpdateFunc(const ICDInfo& icdInfo, bool isXcd){
        std::unique_lock<std::mutex> locker(mtx_);
        if(isXcd) {
            xcdInfoMap_.insert(icdInfo.name_, icdInfo);
        } else {
            icdInfoMap_.insert(icdInfo.name_, icdInfo);
        }
    }

    void updateMap(const ICDInfo& icdInfo, bool isXcd = false) {
        if(isXcd) {
            xcdInfoMap_.insert(icdInfo.name_, icdInfo);
        } else {
            icdInfoMap_.insert(icdInfo.name_, icdInfo);
        }
    }

    const QMultiHash<QString, BlockInfoPtr>& getFilterInfos() const;
    QStringList getFilterNames() const;
    QStringList getFilterTypes() const;

    QMap<QString, QMap<QString, InfoTuple> > getDatasPercent(const QStringList &names, const QStringList &types, ShowLevel showLevel);

    QString getIcdFilePath() const {
        return icdFilePath_;
    }

    QString getXcdFilePath() const {
        return xcdFilePath_;
    }

    QString getIwrp2FilePath() const {
        return iwrp2FilePath_;
    }

private slots:
    void slot_DoneICDInfo(ICDInfo icdInfo, bool isXcd);

private:
    QMultiHash<QString, BlockInfoPtr> blockInfoMultiHash_;
    QStringList blockTypes_;

    QMap<QString, ICDInfo> icdInfoMap_;
    QMap<QString, ICDInfo> xcdInfoMap_;

    Node*   rootNode_;
    std::mutex mtx_;

    QString iwrp2FilePath_;
    QString icdFilePath_;
    QString xcdFilePath_;
    void processBlock(const QList<BlockInfoPtr> &blocks, const QStringList &types, ShowLevel showLevel, QMultiMap<QString, QMap<QString, InfoTuple> > &retMap);
};

#endif // DATA_CENTER_H
