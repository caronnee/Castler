#include "inputlist.h"
#include <qfiledialog.h>

InputList::InputList(QWidget * parent) : QListWidget(parent) {
	createActions();
}

InputList::~InputList() {
	clearActions();
}

QString InputList::_lastDirectory = "C:";

void InputList::deleteVideo()
{
	QListWidgetItem * item = takeItem( selectedIndexes()[0].row() );
	delete item;
}

void InputList::addVideo(const QString & str)
{
	QStringList lst = str.split('/');
	QString display = lst[lst.size() - 1];
	QListWidgetItem * item = new QListWidgetItem(str);
	item->setData(Qt::DisplayRole, display);
	item->setData(Qt::UserRole, str);
	addItem(item);
	this->setItemSelected(item, true);
}

void InputList::addVideo()
{
	QStringList files = QFileDialog::getOpenFileNames(
		this,
		"Select one or more video files to open",
		_lastDirectory,
		"Images (*.avi *.mpeg *.mp4)");
	if (files.count() == 0)
		return;
	_lastDirectory = files[0];
	for (int i = 0; i < files.size(); i++)
	{
		addVideo(files[i]);		
	}
}

void InputList::createActions()
{
	newAct[ActionNewVideo] = new QAction(tr("New video"), this);
	newAct[ActionNewVideo]->setStatusTip(tr("Add a video to the list"));
	newAct[ActionDeleteVideo] = new QAction(tr("Delete video"), this);
	newAct[ActionDeleteVideo]->setStatusTip(tr("Remove video from list"));
	connect(newAct[ActionNewVideo], SIGNAL(triggered(void)), this, SLOT(addVideo(void)));
	connect(newAct[ActionDeleteVideo], &QAction::triggered, this, &InputList::deleteVideo);
}

void InputList::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	for (int i = 0; i < NActions; i++)
	{
		menu.addAction(newAct[i]);
	}
	menu.exec(event->globalPos());
	event->accept();
}
void InputList::clearActions()
{
	for (int i = 0; i < NActions; i++)
		delete newAct[i];
}