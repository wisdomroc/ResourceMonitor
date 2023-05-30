#include "resource_monitor.h"
#include "ui_resource_monitor.h"
#include <QSettings>
#include <QStyleFactory>
#include <QDebug>
#include "treemodel.h"

const QString APPLICATION_NAME      = QString("ResourceMonitor");
const QString APPLICATION_NAME_CH   = QStringLiteral("存储资源占空比监控程序");
const QString APPLICATION_VERSION   = QString("v0.0.1");
const QStringList CHART_THEME_NAMES = { "Light", "BlueCerulean", "Dark", "BrownSand", "BlueNcs", "HighContrast", "BlueIcy", "Qt"};

ResourceMonitor::ResourceMonitor(const QString &iwrpFilename, const QString &icdFilepath, const QString &xcdFilepath, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResourceMonitor),
    showLevel_(RYGLevel),
    isInitialized_(false)
{
    ui->setupUi(this);
    ui->widget->setFocusPolicy(Qt::StrongFocus);

    QStringList strList;
    strList << "1" << "12" << "123" << "1234" << "12345";
    for(auto& str : strList) {

        ui->comboBox->addItem(str);
    }


    setIwrpFilePath(iwrpFilename);
    setIcdAndXcdFilePath(icdFilepath, xcdFilepath);

    initUi();
    ui->treeView->setStyle(QStyleFactory::create("windows"));
    checkWorkMode();
    isInitialized_ = true;
}

ResourceMonitor::~ResourceMonitor()
{
    QString theme  = ui->comboBox_chartStyle->currentText();
    int showMode   = ui->comboBox_showMode->currentIndex();
    QSettings settings;
    settings.setValue("theme", theme);
    settings.setValue("ShowMode", showMode);
    settings.setValue("ShowLevel", showLevel_);
    delete ui;
}

void ResourceMonitor::initUi()
{
    setWindowTitle(APPLICATION_NAME_CH + " " + APPLICATION_VERSION);
    ui->comboBox_nameFilter->addItems(dataCenter_.getFilterInfos());
    ui->comboBox_typeFilter->addItems(dataCenter_.getFilterTypes());
    ui->comboBox_nameFilter->setCurrentText("");
    ui->comboBox_typeFilter->setCurrentText("");
    ui->comboBox_chartStyle->addItems(CHART_THEME_NAMES);
    connect(ui->comboBox_chartStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_changeTheme(int)));
    connect(ui->comboBox_nameFilter, &WComboBox::done, [this](){ refreshUi(); });
    connect(ui->comboBox_typeFilter, &WComboBox::done, [this](){ refreshUi(); });


    QSettings settings;
    QString theme = settings.value("theme").toString();
    int showMode  = settings.value("ShowMode").toInt();
    showLevel_    = (ShowLevel)settings.value("ShowLevel").toInt();
    {
        if(showLevel_ & RedLevel) {
            ui->pushButton_red->setChecked(true);
        } else {
            ui->pushButton_red->setChecked(false);
        }
        if(showLevel_ & YellowLevel) {
            ui->pushButton_yellow->setChecked(true);
        } else {
            ui->pushButton_yellow->setChecked(false);
        }
        if(showLevel_ & GreenLevel) {
            ui->pushButton_green->setChecked(true);
        } else {
            ui->pushButton_green->setChecked(false);
        }
    }
    int themeIndex = CHART_THEME_NAMES.indexOf(theme);
    ui->comboBox_chartStyle->setCurrentIndex(themeIndex);
    ui->comboBox_showMode->setCurrentIndex(showMode);
    slot_changeTheme(themeIndex);

    treeMode_ = new TreeModel(QStringList() << QStringLiteral("结构树"));
    treeMode_->setRootNode(dataCenter_.getRootNode());
    sortFilter_ = new WSortFilterProxyModel();
    sortFilter_->setSourceModel(treeMode_);
    ui->treeView->setModel(sortFilter_);

    connect(ui->widget, SIGNAL(activeBlock(QString)), this, SLOT(slot_activeBlockTreeView(QString)));
    connect(ui->treeView, SIGNAL(pressed(const QModelIndex&)),
            this, SLOT(slot_treeViewPressed(const QModelIndex&)));

    refreshUi();
}

