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

void InputList::AddInputItem(const QString & str)
{
	QStringList lst = str.split('/');
	QString display = lst[lst.size() - 1];
	QListWidgetItem * item = new QListWidgetItem(str);
	item->setData(Qt::DisplayRole, display);
	item->setData(Qt::UserRole, str);
	addItem(item);
	this->setItemSelected(item, true);
}

void InputList::AddInputItem()
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
		AddInputItem(files[i]);		
	}
}

#include "imageprovider.h"

void InputList::AddDir(const QString & name)
{
	AddInputItem(name);
	/*QDir dir(name);
	QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	foreach(QFileInfo finfo, list) {
		if (finfo.isDir())
			continue;
		if ( IsSupportedFile(finfo.absoluteFilePath()) )
			AddInputItem(finfo.absoluteFilePath());
	}*/
}

void InputList::addFolderSlot()
{
	QString directory = QFileDialog::getExistingDirectory(
		this, "Select directory",
		_lastDirectory
	);
	if (directory.isEmpty())
		return;
	//for each file in directory ( non-recursive, add it do thelist )
	AddDir(directory);
}

void InputList::createActions()
{
	_inputActions[ActionNewVideo] = new QAction(tr("New video"), this);
	_inputActions[ActionNewVideo]->setStatusTip(tr("Add a video to the list"));
	connect(_inputActions[ActionNewVideo], SIGNAL(triggered(void)), this, SLOT(AddInputItem(void)));

	_inputActions[ActionNewFolder] = new QAction(tr("Add directory"), this);
	_inputActions[ActionNewFolder]->setStatusTip(tr("Add directory content to the list"));
	connect(_inputActions[ActionNewFolder], SIGNAL(triggered(void)), this, SLOT(addFolderSlot(void)));


	_inputActions[ActionRefresh] = new QAction(tr("Refresh"), this);
	_inputActions[ActionRefresh]->setStatusTip(tr("Refresh"));
	
	_inputActions[ActionDeleteVideo] = new QAction(tr("Delete video"), this);
	_inputActions[ActionDeleteVideo]->setStatusTip(tr("Remove video from list"));
	connect(_inputActions[ActionDeleteVideo], &QAction::triggered, this, &InputList::deleteVideo);
}

void InputList::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	for (int i = 0; i < NActions; i++)
	{
		menu.addAction(_inputActions[i]);
	}
	menu.exec(event->globalPos());
	event->accept();
}
void InputList::clearActions()
{
	for (int i = 0; i < NActions; i++)
		delete _inputActions[i];
}