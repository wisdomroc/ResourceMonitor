
#include "MultiSelectComboBox.h"
#include <QStandardItem>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>


#include <QCompleter>
MultiSelectComboBox::MultiSelectComboBox(QWidget* parent)
	: QComboBox(parent)
{
	setFixedWidth(200);
	setFixedHeight(60);
	//ȥ��������ͷ
//	setStyleSheet("QComboBox::drop-down{border-style:none;}");
	//���ÿɱ༭
	setEditable(true);
	//selectItemView��סQComboBox
	selectItemView = new QListView(this);
	selectItemView->setFixedHeight(30);
	//QListView{outline: none;} ȥ�������߿�                     ��������״̬��ѡ��Ϊ״̬item�ı���ɫ��ͬ
	selectItemView->setStyleSheet("QListView{outline: none;} QListView::item {background:#BDD7FD;} QListView::item:hover {background: #BDD7FD;}");
	selectItemView->setIconSize(QSize(12, 12));
	//���ø���ļ��
	selectItemView->setSpacing(3);
	//����Ϊ����������Ȼ������޸�������
	selectItemView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//���û����
	selectItemView->setResizeMode(QListView::Adjust);
	//����Ϊ�Զ�����
	selectItemView->setWrapping(true);
	//���ø�������
	selectItemView->setFlow(QListView::LeftToRight);
	//ˮƽ���������ò�����ʾ
	selectItemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//ѡ��ģʽ����Ϊ����ʾ
	selectItemView->setSelectionMode(QAbstractItemView::NoSelection);

	selectModel = new QStandardItemModel();
	selectItemView->setModel(selectModel);

	QVBoxLayout* lineLayout = new QVBoxLayout(this);
	lineLayout->setMargin(0);
	lineLayout->setSpacing(0);
	m_pLineEdit = new QLineEdit;
	m_pLineEdit->setFixedHeight(35);
	lineLayout->addWidget(m_pLineEdit);
	lineLayout->addWidget(selectItemView);

	//��װ�¼���������������xɾ��
	selectItemView->viewport()->installEventFilter(this);
	popupView = new QListView();
	popupModel = new QStandardItemModel();

	m_proxyModel = new QSortFilterProxyModel(popupView);
	m_proxyModel->setSourceModel(popupModel);
	m_proxyModel->setFilterRole(Qt::DisplayRole);
	m_proxyModel->setDynamicSortFilter(false);
	m_proxyModel->setFilterKeyColumn(0);
	popupView->setModel(m_proxyModel);

	setView(popupView);
	setModel(m_proxyModel);

	installEventFilter(this);
	//���������б�ĳ�е��źŴ�����
	connect(this, QOverload<const QString&>::of(&QComboBox::activated),
		[=](const QString& _displayStr) {
			bool hasExist = false;
			//��_displayStrΨһ��ʱ��ֱ��ȡ����Ϊ0��item
			QStandardItem* clickItem = popupModel->findItems(_displayStr)[0];
			QString text = clickItem->text();
			//�ж����Ƿ��Ѿ�ѡ����ѡ����ɾ��ѡ����
			for (int row = 0; row < selectModel->rowCount(); row++)
			{
				QStandardItem* item = selectModel->item(row);
				if (item->text() == text)
				{
					selectModel->removeRow(row);
					hasExist = true;
					break;
				}
			}
			if (!hasExist)
			{
				//ûѡ������ӣ����ı�ѡ���״̬
				clickItem->setCheckState(Qt::Checked);
                QStandardItem* item = new QStandardItem(QIcon(":/img/delete.png"), text);
				selectModel->appendRow(item);

				QString str = currentText().join(";");
				setCurrentText(str);

				emit signal_currentTextChanged(str);
			}
			else
			{
				//ѡ�й������Ϊδѡ��״̬
				clickItem->setCheckState(Qt::Unchecked);
				QString str = currentText().join(";");
				emit signal_currentTextChanged(str);
			}
		});
	connect(m_pLineEdit, &QLineEdit::textChanged, [=](const QString& _str) {
		setFilterFixedString(_str);
		});


	//frameΪ���������б��
	QFrame* frame = this->findChild<QFrame*>();
	if (frame)
	{
		frame->installEventFilter(this);
	}
	installEventFilter(this);

}

