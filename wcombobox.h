/**
* @filename wcombobox.h
* @brief    自定义ComboBox类，用于多选过滤
* @author   wanghp
* @date     2023-05-18
*/

#ifndef WCOMBOBOX_H
#define WCOMBOBOX_H
#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>

class WComboBox : public QComboBox
{
    Q_OBJECT
public:
    WComboBox(QWidget *parent = nullptr)
        : QComboBox (parent), isModified_(false)
    {
        m_lineEdit = new QLineEdit();
        m_lineEdit->setReadOnly(true);
        m_listView = new QListView();
        m_model = new QStandardItemModel();
        this->setLineEdit(m_lineEdit);
        this->setView(m_listView);
        this->setModel(m_model);
        connect(this, SIGNAL(activated(int)), this, SLOT(slot_activated(int)));
    }

    QStringList getCheckedData()
    {
        QStringList checkedData;
        for(int i = 0; i < m_model->rowCount(); i ++)
        {
            if(m_model->item(i)->checkState() == Qt::Checked) {
                checkedData.append(m_model->item(i)->text());
            }

        }
        return checkedData;
    }

    QMap<QString,int> getCheckStates()
    {
        QMap<QString, int> checkStates;
        for(int i = 0; i < m_model->rowCount(); i ++)
        {
            checkStates.insert(m_model->item(i)->text(), m_model->item(i)->checkState() == Qt::Checked ? 0 : 1);
        }
    }

    void addItems(const QStringList &labels)
    {
        for(auto i: labels)
        {
            QStandardItem *item = new QStandardItem(i);
            item->setCheckState(Qt::Unchecked);
            m_model->appendRow(item);
        }
    }

    void clearChecked()
    {
        for(int i = 0; i < m_model->rowCount(); i ++)
        {
            QString curText = m_model->item(i)->text();
            m_model->item(i)->setCheckState(Qt::Unchecked);
        }
        this->clearEditText();

        isModified_ = false;
        emit done();
    }

    void setChecked(const QStringList &checkedList)
    {
        for(int i = 0; i < m_model->rowCount(); i ++)
        {
            QString curText = m_model->item(i)->text();
            if(checkedList.contains(curText))
            {
                m_model->item(i)->setCheckState(Qt::Checked);
            }
        }
    }

protected:
    void hidePopup()
    {
        int width = this->view()->width();
        int height = this->view()->height();
        QRect rect(0, this->height(), width, height);
        int x = QCursor::pos().x() - mapToGlobal(this->geometry().topLeft()).x() + this->geometry().x();
        int y = QCursor::pos().y() - mapToGlobal(this->geometry().topLeft()).y() + this->geometry().y();
        if(!rect.contains(x, y))
        {
            QComboBox::hidePopup();
            if(isModified_) {
                isModified_ = false;
                emit done();
            }
        }
    }

private:
    void updateText()
    {
        QStringList checkedLabels;
        for(int i = 0; i < m_model->rowCount(); i ++)
        {
            if(m_model->item(i)->checkState() == Qt::Checked)
            {
                checkedLabels.append(m_model->item(i)->text());
            }
        }
        m_lineEdit->setText(checkedLabels.join(","));
        m_lineEdit->setToolTip(checkedLabels.join(","));
    }

private slots:
    void slot_activated(int index)
    {
        QStandardItem *cur_item = m_model->item(index);
        if(!cur_item) return;

        Qt::CheckState state = cur_item->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked;
        cur_item->setCheckState(state);

        updateText();
        isModified_ = true;
    }

signals:
    void done();

private:
    QLineEdit *m_lineEdit;
    QListView *m_listView;
    QStandardItemModel *m_model;
    bool      isModified_;
};

#endif // WCOMBOBOX_H
