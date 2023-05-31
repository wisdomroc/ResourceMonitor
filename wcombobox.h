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
#include <QCompleter>
#include <QLineEdit>
#include <QStandardItemModel>
#include "global.h"

class WLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    WLineEdit(QWidget *parent = nullptr):QLineEdit(parent)
    {

    }

protected:
    void focusInEvent(QFocusEvent *event) {
        emit focusIn();
    };

signals:
    void focusIn();


};

class WComboBox : public QComboBox
{
    Q_OBJECT
public:
    WComboBox(QWidget *parent = nullptr);

    QStringList getCheckedUuidData();
    QStringList getCheckedToolTipData();
    QStringList getCheckedData();
    QMap<QString,int> getCheckStates();

    void addItems(const QMultiHash<QString, BlockInfoPtr>& hash);
    void addItems(const QStringList &labels);
    void clearChecked();
    void setChecked(const QStringList &checkedList);

protected:

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
//    void showPopup() override;
    void hidePopup() override;

private:
    void updateText();

private slots:
    void slot_activated(int index);
    void slot_hideListView(QString str);
    void slot_editingFinished(QString str);
//    void slot_cursorPositionChanged(int old, int new);

signals:
    void done();

private:
    WLineEdit               *m_lineEdit;
    QCompleter              *m_completer;
    QListView               *m_listView;
    QStandardItemModel      *m_model;
    bool                    isModified_;
    bool                    isPermitHidePopup;
    QStringList             selectedStrList;
    bool                    popupFlag_;
};

#endif // WCOMBOBOX_H