MultiSelectComboBox::~MultiSelectComboBox()
{

}
//���������ķ�����ѡ���ø��෽ʽ�����ÿ���check
void MultiSelectComboBox::addItem(const QString& text, const QVariant& userData)
{
	QStandardItem* item = new QStandardItem(text);
	item->setCheckable(true);
	popupModel->appendRow(item);
}

void MultiSelectComboBox::addItem(const QIcon& icon, const QString& text, const QVariant& userData)
{
	QComboBox::addItem(icon, text, userData);
}

void MultiSelectComboBox::addItems(const QStringList& texts)
{
	QComboBox::addItems(texts);
	for (int row = 0; row < popupModel->rowCount(); row++)
	{
		popupModel->item(row)->setCheckable(true);
	}
}

//��������ѡ��������ݣ���QStringList�洢
QStringList MultiSelectComboBox::currentText()
{
	QStringList items;
	for (int row = 0; row < popupModel->rowCount(); row++)
	{
		QStandardItem* item = popupModel->item(row);
		if (item->checkState() == Qt::Checked)
		{
			items << item->text();
		}
	}
	return items;
}
//��������ѡ�������������QList<int>�洢
QList<int> MultiSelectComboBox::currentIndex()
{
	QList<int> indexs;
	for (int row = 0; row < popupModel->rowCount(); row++)
	{
		QStandardItem* item = popupModel->item(row);
		if (item->checkState() == Qt::Checked)
		{
			indexs << row;
		}
	}
	return indexs;
}

void MultiSelectComboBox::setFilterFixedString(const QString& _strFilter)
{
	m_proxyModel->setFilterFixedString(_strFilter);
}

bool MultiSelectComboBox::eventFilter(QObject* watched, QEvent* event)
{
	QFrame* frame = this->findChild<QFrame*>();
	if (frame && frame == watched)
	{

		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint globalPos = mouseEvent->globalPos();
			QRect rect(frame->mapToGlobal(QPoint(0, 0)), QSize(frame->width(), frame->height()));
			QRect rect1(selectItemView->viewport()->mapToGlobal(QPoint(0, 0)), QSize(selectItemView->viewport()->width(), selectItemView->viewport()->height()));
			//���������б�����ʾѡ�����viewport��Χ�ڵİ���������ر�
			if (!rect.contains(globalPos) && !rect1.contains(globalPos))
			{
				isPermitHidePopup = true;

			}
			else
			{
				selectItemViewPress(selectItemView->viewport()->mapFromGlobal(globalPos));
			}

		}

	}

	if (watched == selectItemView->viewport())
	{
		//����selectItemView->viewport����갴���¼�
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			selectItemViewPress(mouseEvent->pos());
		}
	}
	//���λ����¼�
	if (watched == this)
	{
		if (event->type() == QEvent::Wheel)
		{
			return true;
		}

	}

	return QComboBox::eventFilter(watched, event);
}

void MultiSelectComboBox::showPopup()
{
	QComboBox::showPopup();
	isPermitHidePopup = false;
}

void MultiSelectComboBox::hidePopup()
{
	if (isPermitHidePopup)
	{
		QComboBox::hidePopup();
	}
}

void MultiSelectComboBox::selectItemViewPress(QPoint pos)
{
	QModelIndex index = selectItemView->indexAt(pos);
	//��Ч�����������ѡ����
	if (index.isValid())
	{
		QRect rect = selectItemView->visualRect(index);
		QSize iconSize = selectItemView->iconSize();
		//xͼ�������
		QRect iconRect = QRect(rect.left(), rect.top(), iconSize.width(), iconSize.height());
		//�����xͼ���������
		if (iconRect.contains(pos))
		{
			QStandardItem* selectItem = selectModel->itemFromIndex(index);
			QString selectStr = selectItem->text();
			QStandardItem* popupItem = popupModel->findItems(selectStr)[0];
			popupItem->setCheckState(Qt::Unchecked);
			//ɾ����
			selectModel->removeRow(index.row());
			QString str = currentText().join(";");
			emit signal_currentTextChanged(str);
		}

	}
	showPopup();
}
