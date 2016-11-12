#pragma once
#ifndef INPUTLIST_H
#define INPUTLIST_H
#include <QListWidget>
#include <qaction.h>
#include <qmenu.h>
#include <qevent.h>
#include <QFile>

class InputList : public QListWidget {
	Q_OBJECT

public:
	InputList(QWidget * parent = Q_NULLPTR);
	~InputList();

	static QString _lastDirectory;
protected:
	enum Actions
	{
		ActionNewVideo,
		ActionNewFolder,
		ActionRefresh,
		ActionDeleteVideo,
		NActions
	};
	QAction *_inputActions[NActions];
	void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
	
public slots:
	void AddInputItem();
	void addFolderSlot();
	void deleteVideo();

private:
	void createActions();
	void clearActions();
	void AddDir(const QString & name);

public:
	void AddInputItem(const QString & str);

};
#endif // INPUTLIST_H