void ResourceMonitor::refreshUi()
{
    ui->widget->clearAllBlock();
    QStringList filterNames = ui->comboBox_nameFilter->getCheckedToolTipData();
    QStringList filterTypes = ui->comboBox_typeFilter->getCheckedData();


    QString keyStr = "";
    if(filterNames.size() > 0) {
        keyStr = filterNames.join("|");
        QRegExp regExp(keyStr, Qt::CaseInsensitive, QRegExp::RegExp);
        sortFilter_->setFilterRegExp(regExp);
    } else {
        sortFilter_->setFilterRegExp(nullptr);
    }

    if(!filterNames.empty()) {
        ui->treeView->expandAll();
    }



    datasPercent_ = dataCenter_.getDatasPercent(filterNames, filterTypes, showLevel_);
    for(auto data = datasPercent_.begin(); data != datasPercent_.end(); ++data) {
        QString blockName = data.key();
        ui->widget->addBlockPercent(blockName, data.value());
    }

    checkWorkMode();
}

void ResourceMonitor::checkWorkMode()
{
    if(ui->comboBox_showMode->currentIndex() == 0) {
        ui->widget->setShowMode(ChartMode);
    } else {
        ui->widget->setShowMode(TableMode);
    }
}

void ResourceMonitor::slot_changeTheme(int theme)
{
    ui->widget->setTheme(QChart::ChartTheme(theme));
}

void ResourceMonitor::slot_activeBlockTreeView(const QString &blockFullname)
{
    QModelIndex index       = treeMode_->indexFromNodeFullName(blockFullname);
    QModelIndex index_proxy = sortFilter_->mapFromSource(index);
    if(!index_proxy.isValid()) {
        qDebug() <<"index_proxy is InValid" << endl;
    }
    ui->treeView->setCurrentIndex(index_proxy);
    QItemSelectionModel *_model = new QItemSelectionModel(sortFilter_);
    QItemSelection selection(index_proxy, index_proxy);
    _model->select(selection, QItemSelectionModel::SelectCurrent);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setSelectionModel(_model);
    ui->treeView->scrollTo(index_proxy);
    ui->treeView->setExpanded(index_proxy, true);
}

void ResourceMonitor::slot_treeViewPressed(const QModelIndex& index)
{
    Node* curNode = treeMode_->nodeFromIndex(sortFilter_->mapToSource(index));
    if(curNode->type_ != 2)
        return;

    QStringList fullnameList;
    QModelIndex index_src = index;
    while (index_src.parent().isValid()) {
        QString name = index_src.data().toString();
        fullnameList.insert(0, "/" + name);
        index_src = index_src.parent();
    }
    fullnameList.insert(0, "/" + index_src.data().toString());

    ui->widget->activeOneBlock(fullnameList.join("\n"));
}

void ResourceMonitor::on_pushButton_clearName_clicked()
{
    ui->comboBox_nameFilter->clearChecked();
}

void ResourceMonitor::on_pushButton_clearType_clicked()
{
    ui->comboBox_typeFilter->clearChecked();
}

void ResourceMonitor::on_comboBox_showMode_currentIndexChanged(int index)
{
    if(index == 0) {
        ui->widget->setShowMode(ChartMode);
    } else {
        ui->widget->setShowMode(TableMode);
    }
}

void ResourceMonitor::on_pushButton_red_toggled(bool checked)
{
    if(checked) {
        showLevel_ = (ShowLevel)(showLevel_ | RedLevel);
    } else {
        showLevel_ = (ShowLevel)(showLevel_ & ~RedLevel);
    }
    if(isInitialized_) {
        refreshUi();
    }
}

void ResourceMonitor::on_pushButton_yellow_toggled(bool checked)
{
    if(checked) {
        showLevel_ = (ShowLevel)(showLevel_ | YellowLevel);
    } else {
        showLevel_ = (ShowLevel)(showLevel_ & ~YellowLevel);
    }
    if(isInitialized_) {
        refreshUi();
    }
}

void ResourceMonitor::on_pushButton_green_toggled(bool checked)
{
    if(checked) {
        showLevel_ = (ShowLevel)(showLevel_ | GreenLevel);
    } else {
        showLevel_ = (ShowLevel)(showLevel_ & ~GreenLevel);
    }
    if(isInitialized_) {
        refreshUi();
    }
}

void ResourceMonitor::on_pushButton_refresh_clicked()
{
    isInitialized_ = false;

    {
        dataCenter_.setIcdAndXcdFilePath(dataCenter_.getIcdFilePath(), dataCenter_.getXcdFilePath());
        dataCenter_.setIwrp2FilePath(dataCenter_.getIwrp2FilePath());
    }

    refreshUi();
    isInitialized_ = true;
}
