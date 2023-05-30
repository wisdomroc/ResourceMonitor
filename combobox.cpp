#include "combobox.h"

#include <QCompleter>
#include <QMouseEvent>
#include <QDebug>

ComboBox::ComboBox(QWidget *parent) :
    QComboBox(parent)
{
    view()->viewport()->installEventFilter(this);
    setEditable(true);
    completer_ = new QCompleter(this);
    setCompleter(completer_);
}

bool ComboBox::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QModelIndex ind = view()->indexAt(((QMouseEvent*)event)->pos());
        bool checked = view()->model()->data(ind, Qt::CheckStateRole).toBool();
        view()->model()->setData(ind, !checked, Qt::CheckStateRole);
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void ComboBox::hidePopup()
{
    QStringList values;
    for (int i=0; i < count(); i++){
      if (itemData(i, Qt::CheckStateRole).toBool()){
        values << itemText(i);
      }
    }
    setCurrentText(values.join(_delimiter));

    QComboBox::hidePopup();
}

void ComboBox::showPopup()
{
    QStringList values = currentText().split(_delimiter);

    emit beforeOpen();

    for (int i=0; i<count(); i++){
      setItemData(i, values.contains(itemText(i)), Qt::CheckStateRole);
    }
    QComboBox::showPopup();
}
