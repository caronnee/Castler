#pragma once

#include<Qtextbrowser>

class LogHandler :public QTextBrowser
{
	Q_OBJECT

public:

	enum MessageLevel
	{
		MWarning,
		MError,
		MInfo,
		MLevels
	};

	LogHandler(QWidget *parent);
	~LogHandler();
	void Log(MessageLevel level, const QString & str);
};
