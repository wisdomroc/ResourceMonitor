#ifndef MULTISELECTCOMBOBOX_H
#define MULTISELECTCOMBOBOX_H

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QLineEdit>

class MultiSelectComboBox : public QComboBox
{
	Q_OBJECT

public:
	MultiSelectComboBox(QWidget* parent = nullptr);
	~MultiSelectComboBox() override;
	void addItem(const QString& text, const QVariant& userData = QVariant());
	void addItem(const QIcon& icon, const QString& text, const QVariant& userData = QVariant());
	void addItems(const QStringList& texts);
	QStringList currentText();
	QList<int> currentIndex();
	void setFilterFixedString(const QString& _strFilter);

signals:
	//qcombox 的currentTextChanged不知道为啥每次只发送一个值，所以我自定义了一个信号
	void signal_currentTextChanged(const QString&_str);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void showPopup() override;
	void hidePopup() override;
private:
	bool isPermitHidePopup;
	QListView* selectItemView;
	QStandardItemModel* selectModel;
	QListView* popupView;
	QStandardItemModel* popupModel;
	void selectItemViewPress(QPoint pos);
	QSortFilterProxyModel* m_proxyModel;
	QLineEdit* m_pLineEdit;

};

#endif // MULTISELECTCOMBOBOX_H