#include "wcombobox.h"


WComboBox::WComboBox(QWidget *parent):QComboBox (parent)
{
    isModified_ = false;
    popupFlag_  = false;
    m_lineEdit  = new WLineEdit();
    m_completer = new QCompleter();
    m_model     = new QStandardItemModel();
    m_listView  = new QListView();
    m_completer->setPopup(m_listView);
    m_completer->setModel(m_model);
    m_lineEdit->setCompleter(m_completer);
    this->setLineEdit(m_lineEdit);
    this->setModel(m_model);
    this->setEditable(true);

    connect(m_completer, SIGNAL(activated(QString)), this, SLOT(slot_hideListView(QString)));
    connect(this, SIGNAL(activated(int)), this, SLOT(slot_activated(int)));
    connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(slot_editingFinished(QString)));
    connect(m_lineEdit, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(slot_cursorPositionChanged(int, int)));
    m_listView->viewport()->installEventFilter(this);
}

QStringList WComboBox::getCheckedUuidData()
{
    QStringList checkedData;
    for(int i = 0; i < m_model->rowCount(); i ++)
    {
        if(m_model->item(i)->checkState() == Qt::Checked) {
            checkedData.append(m_model->item(i)->data(UUID_ROLE).toString());
        }

    }
    return checkedData;
}

QStringList WComboBox::getCheckedToolTipData()
{
    QStringList checkedData;
    for(int i = 0; i < m_model->rowCount(); i ++)
    {
        if(m_model->item(i)->checkState() == Qt::Checked) {
            checkedData.append(m_model->item(i)->data(Qt::ToolTipRole).toString());
        }

    }
    return checkedData;
}

QStringList WComboBox::getCheckedData()
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

QMap<QString, int> WComboBox::getCheckStates()
{

        QMap<QString, int> checkStates;
        for(int i = 0; i < m_model->rowCount(); i ++)
        {
            checkStates.insert(m_model->item(i)->text(), m_model->item(i)->checkState() == Qt::Checked ? 0 : 1);
        }
        return checkStates;

}

void WComboBox::addItems(const QMultiHash<QString, BlockInfoPtr>& hash)
{
    for(auto itor = hash.begin(); itor != hash.end(); ++itor)
    {
        QStandardItem *item = new QStandardItem(itor.key());
        item->setCheckState(Qt::Unchecked);
        QString fullname = itor.value()->namepath;
        item->setToolTip(fullname);
        item->setData(itor.value()->uuid, UUID_ROLE);
        m_model->appendRow(item);
    }
}

void WComboBox::addItems(const QStringList &labels)
{
    for(auto i: labels)
    {
        QStandardItem *item = new QStandardItem(i);
        item->setCheckState(Qt::Unchecked);
        m_model->appendRow(item);
    }
}

void WComboBox::clearChecked()
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

void WComboBox::setChecked(const QStringList &checkedList)
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

void WComboBox::updateText()
{
    QStringList checkedLabels;
    for(int i = 0; i < m_model->rowCount(); i ++)
    {
        if(m_model->item(i)->checkState() == Qt::Checked)
        {
            checkedLabels.append(m_model->item(i)->text());
        }
    }
    disconnect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(slot_editingFinished(QString)));
    m_lineEdit->setText(checkedLabels.join(","));
    connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(slot_editingFinished(QString)));
    m_lineEdit->setToolTip(checkedLabels.join(","));
}

void WComboBox::slot_activated(int index)
{
    qDebug() << "listview activated" << endl;
    QStandardItem *cur_item = m_model->item(index);
    if(!cur_item) return;

    Qt::CheckState state = cur_item->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked;
    cur_item->setCheckState(state);
    updateText();
    isModified_ = true;
}

void WComboBox::slot_hideListView(QString str)
{
    qDebug() << "hide Event called" << endl;
    m_completer->popup();
}

void WComboBox::slot_editingFinished(QString str)
{
    if(!popupFlag_) {
        updateText();
        qDebug() << "edit finished" << endl;
    }
}

void WComboBox::hidePopup()
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

bool WComboBox::eventFilter(QObject * watched, QEvent * event)
{
    if(event->type() == QEvent::Show)
    {
        popupFlag_ = true;
        qDebug() << "popup show" << endl;
    }
    if (event->type() == QEvent::Hide)
    {
        qDebug() <<"popup hide" << endl;
        updateText();
        if(isModified_)
        {
            emit done();
            isModified_ = false;
        }
        popupFlag_ = false;
    }
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QModelIndex ind = m_completer->popup()->indexAt(((QMouseEvent*)event)->pos());

        QString uuid = m_completer->popup()->model()->data(ind, UUID_ROLE).toString();
        QModelIndex rootIndex = m_model->invisibleRootItem()->index();
        int findRow = -1;
        for(int i = 0; i < m_model->rowCount(); ++i) {
            QString str = m_model->index(i, 0, rootIndex).data(UUID_ROLE).toString();
            if(str == uuid) {
                findRow = i;
                break;
            }
        }

        if(findRow != -1) {

            QString str   = m_completer->popup()->model()->data(ind, Qt::DisplayRole).toString();
            bool checked1 = m_completer->popup()->model()->data(ind, Qt::CheckStateRole).toBool();
            if(checked1)
            {
                //! step1
                m_completer->popup()->model()->setData(ind, Qt::Unchecked , Qt::CheckStateRole);
                //! step2
                QStandardItem *findItem = m_model->item(findRow);
                findItem->setCheckState(Qt::Unchecked);
            }
            else
            {
                //! step1
                m_completer->popup()->model()->setData(ind, Qt::Checked , Qt::CheckStateRole);
                //! step1
                QStandardItem *findItem = m_model->item(findRow);
                findItem->setCheckState(Qt::Checked);
            }

            isModified_ = true;
        }
        else
        {
            qDebug() << "will never be reached" << endl;
        }
        return true;
    }
    return QObject::eventFilter(watched, event);
}
