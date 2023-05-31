#include "percentunit.h"
#include "ui_Percentunit.h"
#include <QDebug>
#include <QScrollBar>
#include <QResizeEvent>
#include <QAction>

PercentUnit::PercentUnit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PercentUnit),
    flowLayout_(new FlowLayout(this)),
    canShowMenu_(false),
    currentBlock_(nullptr)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    initConnections();
    initUi();
}

PercentUnit::~PercentUnit()
{
    delete ui;
}

void PercentUnit::activeOneBlock(const QString &uuid)
{
    for(auto block : blockList_) {
        QString blockUuid = block->getBlockUuid();
        if(blockUuid == uuid) {
            block->setActive(true);
            QPoint positionPos = block->mapToParent(QPoint(0,0));
            ui->scrollArea->verticalScrollBar()->setValue(positionPos.y());//scrollTo the current block
        } else {
            block->setActive(false);
        }
    }
}

void PercentUnit::addBlockPercent(const QString &name, const QString &uuid, QMap<QString, InfoTuple> icdPercents)
{
    OneBlock *block = new OneBlock(name, uuid, icdPercents);
    blockList_.append(block);

    flowLayout_->addWidget(block);
}

void PercentUnit::clearAllBlock()
{
    int itemCount = flowLayout_->count();
    for(int i = itemCount - 1; i >= 0; --i) {
        QWidget *widget = flowLayout_->itemAt(i)->widget();
        flowLayout_->removeWidget(widget);
        widget->deleteLater();
    }

    blockList_.clear();
}

void PercentUnit::resizeEvent(QResizeEvent *event)
{
    ui->scrollArea->resize(event->size());
    ui->scrollAreaWidgetContents->resize(event->size());
}

void PercentUnit::mousePressEvent(QMouseEvent *event)
{
    canShowMenu_ = false;
    QPoint point = event->pos();
    point.ry() += ui->scrollArea->verticalScrollBar()->value();
    qDebug() << "point---->" << point << endl;
    for(auto one : blockList_) {
        QRect rect = one->rect();
        QPoint offset_point = one->mapToParent(one->rect().topLeft());
        rect.translate(offset_point);
        if(rect.contains(point)) {
            one->setActive(true);
            QString uuid = one->getBlockUuid();
            emit activeBlock(uuid);
            canShowMenu_ = true;
            currentBlock_ = one;
        } else {
            one->setActive(false);
        }
    }
}

void PercentUnit::contextMenuEvent(QContextMenuEvent *event)
{
    if(canShowMenu_) {
        canShowMenu_ = false;
        menu_.popup(mapToGlobal(event->pos()));
    }
}

void PercentUnit::initUi()
{
    ui->scrollArea->setStyleSheet("QScrollBar:horizontal{background:transparent; height:10px;}\
                                   QScrollBar:vertical{background:transparent; width: 10px;}\
                                   QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}\
                                   QScrollBar::handle:hover{background:gray;}\
                                   QScrollBar::sub-line{background:transparent;}\
                                   QScrollBar::add-line{background:transparent;}\
                                   QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}\
                                   QScrollBar::handle:hover{background:gray;}\
                                   QScrollBar::sub-line{background:transparent;}\
                                   QScrollBar::add-line{background:transparent;}");


    ui->scrollAreaWidgetContents->setLayout(flowLayout_);

    QAction* action_top = menu_.addAction(QIcon(":/img/totop.png"), QStringLiteral("放到最前"));
    connect(action_top, SIGNAL(triggered(bool)), this, SLOT(slot_top(bool)));
}

void PercentUnit::initConnections()
{
}

void PercentUnit::slot_top(bool checked)
{
    if(currentBlock_) {
        blockList_.removeOne(currentBlock_);
        blockList_.insert(0, currentBlock_);

        int itemCount = flowLayout_->count();
        for(int i = itemCount - 1; i >= 0; --i) {
            QWidget *widget = flowLayout_->itemAt(i)->widget();
            flowLayout_->removeWidget(widget);
        }

        for(auto& block : blockList_) {
            flowLayout_->addWidget(block);
        }
    }
}
