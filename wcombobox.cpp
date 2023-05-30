#include "wcombobox.h"


WComboBox::WComboBox(QWidget *parent):QComboBox (parent)
{
    m_lineEdit  = new QLineEdit();
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
    m_listView->viewport()->installEventFilter(this);
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
    m_lineEdit->setText(checkedLabels.join(","));
    m_lineEdit->setToolTip(checkedLabels.join(","));
}

void WComboBox::slot_activated(int index)
{
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
    if(str == "")
    {
        for (int i=0; i < m_completer->popup()->model()->rowCount(); i++){
            QModelIndex index = m_completer->popup()->model()->index(i, 0, QModelIndex());
            m_completer->popup()->model()->setData(index, Qt::Unchecked , Qt::CheckStateRole);
        }
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
        QStringList values;
        for (int i=0; i < m_completer->popup()->model()->rowCount(); i++){
            QModelIndex index = m_completer->popup()->model()->index(i, 0, QModelIndex());
            qDebug() << index.data(Qt::DisplayRole).toString() << endl;
          if (m_completer->popup()->model()->data(index, Qt::CheckStateRole).toBool()){
            values << index.data(Qt::DisplayRole).toString();
          }
        }
        if(!values.isEmpty()) {
            setCurrentText(values.join(","));
        }


        QComboBox::hidePopup();
        if(isModified_) {
            isModified_ = false;
            emit done();
        }
    }
}

bool WComboBox::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QModelIndex ind = m_completer->popup()->indexAt(((QMouseEvent*)event)->pos());

        QString tooltip = m_completer->popup()->model()->data(ind, Qt::ToolTipRole).toString();
        qDebug() <<"tooltip:" << tooltip << endl;
        qDebug() << "model.rowCount: " << m_model->rowCount() << endl;
        QModelIndex rootIndex = m_model->invisibleRootItem()->index();
        int findRow = -1;
        for(int i = 0; i < m_model->rowCount(); ++i) {
            QString str = m_model->index(i, 0, rootIndex).data(Qt::ToolTipRole).toString();
            if(str == tooltip) {
                findRow = i;
                break;
            }
        }
        if(findRow != -1) {
            //! step1
            bool checked1 = m_completer->popup()->model()->data(ind, Qt::CheckStateRole).toBool();
            QString str   = m_completer->popup()->model()->data(ind, Qt::DisplayRole).toString();
            if(checked1)
            {
                m_completer->popup()->model()->setData(ind, Qt::Unchecked , Qt::CheckStateRole);

            }
            else
            {
                m_completer->popup()->model()->setData(ind, Qt::Checked , Qt::CheckStateRole);

            }

            //! step2, 不使用貌似也可以？
//            QStandardItem *findItem = m_model->item(findRow);
//            findItem->setCheckState(findItem->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
        }
        return true;
    }
    return QObject::eventFilter(watched, event);
}
