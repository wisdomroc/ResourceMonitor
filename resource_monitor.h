/**
* @filename resource_monitor.h
* @brief    主程序框架RecouceManitor（资源占空比监视器）
*           多线程读取xml文件并分析，数据存于QMultiMap中
*           界面使用Model&View保证效率
* @author   wanghp
* @date     2023-05-10
*/

#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H
#include "data_center.h"
#include <QWidget>
#include <QMessageBox>

namespace Ui {
class ResourceMonitor;
}
class WSortFilterProxyModel;
class QSortFilterProxyModel;
class TreeModel;

extern const QString APPLICATION_NAME;
extern const QString APPLICATION_NAME_CH;
extern const QString APPLICATION_VERSION;

class ResourceMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit ResourceMonitor(const QString& iwrpFilename,
                             const QString& icdFilepath,
                             const QString& xcdFilepath,
                             QWidget *parent = nullptr);
    ~ResourceMonitor();

    void setIwrpFilePath(const QString& iwrpFileName) {
        dataCenter_.setIwrp2FilePath(iwrpFileName);
    }

    void setIcdAndXcdFilePath(const QString& icdFilePath, const QString& xcdFilePath) {
        dataCenter_.setIcdAndXcdFilePath(icdFilePath, xcdFilePath);
    }

private:
    void initUi();
    void refreshUi();
    void checkWorkMode();

private slots:
    void slot_changeTheme(int theme);
    void slot_activeBlockTreeView(const QString& uuid);
    void slot_treeViewPressed(const QModelIndex &index);

    void on_pushButton_clearName_clicked();

    void on_pushButton_clearType_clicked();

    void on_comboBox_showMode_currentIndexChanged(int index);

    void on_pushButton_red_toggled(bool checked);

    void on_pushButton_yellow_toggled(bool checked);

    void on_pushButton_green_toggled(bool checked);

    void on_pushButton_refresh_clicked();

private:
    Ui::ResourceMonitor *ui;
    DataCenter dataCenter_;
    QMultiMap<QString, QMap<QString, InfoTuple>> datasPercent_;
    WSortFilterProxyModel* sortFilter_;
    TreeModel *treeMode_;
    ShowLevel showLevel_;
    bool isInitialized_;
};

#endif // RESOURCE_MONITOR_H
