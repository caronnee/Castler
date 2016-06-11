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
	void Load(QFile & loader);
	void Save(QFile & writer);

	static QString _lastDirectory;
protected:
	enum Actions
	{
		ActionNewVideo,
		ActionDeleteVideo,
		NActions
	};
	QAction *newAct[NActions];
	void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
	void addVideo(const QString & str);

private slots:
	void addVideo();
	void deleteVideo();

private:
	void createActions();
	void clearActions();

};
#endif // INPUTLIST_H