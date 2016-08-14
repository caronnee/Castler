#pragma once

#include <Qtextbrowser>
#include "ReportFunctions.h"

class LogHandler :public QTextBrowser, public IReportFunction
{
	Q_OBJECT

private:
	QAction * _clearAction;

private slots:
	void clearLog();

public slots:
	void Report(MessageLevel level, const QString & str);

public:

	LogHandler(QWidget *parent);
	~LogHandler();


	void contextMenuEvent(QContextMenuEvent *event);
	
};

