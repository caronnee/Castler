#include "inputlist.h"
#include <qfiledialog.h>

InputList::InputList(QWidget * parent) : QListWidget(parent) {
	createActions();
}

InputList::~InputList() {
	clearActions();
}

void InputList::Load(QFile & loader)
{
	QByteArray arr = loader.readAll();
	QByteArrayList l = arr.split('@');
	_lastDirectory = QString(l[0]);
	for (int i = 1; i < l.count(); i++)
	{
		addItem(l[i]);
	}
}
void InputList::Save(QFile & writer)
{
	writer.write(_lastDirectory.toStdString().c_str());
	char  delim = '@';
	writer.write(&delim,1);
	for (int i = 0; i < count(); i++)
	{
		QListWidgetItem * it = item(i);
		QByteArray str = it->data(0).toByteArray();
		size_t written = writer.write(str);
		writer.write(&delim,1);
	}
}

QString InputList::_lastDirectory = "C:";

void InputList::deleteVideo()
{
	removeItemWidget(takeItem( selectedIndexes()[0].row() ));
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
		addItem(files[i]);
	}
}

void InputList::createActions()
{
	newAct[ActionNewVideo] = new QAction(tr("New video"), this);
	newAct[ActionNewVideo]->setStatusTip(tr("Add a video to the list"));
	newAct[ActionDeleteVideo] = new QAction(tr("Delete video"), this);
	newAct[ActionDeleteVideo]->setStatusTip(tr("Remove video from list"));
	connect(newAct[ActionNewVideo], &QAction::triggered, this, &InputList::addVideo);
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