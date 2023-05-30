
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
	//去掉下拉箭头
//	setStyleSheet("QComboBox::drop-down{border-style:none;}");
	//设置可编辑
	setEditable(true);
	//selectItemView盖住QComboBox
	selectItemView = new QListView(this);
	selectItemView->setFixedHeight(30);
	//QListView{outline: none;} 去掉项虚线框                     设置正常状态和选中为状态item的背景色相同
	selectItemView->setStyleSheet("QListView{outline: none;} QListView::item {background:#BDD7FD;} QListView::item:hover {background: #BDD7FD;}");
	selectItemView->setIconSize(QSize(12, 12));
	//设置各项的间隔
	selectItemView->setSpacing(3);
	//设置为不触发，不然点击可修改项内容
	selectItemView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//设置会调整
	selectItemView->setResizeMode(QListView::Adjust);
	//设置为自动换行
	selectItemView->setWrapping(true);
	//设置浮动方向
	selectItemView->setFlow(QListView::LeftToRight);
	//水平滚动条设置不不显示
	selectItemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//选择模式设置为不显示
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

	//安装事件过滤器，处理点击x删除
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
	//激活下拉列表某行的信号处理函数
	connect(this, QOverload<const QString&>::of(&QComboBox::activated),
		[=](const QString& _displayStr) {
			bool hasExist = false;
			//当_displayStr唯一的时候直接取索引为0的item
			QStandardItem* clickItem = popupModel->findItems(_displayStr)[0];
			QString text = clickItem->text();
			//判断项是否已经选过，选过则删除选中项
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
				//没选过则添加，并改变选项的状态
				clickItem->setCheckState(Qt::Checked);
                QStandardItem* item = new QStandardItem(QIcon(":/img/delete.png"), text);
				selectModel->appendRow(item);

				QString str = currentText().join(";");
				setCurrentText(str);

				emit signal_currentTextChanged(str);
			}
			else
			{
				//选中过点击改为未选中状态
				clickItem->setCheckState(Qt::Unchecked);
				QString str = currentText().join(";");
				emit signal_currentTextChanged(str);
			}
		});
	connect(m_pLineEdit, &QLineEdit::textChanged, [=](const QString& _str) {
		setFilterFixedString(_str);
		});


	//frame为弹出下拉列表框
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
//所有添加项的方法都选调用父类方式再设置可以check
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

//返回所有选中项的内容，用QStringList存储
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
//返回所有选中项的索引，用QList<int>存储
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
			//不在下拉列表框和显示选项项的viewport范围内的按下则允许关闭
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
		//处理selectItemView->viewport的鼠标按下事件
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			selectItemViewPress(mouseEvent->pos());
		}
	}
	//屏蔽滑轮事件
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
	//有效，则该坐标有选中项
	if (index.isValid())
	{
		QRect rect = selectItemView->visualRect(index);
		QSize iconSize = selectItemView->iconSize();
		//x图标的区域
		QRect iconRect = QRect(rect.left(), rect.top(), iconSize.width(), iconSize.height());
		//鼠标在x图标的区域内
		if (iconRect.contains(pos))
		{
			QStandardItem* selectItem = selectModel->itemFromIndex(index);
			QString selectStr = selectItem->text();
			QStandardItem* popupItem = popupModel->findItems(selectStr)[0];
			popupItem->setCheckState(Qt::Unchecked);
			//删除项
			selectModel->removeRow(index.row());
			QString str = currentText().join(";");
			emit signal_currentTextChanged(str);
		}

	}
	showPopup();
}
