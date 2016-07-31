﻿#pragma once
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
		ActionDeleteVideo,
		NActions
	};
	QAction *newAct[NActions];
	void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

public:
	void addVideo(const QString & str);

public slots:
	void addVideo();
	void deleteVideo();

private:
	void createActions();
	void clearActions();

};
#endif // INPUTLIST_H