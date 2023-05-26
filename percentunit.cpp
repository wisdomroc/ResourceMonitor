#include "percentunit.h"
#include "ui_Percentunit.h"
#include <QDebug>
#include <QScrollBar>
#include <QResizeEvent>

PercentUnit::PercentUnit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PercentUnit),
    flowLayout_(new FlowLayout(this))
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

void PercentUnit::activeOneBlock(const QString &blockpath)
{
    for(auto block : blockList_) {
        QString fullname = block->getBlockFullName();
        if(fullname == blockpath) {
            block->setActive(true);
            QPoint positionPos = block->mapToParent(QPoint(0,0));
            ui->scrollArea->verticalScrollBar()->setValue(positionPos.y());//scrollTo the current block
        } else {
            block->setActive(false);
        }
    }
}

void PercentUnit::addBlockPercent(const QString &name, QMap<QString, InfoTuple> icdPercents)
{
    OneBlock *block = new OneBlock(name, icdPercents);
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
    QPoint point = event->pos();
    point.ry() += ui->scrollArea->verticalScrollBar()->value();
    qDebug() << "point---->" << point << endl;
    for(auto one : blockList_) {
        QRect rect = one->rect();
        QPoint offset_point = one->mapToParent(one->rect().topLeft());
        rect.translate(offset_point);
        if(rect.contains(point)) {
            one->setActive(true);
            QString blockFullname = one->getBlockFullName();
            emit activeBlock(blockFullname);
        } else {
            one->setActive(false);
        }
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
}

void PercentUnit::initConnections()
{
}
