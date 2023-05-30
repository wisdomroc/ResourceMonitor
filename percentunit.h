/**
* @filename percentunit.h
* @brief    PercentUnit 为整个资源占比显示单元，由QScrollBar和流式布局进行承载
*           OneBlock    为多个种类ICD的百分比显示单元
*           OnePercent  为一个单独的百分比显示单元
* @author   wanghp
* @date     2023-05-10
*/

#ifndef PERCENTUNIT_H
#define PERCENTUNIT_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include "flowlayout.h"
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include "callout.h"
#include "global.h"

namespace Ui {
class PercentUnit;
}

QT_CHARTS_USE_NAMESPACE

class OnePercent : public QFrame
{
    Q_OBJECT
public:
    explicit OnePercent(const QString& name, const InfoTuple& percent, QWidget *parent = nullptr): QFrame (parent), name_(name), percent_(percent),m_tooltip(nullptr){

        QLineSeries *series = new QLineSeries();
        int value = static_cast<int>(std::get<0>(percent).toDouble()*100);
        realSize_ = std::get<1>(percent);
        customSize_ = std::get<2>(percent);
        series->append(0,  value);
        series->append(10, value);

        chart_ = new QChart();
        chart_->addSeries(series);
        chart_->legend()->hide();
        chart_->createDefaultAxes();

        QValueAxis *axisX = new QValueAxis();
        axisX->setLabelFormat("f");
        chart_->setAxisX(axisX);
        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0.0, 100.0);
        if(value > 100.0) {
            axisY->setRange(0.0, value);
        }
        axisY->setTickCount(5);
        axisY->setLabelFormat("%d%");//%.2f
        chart_->setAxisY(axisY);
        series->attachAxis(axisX);
        series->attachAxis(axisY);

        chart_->setTitle(name_);
        QFont font = QApplication::font();
        font.setBold(true);
        chart_->setTitleFont(font);

        QChartView *chartView = new QChartView(chart_);
        chartView->setRenderHint(QPainter::Antialiasing);

        hBoxLayout_.addWidget(chartView);
        hBoxLayout_.setContentsMargins(0, 0, 0, 0);
        hBoxLayout_.setSpacing(0);
        this->setObjectName("OnePercent");
        this->setLayout(&hBoxLayout_);
        this->setStyleSheet("#OnePercent{border: 0px solid blue;}");
        this->setMinimumSize(260, 260);
        connect(series, &QLineSeries::hovered, this, &OnePercent::tooltip);
    }

    void setTheme(QChart::ChartTheme theme) {
        chart_->setTheme(theme);
        QFont font = QApplication::font();
        font.setBold(true);
        chart_->setTitleFont(font);
    }

    QString getPercentName(){
        return name_;
    }

    ~OnePercent() {
        if(chart_) {
            delete chart_;
            chart_ = nullptr;
        }
    }

protected:

signals:
    void deletePercent(const QString& name);

private slots:
    void tooltip(QPointF point, bool state) {
        if (m_tooltip == 0)
            m_tooltip = new Callout(chart_);

        if (state) {
            m_tooltip->setText(QStringLiteral("分配缓存: %1 \n实际缓存: %2 ").arg(realSize_).arg(customSize_));
            m_tooltip->setAnchor(point);
            m_tooltip->setZValue(11);
            m_tooltip->updateGeometry();
            m_tooltip->show();
        } else {
            m_tooltip->hide();
        }
    }

private:
    QString     name_;
    InfoTuple     percent_;
    QHBoxLayout hBoxLayout_;
    QChart      *chart_;
    Callout     *m_tooltip;
    int         realSize_;
    int         customSize_;
};

class OneBlock : public QFrame
{
    Q_OBJECT
public:
    explicit OneBlock(const QString &name, const QMap<QString, InfoTuple>& icdPercents, QWidget *parent = nullptr): QFrame (parent), name_(name), icdPercents_(icdPercents){
        setObjectName("BorderFrame");
        setStyleSheet("#BorderFrame { border: 2px solid gray; }");
        QHBoxLayout *hBoxLayout = new QHBoxLayout(this);
        hBoxLayout->setMargin(6);
        QLabel *label = new QLabel(this);
        label->setFocusPolicy(Qt::ClickFocus);
        label->setMinimumHeight(260);
        label->setText(name);
        QFont font = QApplication::font();
        font.setBold(true);
        label->setFont(font);
        hBoxLayout->addWidget(label);
        for(auto icd = icdPercents.begin(); icd != icdPercents.end(); ++icd) {
            OnePercent *onePercent = new OnePercent(icd.key(), icd.value(), this);
            hBoxLayout->addWidget(onePercent);
            percentList_.append(onePercent);
        }
        initTableWidget();
        hBoxLayout->addWidget(tableWidget_);
    }

