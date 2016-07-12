#pragma once

#include<Qtextbrowser>

class LogHandler :public QTextBrowser
{
	Q_OBJECT

private:
	QAction * _clearAction;

private slots:
	void clearLog();

public:

	enum MessageLevel
	{
		MInfo,
		MWarning,
		MError,
		MLevels
	};


	LogHandler(QWidget *parent);
	~LogHandler();

	void Report(MessageLevel level, const QString & str);

	void contextMenuEvent(QContextMenuEvent *event);
};

void SetLogging(LogHandler * handler);
void gf_report(LogHandler::MessageLevel, const char * format,...);

