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

public:

	LogHandler(QWidget *parent);
	~LogHandler();

	void Report(MessageLevel level, const QString & str);

	void contextMenuEvent(QContextMenuEvent *event);
};