    ~OneBlock() {
        int itemCount = hBoxLayout_.count();
        for(int i = itemCount - 1; i >= 0; i --) {
            QWidget* widget = hBoxLayout_.itemAt(i)->widget();
            hBoxLayout_.removeWidget(widget);
            widget->deleteLater();
        }
    }

    void setShowMode(ShowMode showMode) {
        showMode_ = showMode;
        if(showMode_ == ChartMode) {
            tableWidget_->setVisible(false);
            for(auto percent : percentList_) {
                percent->setVisible(true);
            }
        } else {
            tableWidget_->setVisible(true);
            for(auto percent : percentList_) {
                percent->setVisible(false);
            }
        }
    }

    void initTableWidget() {
        tableWidget_ = new QTableWidget();
        tableWidget_->setMinimumSize(420, 260);
        tableWidget_->setColumnCount(4);
        tableWidget_->setHorizontalHeaderLabels(QStringList() << QStringLiteral("类型") << QStringLiteral("实际缓存") << QStringLiteral("分配缓存") << QStringLiteral("占空比"));
        tableWidget_->setRowCount(icdPercents_.size());
        tableWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableWidget_->horizontalHeader()->setStretchLastSection(true);
        int i = 0;
        for(auto icd = icdPercents_.begin(); icd != icdPercents_.end(); ++icd) {
            QString name = icd.key();
            InfoTuple value = icd.value();
            double percent = std::get<0>(value).toDouble();
            QTableWidgetItem *item0 = new QTableWidgetItem(name);
            QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(std::get<1>(value)));
            QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(std::get<2>(value)));
            QTableWidgetItem *item3 = new QTableWidgetItem(std::get<0>(value));
            item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);
            item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
            item2->setFlags(item2->flags() & ~Qt::ItemIsEditable);
            item3->setFlags(item3->flags() & ~Qt::ItemIsEditable);
            tableWidget_->setItem(i, 0, item0);
            tableWidget_->setItem(i, 1, item1);
            tableWidget_->setItem(i, 2, item2);
            tableWidget_->setItem(i, 3, item3);
            QColor bgColor;
            if(1.0 - percent > PRECISION) {
                bgColor = "green";
            } else if(qAbs(percent-1.0) < PRECISION) {
                bgColor = "yellow";
            } else if(percent - 1.0 > PRECISION){
                bgColor = "red";
            }
            item0->setBackgroundColor(bgColor);
            item1->setBackgroundColor(bgColor);
            item2->setBackgroundColor(bgColor);
            item3->setBackgroundColor(bgColor);
            ++i;
        }

    }

    void setActive(bool active) {
        if(active) {
            setStyleSheet("#BorderFrame { border: 2px solid blue; }");
        } else {
            setStyleSheet("#BorderFrame { border: 2px solid gray; }");
        }
    }

    void setTheme(QChart::ChartTheme theme) {
        for(auto percent : percentList_) {
            percent->setTheme(theme);
        }
    }

    QString getBlockFullName() {
        int index_left_bracket  = name_.indexOf("\n(");
        int index_right_bracket = name_.indexOf("\n)");
        QString fullname = name_.mid(index_left_bracket + 3, index_right_bracket - index_left_bracket - 3);
        return fullname;
    }

    QList<OnePercent*> getPercentList() {
        return percentList_;
    }

private:
    QString                 name_;
    QMap<QString, InfoTuple>  icdPercents_;
    QHBoxLayout             hBoxLayout_;
    QList<OnePercent*>      percentList_;
    QTableWidget*           tableWidget_;
    ShowMode                showMode_;
};


class PercentUnit : public QWidget
{
    Q_OBJECT

public:
    explicit PercentUnit(QWidget *parent = nullptr);
    ~PercentUnit();

    void setShowMode(ShowMode showMode) {
        showMode_ = showMode;
        for(auto block : blockList_) {
            block->setShowMode(showMode_);
        }
    }

    void setTheme(QChart::ChartTheme theme) {
        for(auto block : blockList_) {
            block->setTheme(theme);
        }
    }

    void activeOneBlock(const QString& blockpath);

    void addBlockPercent(const QString &name, QMap<QString, InfoTuple> icdPercents);
    void clearAllBlock();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void initUi();
    void initConnections();

signals:
    void activeBlock(const QString& blockFullname);

private slots:
    void slot_top(bool checked);

private:
    Ui::PercentUnit     *ui;
    FlowLayout          *flowLayout_;
    QList<OneBlock *>   blockList_;
    ShowMode            showMode_;
    QMenu               menu_;
    bool                canShowMenu_;
    OneBlock*           currentBlock_;
};

#endif // PERCENTUNIT_H
